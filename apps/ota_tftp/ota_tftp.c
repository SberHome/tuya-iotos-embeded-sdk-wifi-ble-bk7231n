#include "ota_tftp.h"
#include "net.h"
#include "rtos_pub.h"
#include "my_tftpclient.h"
#include "wlan_ui_pub.h"

#ifndef APP_DEBUG
#define APP_DEBUG 0
#endif
#define debug_print(...)  do { if (APP_DEBUG) os_printf("[APP]"__VA_ARGS__); } while (0);
#define error_print(error_code, message) debug_print("ERROR: %d. " message "\n", error_code)

static bool g_wifi_flag = false;

static network_InitTypeDef_st wNetConfig = {.wifi_ssid = WIFI_SSID,
                                            .wifi_key = WIFI_PASSWORD,
                                            .wifi_mode = STATION,
                                            .dhcp_mode = DHCP_CLIENT,
                                            .wifi_retry_interval = 100};

void wifi_connect_cb(void)
{
    debug_print("wifi_connect_cb\n");
    g_wifi_flag = true;
}

bool is_wifi_connected(void) { return g_wifi_flag; }

void waiting_for_wifi_connected(void)
{
    while (!is_wifi_connected())
    {
        rtos_delay_milliseconds(5000);
    }
}

void ota_tftp_thread(void *param)
{

    while (true)
    {
        debug_print("Thread running...\n");
        rtos_delay_milliseconds(3000);
    }
}

OSStatus wifi_station_init()
{
    OSStatus ret = kNoErr;

    debug_print("ssid:%s key:%s\n", wNetConfig.wifi_ssid, wNetConfig.wifi_key);
    ret = bk_wlan_start(&wNetConfig);

    if (ret != kNoErr)
        error_print(ret, "bk_wlan_start failed");

    return ret;
}

OSStatus user_main(void)
{
    OSStatus ret = kNoErr;
    extended_app_waiting_for_launch(); // need to wait for rl_init() to finish
    net_set_sta_ipup_callback(wifi_connect_cb);

    wifi_station_init();
    tftp_start();
    return ret;
}