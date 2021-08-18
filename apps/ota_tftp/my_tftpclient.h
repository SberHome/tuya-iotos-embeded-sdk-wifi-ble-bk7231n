#ifndef __TFTPCLIENT_H__
#define __TFTPCLIENT_H__

#include "app.h"
#include "str_pub.h"
#include "fake_clock_pub.h"
#include "lwip/sockets.h"


//#define OS1_FLASH_ADDR 0x8C000 // dont know what this is for
typedef struct send_data_hd
{
    u32 total_len;
    u16 seq;
    u16 total_seq;
    u32 os0_ex_addr;
    u32 os_hd_addr;
    u32 os0_flash_addr;
    u32 os1_flash_addr;
} SEND_PTK_HD;

typedef struct img_head
{
    uint32_t bkup_addr;
    uint32_t bkup_len;
    uint32_t ex_addr;
    uint32_t os_addr;
    uint32_t hd_addr;
    uint32_t crc;
    uint32_t status;
    uint32_t bk;
} IMG_HEAD, *IMG_HEAD_P;

#define TFTP_WELL_KNOWN_PORT	69		/* Well known TFTP port #		*/
#define TIMEOUT		5UL		/* Seconds to timeout for a lost pkt	*/
#define TIMEOUT_COUNT	10		/* # of timeouts before giving up  */


/*
 *	TFTP operations.
 */
typedef enum Tftp_operation_e {
    TFTP_RRQ = 1,
    TFTP_WRQ = 2,
    TFTP_DATA =	3,
    TFTP_ACK = 4,
    TFTP_ERROR = 5,
    TFTP_OACK =	6
} Tftp_operation_t;

typedef enum TftpState_e {
    STATE_RRQ = 1,
    STATE_DATA = 2,
    STATE_TOO_LARGE	= 3,
    STATE_BAD_MAGIC	= 4,
    STATE_OACK = 5
} TftpState_t;

typedef struct TftpHandle_s {
    TftpState_t state;
    uint16_t block;     // current block number
    uint16_t lastblock; /* last packet sequence number received */
    uint16_t req_block_size; // requested block size
    uint16_t block_size;    // actual block size,
    uint16_t server_port;
    uint64_t blockwrap;  /* count of sequence number wraparounds */
    uint32_t timeout_counter;
    uint8_t* buf; // data buffer
    char* filename;
} TftpHandle_t;


#define DOWNLOAD_AREA_ADDR 0x00132000   // Методом тыка определил, что прошивку нужно загружать по этому адресу

#define TFTP_DEFAULT_BLOCK_SIZE		512		    /* default TFTP block size	*/
#define TFTP_SEQUENCE_SIZE	((uint64_t)(1<<16))    /* sequence number is 16 bit */
#define TFTP_TIMER    10000   // ms
#define TFTP_SERVER_IP "192.168.43.58"
#define TFTP_FIRMWARE_FILENAME "mqtt_1.0.0.rbl"

/* 512 is poor choice for ethernet, MTU is typically 1500.
 * Minus eth.hdrs thats 1468.  Can get 2x better throughput with
 * almost-MTU block sizes.  At least try... fall back to 512 if need be.
 */
//#define TFTP_MTU_BLOCKSIZE (1024 + sizeof(SEND_PTK_HD))
#define TFTP_REQ_MTU_BLOCKSIZE 1024
#define TFTP_BUF_LEN 1600

OSStatus my_tftp_start(void);

#endif // __TFTPCLIENT_H__