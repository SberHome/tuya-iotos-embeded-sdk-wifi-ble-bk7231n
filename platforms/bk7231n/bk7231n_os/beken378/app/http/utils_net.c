/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <string.h>

#include "include.h"
#include "utils_net.h"
#include "lite-log.h"
#include "errno.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "utils_timer.h"
#include "str_pub.h"
#include "mem_pub.h"

#ifndef UTILS_NET_DEBUG
#define UTILS_NET_DEBUG 0
#endif
#define print_inf(...)                                            \
    do {                                                          \
        if (UTILS_NET_DEBUG) os_printf("[UNET][INF]"__VA_ARGS__); \
    } while (0)
#define print_dbg(...)                                            \
    do {                                                          \
        if (UTILS_NET_DEBUG) os_printf("[UNET][DBG]"__VA_ARGS__); \
    } while (0)
#define print_wrn(...)                                            \
    do {                                                          \
        if (UTILS_NET_DEBUG) os_printf("[UNET][WRN]"__VA_ARGS__); \
    } while (0)
#define print_err(...)                                            \
    do {                                                          \
        if (UTILS_NET_DEBUG) os_printf("[UNET][ERR]"__VA_ARGS__); \
    } while (0)

static handle_type HAL_TCP_Establish(const char* host, uint16_t port) {
    struct addrinfo hints;
    struct addrinfo* addrInfoList = NULL;
    struct addrinfo* cur = NULL;
    int fd = 0;
    int rc = 0;
    char service[6];

    os_memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;  //only IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    sprintf(service, "%u", port);

    if ((rc = getaddrinfo(host, service, &hints, &addrInfoList)) != 0) {
        print_err("getaddrinfo error\n");
        return -1;
    }

    print_dbg("rc=%d\n", rc);

    for (cur = addrInfoList; cur != NULL; cur = cur->ai_next) {
        if (cur->ai_family != AF_INET) {
            print_err("socket type error\n");
            rc = -1;
            continue;
        }

        fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (fd < 0) {
            print_err("create socket error\n");
            rc = -1;
            continue;
        }
        print_dbg("socket fd: %d\n", fd);

        int con_res = connect(fd, cur->ai_addr, cur->ai_addrlen);
        if (con_res != 0) {
            print_err("connect failed, %d\n", con_res);
            close(fd);
            rc = -1;
            continue;
        }
        print_dbg("connect successful:\n");
        rc = fd;
        break;
    }

    if (-1 == rc)
        print_err("fail to establish tcp\n");

    print_dbg("tcp conenction open, fd=%d\n", fd);
    freeaddrinfo(addrInfoList);
    return fd + 1;
}

static int32_t HAL_TCP_Destroy(handle_type fd) {
    fd = fd - 1;
    int rc;

    //Shutdown both send and receive operations.
    print_dbg("shutdown socket fd: %d\n", fd);
    rc = shutdown(fd, 2);
    if (0 != rc) {
        print_err("shutdown error, %d\n", rc);
        return -1;
    }

    print_dbg("close socket fd: %d\n", fd);
    rc = close(fd);
    if (0 != rc) {
        print_err("close socket error, %d\n", rc);
        return -1;
    }
    return 0;
}

static int32_t HAL_TCP_Write(handle_type fd, const char* buf, uint32_t len, uint32_t timeout_ms) {
    fd = fd - 1;
    int ret, err_code;
    uint32_t len_sent;
    uint64_t t_end, t_left;
    fd_set sets;

    t_end = utils_time_get_ms() + timeout_ms;
    len_sent = 0;
    err_code = 0;
    ret = 1;  //send one time if timeout_ms is value 0

    do {
        t_left = utils_time_left(t_end, utils_time_get_ms());

        if (0 != t_left) {
            struct timeval timeout;

            FD_ZERO(&sets);
            FD_SET(fd, &sets);

            timeout.tv_sec = t_left / 1000;
            timeout.tv_usec = (t_left % 1000) * 1000;

            print_dbg("select fd: %d\n", fd);
            ret = select(fd + 1, NULL, &sets, NULL, &timeout);
            print_dbg("select returned %d, fd: %d\n", ret, fd);

            if (ret > 0) {
                if (0 == FD_ISSET(fd, &sets)) {
                    print_err("Should NOT arrive\n");
                    //If timeout in next loop, it will not sent any data
                    ret = 0;
                    continue;
                }
            } else if (0 == ret) {
                print_err("select-write timeout %lu\n", fd);
                break;
            } else {
                if (EINTR == errno) {
                    print_err("EINTR be caught\n");
                    continue;
                }

                err_code = -1;
                print_err("select-write fail\n");
                break;
            }
        }

        if (ret > 0) {
            ret = send(fd, buf + len_sent, len - len_sent, 0);
            if (ret > 0) {
                len_sent += ret;
            } else if (0 == ret) {
                print_err("No data be sent\n");
            } else {
                if (EINTR == errno) {
                    print_err("EINTR be caught\n");
                    continue;
                }

                err_code = -1;
                print_err("send fail\n");
                break;
            }
        }
    } while ((len_sent < len) && (utils_time_left(t_end, utils_time_get_ms()) > 0));

    return len_sent;
}

static int32_t HAL_TCP_Read(handle_type fd, char* buf, uint32_t len, uint32_t timeout_ms) {
    fd = fd - 1;
    int ret, err_code, data_over;
    uint32_t len_recv;
    uint64_t t_end, t_left;
    fd_set sets;
    struct timeval timeout;

    t_end = utils_time_get_ms() + timeout_ms;
    len_recv = 0;
    err_code = 0;

    data_over = 0;

    do {
        t_left = utils_time_left(t_end, utils_time_get_ms());
        if (0 == t_left && bk_http_ptr->do_data == 0) {
            break;
        }
        FD_ZERO(&sets);
        FD_SET(fd, &sets);

        timeout.tv_sec = t_left / 1000;
        timeout.tv_usec = (t_left % 1000) * 1000;

        ret = select(fd + 1, &sets, NULL, NULL, NULL);
        if (FD_ISSET(fd, &sets)) {
            if (ret > 0) {
                ret = recv(fd, buf, len, 0);
                if (ret > 0) {
                    if (ret < len) {
                        data_over = 1;
                    }
                    if (bk_http_ptr->do_data == 1) {
                        http_data_process(buf, ret);
                    }

                    len_recv += ret;
                } else if (0 == ret) {
                    print_err("connection is closed\n");
                    err_code = -1;
                    break;
                } else {
                    if (EINTR == errno) {
                        print_err("EINTR be caught\n");
                        continue;
                    }
                    print_err("send fail\n");
                    err_code = -2;
                    break;
                }
            } else if (0 == ret) {
                break;
            } else {
                if (EINTR == errno) {
                    print_err("EINTR be caught-------\n");
                    //continue;
                }
                print_err("select-recv fail");
                err_code = -2;
                break;
            }
        } else {
        }
    } while ((bk_http_ptr->do_data == 1 && len_recv < bk_http_ptr->http_total) || ((len_recv < len) && (0 == data_over)));

    //priority to return data bytes if any data be received from TCP connection.
    //It will get error code on next calling
    return (0 != len_recv) ? len_recv : err_code;
}

/*** TCP connection ***/
static int read_tcp(utils_network_pt pNetwork, char* buffer, uint32_t len, uint32_t timeout_ms) {
    return HAL_TCP_Read(pNetwork->handle, buffer, len, timeout_ms);
}

static int write_tcp(utils_network_pt pNetwork, const char* buffer, uint32_t len, uint32_t timeout_ms) {
    return HAL_TCP_Write(pNetwork->handle, buffer, len, timeout_ms);
}

static int disconnect_tcp(utils_network_pt pNetwork) {
    if (HANDLE_INVALID == pNetwork->handle) {
        return -1;
    }

    HAL_TCP_Destroy(pNetwork->handle);
    pNetwork->handle = HANDLE_INVALID;
    return 0;
}

static int connect_tcp(utils_network_pt pNetwork) {
    if (NULL == pNetwork) {
        print_err("network is null\n");
        return -1;
    }

    print_inf("Opening TCP to %s:%d\n", pNetwork->pHostAddress, pNetwork->port);
    pNetwork->handle = HAL_TCP_Establish(pNetwork->pHostAddress, pNetwork->port);
    if (HANDLE_INVALID == pNetwork->handle) {
        print_err("HAL_TCP_Establish failed\n");
        return -1;
    }
    return 0;
}

/****** network interface ******/

int utils_net_read(utils_network_pt pNetwork, char* buffer, uint32_t len, uint32_t timeout_ms) {
    if (NULL == pNetwork->ca_crt) {  //TCP connection
        return read_tcp(pNetwork, buffer, len, timeout_ms);
    }

    return 0;
}

int utils_net_write(utils_network_pt pNetwork, const char* buffer, uint32_t len, uint32_t timeout_ms) {
    if (NULL == pNetwork->ca_crt) {  //TCP connection
        return write_tcp(pNetwork, buffer, len, timeout_ms);
    }
    return 0;
}

int iotx_net_disconnect(utils_network_pt pNetwork) {
    if (NULL == pNetwork->ca_crt) {  //TCP connection
        return disconnect_tcp(pNetwork);
    }
    return 0;
}

int iotx_net_connect(utils_network_pt pNetwork) {
    if (NULL == pNetwork->ca_crt) {  //TCP connection
        return connect_tcp(pNetwork);
    }
    return 0;
}

int iotx_net_init(utils_network_pt pNetwork, const char* host, uint16_t port, const char* ca_crt) {
    if (!pNetwork || !host) {
        print_err("parameter error! pNetwork=%p, host = %p\n", pNetwork, host);
        return -1;
    }
    pNetwork->pHostAddress = host;
    pNetwork->port = port;
    pNetwork->ca_crt = ca_crt;

    if (NULL == ca_crt) {
        pNetwork->ca_crt_len = 0;
    } else {
        pNetwork->ca_crt_len = os_strlen(ca_crt);
    }

    pNetwork->handle = HANDLE_INVALID;
    pNetwork->read = utils_net_read;
    pNetwork->write = utils_net_write;
    pNetwork->disconnect = iotx_net_disconnect;
    pNetwork->connect = iotx_net_connect;

    return 0;
}
