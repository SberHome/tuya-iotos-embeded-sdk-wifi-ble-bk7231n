#include "my_tftpclient.h"
#include "flash_pub.h"
#include "co_math.h"
#include "rtos_pub.h"

#ifndef TFTP_CLIENT_DEBUG
#define TFTP_CLIENT_DEBUG 0
#endif
#define print_inf(...) do { if (TFTP_CLIENT_DEBUG) os_printf("[TFTP][INF]"__VA_ARGS__); } while (0);
//#define print_dbg(...) do { if (TFTP_CLIENT_DEBUG) os_printf("[TFTP][DBG]"__VA_ARGS__); } while (0);
#define print_dbg(...) 
#define print_wrn(...) do { if (TFTP_CLIENT_DEBUG) os_printf("[TFTP][WRN]"__VA_ARGS__); } while (0);
#define print_err(...) do { if (TFTP_CLIENT_DEBUG) os_printf("[TFTP][ERR]"__VA_ARGS__); } while (0);


void store_block_alt(unsigned int block_num, uint8_t *src, unsigned int len);

SEND_PTK_HD send_hd, send_hd_bk;
static u32 os_data_addr = OS1_FLASH_ADDR;
IMG_HEAD img_hd = {
    0,
};


beken_semaphore_t sm_tftp_server;
beken_timer_t tm_tftp_server;
xTaskHandle tftp_thread_handle = NULL;

static uint32_t tftp_crc = 0;
int udp_tftp_listen_fd = -1;
struct sockaddr_in server_addr;
socklen_t s_addr_len = sizeof(server_addr);
char *tftp_buf = NULL;
char BootFile[128] = TFTP_FIRMWARE_FILENAME; /* Boot File name			*/
static int TftpServerPort;                /* The UDP port at their end		*/
static int TftpTimeoutCount;
static uint16_t TftpBlock;           /* packet sequence number		*/
static uint16_t TftpLastBlock;       /* last packet sequence number received */
static uint64_t TftpBlockWrap;       /* count of sequence number wraparounds */
static uint64_t TftpBlockWrapOffset; /* memory offset due to wrapping	*/
static int TftpState;

static unsigned short TftpBlkSize = TFTP_BLOCK_SIZE;
static unsigned short TftpBlkSizeOption = TFTP_MTU_BLOCKSIZE;

int string_to_ip(char *s)
{
    int tftp_s_addr = inet_addr(s);
    return tftp_s_addr;
}

static void TftpSend(void)
{
    uint8_t pkt_buf[700];
    volatile uint8_t *pkt;
    volatile uint8_t *xp;
    int len = 0;
    volatile uint16_t *s;

    os_memset(pkt_buf, 0, 700);
    pkt = pkt_buf;
    
    switch (TftpState)
    {

    case STATE_RRQ:
        xp = pkt;
        s = (uint16_t *)pkt;
        *s++ = htons(TFTP_RRQ);
        pkt = (uint8_t *)s;
        os_strcpy((char *)pkt, BootFile);
        pkt += os_strlen(BootFile) + 1;
        os_strcpy((char *)pkt, "octet");
        pkt += 5 /*strlen("octet")*/ + 1;
        os_strcpy((char *)pkt, "timeout");
        pkt += 7 /*strlen("timeout")*/ + 1;
        sprintf((char *)pkt, "%lu", TIMEOUT);
        print_inf("RRQ option \"timeout: %s\"\n", (char *)pkt);
        pkt += os_strlen((char *)pkt) + 1;
        /* try for more effic. blk size */
        pkt += sprintf((char *)pkt, "blksize%c%d%c", 0, TftpBlkSizeOption, 0);
        print_inf("RRQ option \"blksize: %d\"\n", TftpBlkSizeOption);

        len = pkt - xp;
        break;

    case STATE_OACK:
    case STATE_DATA:
        xp = pkt;
        s = (uint16_t *)pkt;
        *s++ = htons(TFTP_ACK);
        *s++ = htons(TftpBlock);
        pkt = (uint8_t *)s;
        len = pkt - xp;
        break;

    case STATE_TOO_LARGE:
        xp = pkt;
        s = (uint16_t *)pkt;
        *s++ = htons(TFTP_ERROR);
        *s++ = htons(3);
        pkt = (uint8_t *)s;
        os_strcpy((char *)pkt, "File too large");
        pkt += 14 /*strlen("File too large")*/ + 1;
        len = pkt - xp;
        break;

    case STATE_BAD_MAGIC:
        xp = pkt;
        s = (uint16_t *)pkt;
        *s++ = htons(TFTP_ERROR);
        *s++ = htons(2);
        pkt = (uint8_t *)s;
        os_strcpy((char *)pkt, "File has bad magic");
        pkt += 18 /*strlen("File has bad magic")*/ + 1;
        len = pkt - xp;
        break;
    }

    len = sendto(udp_tftp_listen_fd, pkt_buf, len, 0, (struct sockaddr *)&server_addr, s_addr_len);
    //print_inf("Server port: %d\n", ntohs(server_addr.sin_port));
}

static void Tftp_Uninit(void)
{
    rtos_deinit_timer(&tm_tftp_server);
    close(udp_tftp_listen_fd);

    if (tftp_buf)
    {
        os_free(tftp_buf);
        tftp_buf = NULL;
    }

    if (tftp_thread_handle)
    {
        rtos_delete_thread(&tftp_thread_handle);
        tftp_thread_handle = 0;
    }
}

static void TftpHandler(char *buffer, unsigned int len, u16_t port)
{
    uint16_t proto;
    uint16_t *s = (uint16_t *)buffer;;
    int i;
    volatile uint8_t *pkt;

    if (TftpState != STATE_RRQ && port != TftpServerPort)
    {
        print_err("err %d %x %x\n", TftpState, port, TftpServerPort);
        return;
    }

    if (len < 2)
    {
        return;
    }
    len -= 2;
    /* warning: don't use increment (++) in ntohs() macros!! */
    proto = *s++;
    pkt = (uint8_t *)s;

    switch (ntohs(proto))
    {
    case TFTP_RRQ:
    case TFTP_WRQ:
    case TFTP_ACK:
        break;
    default:
        break;

    case TFTP_OACK:
        print_inf("Got OACK: %s %s\n", pkt, pkt + os_strlen((UINT8 *)pkt) + 1);
        TftpState = STATE_OACK;
        TftpServerPort = port;
        /*
         * Check for 'blksize' option.
         * Careful: "i" is signed, "len" is unsigned, thus
         * something like "len-8" may give a *huge* number
         */
        for (i = 0; i + 8 < len; i++)
        {
            if (os_strcmp((char *)pkt + i, "blksize") == 0)
            {
                TftpBlkSize = (unsigned short) os_strtoul((char *)pkt + i + 8, NULL, 10);

                print_inf("Blocksize ack: %s, %d\n", (char *)pkt + i + 8, TftpBlkSize);
                break;
            }
        }
        TftpSend(); /* Send ACK */
        break;
    case TFTP_DATA:
        TftpBlock = ntohs(*s);
        print_dbg("Got DATA block: %d\n", TftpBlock);
        if (len < 2)
            return;
        len -= 2;
        

        /*
         * RFC1350 specifies that the first data packet will
         * have sequence number 1. If we receive a sequence
         * number of 0 this means that there was a wrap
         * around of the (16 bit) counter.
         */
        if (TftpBlock == 0)
        {
            TftpBlockWrap++;
            TftpBlockWrapOffset += TftpBlkSize * TFTP_SEQUENCE_SIZE;
            print_inf("%lu MB received\n", TftpBlockWrapOffset >> 20);
        }
        else
        {
            if (((TftpBlock - 1) % 10) == 0)
            {
                //print_inf("#");
            }
            else if ((TftpBlock % (10 * HASHES_PER_LINE)) == 0)
            {
                //print_inf("\n");
            }
        }

        if (TftpState == STATE_RRQ)
        {
            print_inf("Server did not acknowledge timeout option!\n");
        }

        if (TftpState == STATE_RRQ || TftpState == STATE_OACK)
        {
            /* first block received */
            TftpState = STATE_DATA;
            TftpServerPort = port;
            TftpLastBlock = 0;
            TftpBlockWrap = 0;
            TftpBlockWrapOffset = 0;

            if (TftpBlock != 1) /* Assertion */
            {
                print_inf("TFTP error: First block is not block 1 (%d) Starting again\n", TftpBlock);
                break;
            }
        }

        if (TftpBlock == TftpLastBlock)
        {
            /*
             *	Same block again; ignore it.
             */
            TftpSend();
            break;
        }

        TftpLastBlock = TftpBlock;
        //store_block(TftpBlock - 1, (UINT8 *)(pkt + 2), len);
        uint16_t* block_data = ++s;
        //store_block(TftpBlock - 1, (UINT8 *)(block_data), len);
        store_block_alt(TftpBlock - 1, (UINT8 *)(block_data), len);

        /*
         *	Acknoledge the block just received, which will prompt
         *	the server for the next one.
         */
        TftpSend();

        if (len < TftpBlkSize)
        {
            /*
             *	We received the whole thing.  Try to
             *	run it.
             */
            print_inf("tftp succeed\n");
            Tftp_Uninit();
        }
        break;

    case TFTP_ERROR:
        print_err("TFTP error: '%s' (%d)\n", pkt + 2, ntohs(*(uint16_t *)pkt));
        print_inf("Starting again\n");
        break;
    }
}

static void TftpTimeout(void *data)
{
    print_inf("------\n");
    if (++TftpTimeoutCount > TIMEOUT_COUNT)
    {
        print_inf("Retry count exceeded; starting again\n");
    }
    else
    {
        TftpSend();
    }
}

static void TftpStart(void)
{
    if (BootFile[0] == '\0')
    {
        print_inf("*** Warning: no boot file name;\n");
    }

    print_inf("Filename '%s'\n", BootFile);
    print_inf("Load addr: 0x%lx\n", OS1_FLASH_ADDR);
    print_inf("Loading: *\n");

    tftp_crc = 0;
    TftpServerPort = WELL_KNOWN_PORT;
    TftpTimeoutCount = 0;
    TftpState = STATE_RRQ;
    /* Use a pseudo-random port unless a specific port is set */
    TftpBlock = 0;
    /* Revert TftpBlkSize to dflt */
    TftpBlkSize = TFTP_BLOCK_SIZE;

}


static void tftp_server_process(beken_thread_arg_t arg)
{
    int result;
    (void)(arg);

    OSStatus err = kNoErr;
    int len = 0;
    int i;

    tftp_buf = (char *)os_malloc(TFTP_BUF_LEN);
    if (tftp_buf == NULL)
    {
        print_err("buf == NULL\n");
        goto exit;
    }

    udp_tftp_listen_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP); //Make listening UDP socket
    if (udp_tftp_listen_fd == -1)
    {
        print_err("Cannot create socket\n");
        goto exit;
    }
    os_memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = string_to_ip(TFTP_SERVER_IP);
    server_addr.sin_port = htons(WELL_KNOWN_PORT);
    
    TftpStart();

    
    result = rtos_init_timer(&tm_tftp_server, TFTP_TIMER, TftpTimeout, NULL);
    if (result != kNoErr)
    {
        print_err("Cannot init timer\n");
        goto exit;
    }
    result = rtos_start_timer(&tm_tftp_server);
    if (result != kNoErr)
    {
        print_err("Cannot start timer\n");
        goto exit;
    }

    flash_protection_op(FLASH_XTX_16M_SR_WRITE_ENABLE, FLASH_PROTECT_NONE);
    print_inf("Flash protection disabled\n");


    while (true)
    {
        len = recvfrom(udp_tftp_listen_fd, tftp_buf, TFTP_BUF_LEN, 0, (struct sockaddr *)&server_addr, &s_addr_len);
        result = rtos_reload_timer(&tm_tftp_server);
        if (result != kNoErr)
        {
            print_err("Cannot restart timer\n");
            goto exit;
        }
        print_dbg("Received len: %d\n", len);
        TftpHandler(tftp_buf, len, server_addr.sin_port);
        //rtos_delay_milliseconds(3000);
        rtos_delay_milliseconds(100);
    }

exit:
    if (err != kNoErr)
        print_inf("Server listener thread exit with err: %d", err);

    close(udp_tftp_listen_fd);

    if (tftp_buf)
        os_free(tftp_buf);

    flash_protection_op(FLASH_XTX_16M_SR_WRITE_ENABLE, FLASH_UNPROTECT_LAST_BLOCK);
    rtos_delete_thread(NULL);
}

// Starts TFTP service
OSStatus tftp_start(void)
{
    OSStatus ret;

    Tftp_Uninit();

    print_inf("Starting TFTP client\n");
    
    rtos_init_semaphore(&sm_tftp_server, 10);

    /* Start ps server listener thread*/
    ret = rtos_create_thread(&tftp_thread_handle, BEKEN_APPLICATION_PRIORITY,
                                 "tftp_ota",
                                 tftp_server_process,
                                 0x1000,
                                 NULL);

    if (kNoErr != ret)
        print_err("TFTP thread create failed\n");
    return ret;
}

// Пишет блоки в память в сыром виде
void store_block_alt(unsigned int block_num, uint8_t *src, unsigned int len)
{
    print_dbg("Store block: %d len: %d\n", block_num, len);
    uint32_t const dwnld_area_addr = 0x00132000;    // Методом тыка определил, что прошивку нужно загружать по этому адресу
    static uint32_t current_address = dwnld_area_addr;

    // Erasing sector
    if (current_address % 0x1000 == 0)
    {
        uint32_t param = current_address;
        flash_ctrl(CMD_FLASH_ERASE_SECTOR, &param);
        print_dbg("erase_addr: %x \n", param);
    }

    print_dbg("Writing to: %x, len: %d\n", current_address, len);
    flash_write(src, len, current_address);

    bool verify_ok = true;
    for (uint32_t offset = 0; offset < len; offset++)
    {
        uint32_t adr = current_address + offset;
        uint8_t r_val;
        uint8_t w_val = src[offset];
        flash_read(&r_val, 4, adr);
        if (r_val != w_val)
            verify_ok = false;
    }

    if (verify_ok) {
        print_inf("block: %d, len: %d, verify OK!\n", block_num, len);
    }
    else {
        print_err("block: %d, len: %d, verify FAIL!\n", block_num, len);
    }

    current_address += len;
}
