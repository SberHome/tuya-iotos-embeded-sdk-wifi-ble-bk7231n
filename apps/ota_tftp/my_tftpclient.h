#ifndef __TFTPCLIENT_H__
#define __TFTPCLIENT_H__

#include "app.h"
#include "str_pub.h"
#include "fake_clock_pub.h"

#include "lwip/sockets.h"

#define OS1_FLASH_ADDR 0x8C000
#define TFTP_ALL_CRC_LEN 4

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

#define TFTP_PKT_HD_LEN (sizeof(SEND_PTK_HD))

extern UINT32 flash_read(char *user_buf, UINT32 count, UINT32 address);
extern UINT32 flash_write(char *user_buf, UINT32 count, UINT32 address);
extern UINT32 flash_ctrl(UINT32 cmd, void *parm);
void store_block (unsigned block, uint8_t *src, unsigned len);


#define ET_DEBUG 0

#define WELL_KNOWN_PORT	69		/* Well known TFTP port #		*/
#define TIMEOUT		5UL		/* Seconds to timeout for a lost pkt	*/
#ifndef	CONFIG_NET_RETRY_COUNT
#define TIMEOUT_COUNT	10		/* # of timeouts before giving up  */
#else
# define TIMEOUT_COUNT  (CONFIG_NET_RETRY_COUNT * 2)
#endif
/* (for checking the image size)	*/
#define HASHES_PER_LINE	65		/* Number of "loading" hashes per line	*/

/*
 *	TFTP operations.
 */
#define TFTP_RRQ	1
#define TFTP_WRQ	2
#define TFTP_DATA	3
#define TFTP_ACK	4
#define TFTP_ERROR	5
#define TFTP_OACK	6

#define STATE_RRQ	1
#define STATE_DATA	2
#define STATE_TOO_LARGE	3
#define STATE_BAD_MAGIC	4
#define STATE_OACK	5
#define TFTP_BLOCK_SIZE		512		    /* default TFTP block size	*/
#define TFTP_SEQUENCE_SIZE	((uint64_t)(1<<16))    /* sequence number is 16 bit */
#define TFTP_TIMER    10000   // ms
#define TFTP_SERVER_IP "192.168.43.58"
#define TFTP_FIRMWARE_FILENAME "mqtt_1.0.0.rbl"

/* 512 is poor choice for ethernet, MTU is typically 1500.
 * Minus eth.hdrs thats 1468.  Can get 2x better throughput with
 * almost-MTU block sizes.  At least try... fall back to 512 if need be.
 */
//#define TFTP_MTU_BLOCKSIZE (1024 + sizeof(SEND_PTK_HD))
#define TFTP_MTU_BLOCKSIZE 1024
#define TFTP_BUF_LEN 1600

OSStatus tftp_start(void);

#endif // __TFTPCLIENT_H__