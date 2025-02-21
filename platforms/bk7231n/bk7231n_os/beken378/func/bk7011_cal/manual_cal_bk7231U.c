#include "include.h"
#include "uart_pub.h"

#if (CFG_SOC_NAME != SOC_BK7231)
#include "temp_detect_pub.h"
#include "arm_arch.h"
#include "sys_ctrl_pub.h"
#include "bk7011_cal_pub.h"
#include "drv_model_pub.h"
#include "ate_app.h"
#include "power_save_pub.h"
#include "mem_pub.h"

#define MCAL_DEBUG 1
#if MCAL_DEBUG
//#define debug_print      os_printf
//#define debug_print     os_printf
//#define debug_print    fatal_prf
#else
#define debug_print null_prf
#define debug_print null_prf
#define debug_print null_prf
#endif

#ifndef CAL_DEBUG
#define CAL_DEBUG 0
#endif
#define debug_print(...)                              \
    do {                                              \
        if (CAL_DEBUG) os_printf("[CAL]"__VA_ARGS__); \
    } while (0)



#if CFG_SUPPORT_MANUAL_CALI
#include "saradc_pub.h"
#include "param_config.h"

#if CFG_SUPPORT_ALIOS
#include "hal/soc/soc.h"
#else
#include "BkDriverFlash.h"
#endif
#include "cmd_evm.h"


#define TXPWR_DEFAULT_TAB 1

/* support max work cuurent<250mA@3.3V */
#define CFG_SUPPORT_LOW_MAX_CURRENT 0

#define MCAL_SAVE_MAC_ADDR 1

#define WLAN_2_4_G_CHANNEL_NUM (14)
#define BLE_2_4_G_CHANNEL_NUM (40)
#define MAX_RATE_FOR_B_5M (5)  // 5.5M
#define MAX_RATE_FOR_B (11)    // 11M
#define MAX_RATE_FOR_G (54)    // 54M
#define MIN_RATE_FOR_N (128)   // MCS0
#define MAX_RATE_FOR_N (135)   // MCS7

#if (CFG_SOC_NAME == SOC_BK7231N)
#define MOD_DIST_G_BW_N20 (4)  // bk7231 is 2    //wyg 20200330  2->4
#define MOD_DIST_G_BW_N40 (4)  //wyg 20200330  4->8
#else
#define MOD_DIST_G_BW_N20 (2)  // bk7231 is 2
#define MOD_DIST_G_BW_N40 (3)
#endif

#define TXPWR_ELEM_INUSED (0)
#define TXPWR_ELEM_UNUSED (1)

#define BK_FLASH_SECTOR_SIZE (4 * 1024)
#define BK_FLASH_WRITE_CHECK_TIMES 3



/* bit[7]: TXPWR flag 
 *          0:  invalid
 *          1:  used
 * bit[4:0]: 5bit TXPWR pwr_gain;
 */
#define FLAG_MASK (0x1u)
#define FLAG_POSI (7)
#if (CFG_SOC_NAME == SOC_BK7231N)
#define GAIN_MASK (0x7fu)
#else
#define GAIN_MASK (0x1fu)
#endif
#define GET_TXPWR_GAIN(p) ((p)->value & GAIN_MASK)
#define SET_TXPWR_GAIN(p, gain)           \
    {                                     \
        (p)->value &= (~GAIN_MASK);       \
        (p)->value |= (gain & GAIN_MASK); \
    }

#define GET_TXPWR_FLAG(p) (((p)->value >> FLAG_POSI) & FLAG_MASK)
#define SET_TXPWR_FLAG(p, flag)                          \
    {                                                    \
        (p)->value &= (~(FLAG_MASK << FLAG_POSI));       \
        (p)->value |= ((flag & FLAG_MASK) << FLAG_POSI); \
    }
#define INIT_TXPWR_VALUE(gain, flag) \
    { (((flag & FLAG_MASK) << FLAG_POSI) | (gain & GAIN_MASK)) }
typedef struct txpwr_st {
    UINT8 value;
} TXPWR_ST, *TXPWR_PTR;

typedef struct txpwr_elem_st {
    UINT32 type;
    UINT32 len;
} TXPWR_ELEM_ST, *TXPWR_ELEM_PTR;

typedef struct tag_txpwr_st {
    TXPWR_ELEM_ST head;
} TAG_TXPWR_ST, *TAG_TXPWR_PTR;

typedef struct tag_enable_st {
    TXPWR_ELEM_ST head;
    UINT32 flag;
} TAG_ENABLE_ST, *TAG_ENABLE_PTR;

typedef struct tag_dif_st {
    TXPWR_ELEM_ST head;
    UINT32 differ;
} TAG_DIF_ST, *TAG_DIF_PTR;

typedef struct tag_txpwr_tab_st {
    TXPWR_ELEM_ST head;
    TXPWR_ST tab[WLAN_2_4_G_CHANNEL_NUM];
} __attribute__((packed)) TAG_TXPWR_TAB_ST, *TAG_TXPWR_TAB_PTR;

typedef struct tag_txpwr_tab_ble_st {
    TXPWR_ELEM_ST head;
    TXPWR_ST tab[BLE_2_4_G_CHANNEL_NUM];
} __attribute__((packed)) TAG_TXPWR_TAB_BLE_ST, *TAG_TXPWR_TAB_BLE_PTR;

typedef struct tag_common_st {
    TXPWR_ELEM_ST head;
    UINT32 value;
} TAG_COMM_ST, *TAG_COMM_PTR;

typedef struct tag_chip_mac_st {
    TXPWR_ELEM_ST head;
    UINT8 arry[6];
} __attribute__((packed)) TAG_CHIPMAC_ST, *TAG_CHIPMAC_PTR;

typedef struct tag_lpf_iq_st {
    TXPWR_ELEM_ST head;
    UINT32 lpf_i;
    UINT32 lpf_q;
} __attribute__((packed)) TAG_LPF_IQ_ST, *TAG_LPF_IQ_PTR;

typedef struct tag_rx_dc_st {
    TXPWR_ELEM_ST head;
    UINT32 value[8];
} TAG_RX_DC_ST, *TAG_RX_DC_PTR;

#if TXPWR_DEFAULT_TAB
#if CFG_SUPPORT_LOW_MAX_CURRENT  //Max default work current<250mA@3.3V
const TXPWR_ST gtxpwr_tab_def_b[WLAN_2_4_G_CHANNEL_NUM] = {
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_INUSED),  // ch1  inused
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(15, TXPWR_ELEM_UNUSED),  // ch4
    INIT_TXPWR_VALUE(15, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(15, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(14, TXPWR_ELEM_UNUSED),  // ch7
    INIT_TXPWR_VALUE(14, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(14, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(13, TXPWR_ELEM_UNUSED),  // ch10
    INIT_TXPWR_VALUE(13, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(13, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(12, TXPWR_ELEM_INUSED),  // ch13  inused
    INIT_TXPWR_VALUE(12, TXPWR_ELEM_UNUSED),
};
#else

#if (CFG_SOC_NAME == SOC_BK7231U)
const TXPWR_ST gtxpwr_tab_def_b[WLAN_2_4_G_CHANNEL_NUM] = {
    INIT_TXPWR_VALUE(20, TXPWR_ELEM_INUSED),  // ch1  inused
    INIT_TXPWR_VALUE(20, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(20, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(19, TXPWR_ELEM_UNUSED),  // ch4
    INIT_TXPWR_VALUE(19, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(19, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(18, TXPWR_ELEM_UNUSED),  // ch7
    INIT_TXPWR_VALUE(18, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(18, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(17, TXPWR_ELEM_UNUSED),  // ch10
    INIT_TXPWR_VALUE(17, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(17, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_INUSED),  // ch13  inused
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),
};
#elif (CFG_SOC_NAME == SOC_BK7231N)
const TXPWR_ST gtxpwr_tab_def_b[WLAN_2_4_G_CHANNEL_NUM] = {
    INIT_TXPWR_VALUE(24, TXPWR_ELEM_INUSED),  // ch1  inused
    INIT_TXPWR_VALUE(24, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(24, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(24, TXPWR_ELEM_UNUSED),  // ch4
    INIT_TXPWR_VALUE(24, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(24, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(24, TXPWR_ELEM_UNUSED),  // ch7
    INIT_TXPWR_VALUE(24, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(24, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(24, TXPWR_ELEM_UNUSED),  // ch10
    INIT_TXPWR_VALUE(24, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(24, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(24, TXPWR_ELEM_INUSED),  // ch13  inused
    INIT_TXPWR_VALUE(24, TXPWR_ELEM_UNUSED),
};
#else
const TXPWR_ST gtxpwr_tab_def_b[WLAN_2_4_G_CHANNEL_NUM] = {
    INIT_TXPWR_VALUE(21, TXPWR_ELEM_INUSED),  // ch1  inused
    INIT_TXPWR_VALUE(20, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(20, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(19, TXPWR_ELEM_UNUSED),  // ch4
    INIT_TXPWR_VALUE(18, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(18, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(17, TXPWR_ELEM_UNUSED),  // ch7
    INIT_TXPWR_VALUE(17, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(17, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),  // ch10
    INIT_TXPWR_VALUE(15, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(15, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(14, TXPWR_ELEM_INUSED),  // ch13  inused
    INIT_TXPWR_VALUE(14, TXPWR_ELEM_UNUSED),
};
#endif  // (CFG_SOC_NAME == SOC_BK7231U)
#endif

#if (CFG_SOC_NAME == SOC_BK7231U)
const TXPWR_ST gtxpwr_tab_def_g[WLAN_2_4_G_CHANNEL_NUM] = {
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_INUSED),  // ch1  inused
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(15, TXPWR_ELEM_UNUSED),  // ch4
    INIT_TXPWR_VALUE(15, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(15, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(14, TXPWR_ELEM_UNUSED),  // ch7
    INIT_TXPWR_VALUE(14, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(14, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(13, TXPWR_ELEM_UNUSED),  // ch10
    INIT_TXPWR_VALUE(13, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(13, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(12, TXPWR_ELEM_INUSED),  // ch13  inused
    INIT_TXPWR_VALUE(12, TXPWR_ELEM_UNUSED),
};

const TXPWR_ST gtxpwr_tab_def_n_40[WLAN_2_4_G_CHANNEL_NUM] = {
    INIT_TXPWR_VALUE(8, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(8, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(8, TXPWR_ELEM_UNUSED),  // ch3
    INIT_TXPWR_VALUE(8, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(8, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(8, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(7, TXPWR_ELEM_UNUSED),  // ch7
    INIT_TXPWR_VALUE(7, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(7, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(6, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(5, TXPWR_ELEM_UNUSED),  // ch11
    INIT_TXPWR_VALUE(5, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(5, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(5, TXPWR_ELEM_UNUSED),
};
#elif (CFG_SOC_NAME == SOC_BK7231N)
const TXPWR_ST gtxpwr_tab_def_g[WLAN_2_4_G_CHANNEL_NUM] = {
    INIT_TXPWR_VALUE(49, TXPWR_ELEM_INUSED),  // ch1  inused
    INIT_TXPWR_VALUE(49, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(49, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(49, TXPWR_ELEM_UNUSED),  // ch4
    INIT_TXPWR_VALUE(49, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(49, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(49, TXPWR_ELEM_UNUSED),  // ch7
    INIT_TXPWR_VALUE(49, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(49, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(49, TXPWR_ELEM_UNUSED),  // ch10
    INIT_TXPWR_VALUE(49, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(49, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(49, TXPWR_ELEM_INUSED),  // ch13  inused
    INIT_TXPWR_VALUE(49, TXPWR_ELEM_UNUSED),
};

const TXPWR_ST gtxpwr_tab_def_n_40[WLAN_2_4_G_CHANNEL_NUM] = {
    INIT_TXPWR_VALUE(45, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(45, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(45, TXPWR_ELEM_UNUSED),  // ch3
    INIT_TXPWR_VALUE(45, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(45, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(45, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(45, TXPWR_ELEM_UNUSED),  // ch7
    INIT_TXPWR_VALUE(45, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(45, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(45, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(45, TXPWR_ELEM_UNUSED),  // ch11
    INIT_TXPWR_VALUE(45, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(45, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(45, TXPWR_ELEM_UNUSED),
};
#else
const TXPWR_ST gtxpwr_tab_def_g[WLAN_2_4_G_CHANNEL_NUM] = {
    INIT_TXPWR_VALUE(18, TXPWR_ELEM_INUSED),  // ch1  inused
    INIT_TXPWR_VALUE(17, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(17, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),  // ch4
    INIT_TXPWR_VALUE(15, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(15, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(14, TXPWR_ELEM_UNUSED),  // ch7
    INIT_TXPWR_VALUE(14, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(14, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(13, TXPWR_ELEM_UNUSED),  // ch10
    INIT_TXPWR_VALUE(12, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(12, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(11, TXPWR_ELEM_INUSED),  // ch13  inused
    INIT_TXPWR_VALUE(11, TXPWR_ELEM_UNUSED),
};

const TXPWR_ST gtxpwr_tab_def_n_40[WLAN_2_4_G_CHANNEL_NUM] = {
    INIT_TXPWR_VALUE(10, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(9, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(9, TXPWR_ELEM_UNUSED),  // ch3
    INIT_TXPWR_VALUE(8, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(7, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(7, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(6, TXPWR_ELEM_UNUSED),  // ch7
    INIT_TXPWR_VALUE(6, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(6, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(5, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(4, TXPWR_ELEM_UNUSED),  // ch11
    INIT_TXPWR_VALUE(4, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(3, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(3, TXPWR_ELEM_UNUSED),
};
#endif  // (CFG_SOC_NAME == SOC_BK7231)
const TXPWR_ST gtxpwr_tab_def_ble[BLE_2_4_G_CHANNEL_NUM] = {
    INIT_TXPWR_VALUE(20, TXPWR_ELEM_UNUSED),  // ch0 2402  inused
    INIT_TXPWR_VALUE(20, TXPWR_ELEM_UNUSED),  // ch1 2404
    INIT_TXPWR_VALUE(20, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(20, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(20, TXPWR_ELEM_UNUSED),  // ch4 2410
    INIT_TXPWR_VALUE(19, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(19, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(19, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(19, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(19, TXPWR_ELEM_UNUSED),  // ch9 2420
    INIT_TXPWR_VALUE(18, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(18, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(18, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(18, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(18, TXPWR_ELEM_UNUSED),  // ch14 2430
    INIT_TXPWR_VALUE(17, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(17, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(17, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(17, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_INUSED),  // ch19 2440 inused
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),  // ch24 2450
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),  // ch29 2460
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),  // ch34 2470
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),
    INIT_TXPWR_VALUE(16, TXPWR_ELEM_UNUSED),  // ch39 2480 inused
};
#endif  // TXPWR_DEFAULT_TAB

TXPWR_ST gtxpwr_tab_b[WLAN_2_4_G_CHANNEL_NUM];
TXPWR_ST gtxpwr_tab_g[WLAN_2_4_G_CHANNEL_NUM];
TXPWR_ST gtxpwr_tab_n[WLAN_2_4_G_CHANNEL_NUM];
TXPWR_ST gtxpwr_tab_n_40[WLAN_2_4_G_CHANNEL_NUM];
TXPWR_ST gtxpwr_tab_ble[BLE_2_4_G_CHANNEL_NUM];
UINT32 g_dif_g_n20 = MOD_DIST_G_BW_N20;
UINT32 g_dif_g_n40 = MOD_DIST_G_BW_N40;
UINT32 g_cur_temp = DEFAULT_TXID_THERMAL;
UINT32 g_cur_temp_flash = DEFAULT_TXID_THERMAL;
UINT32 g_lpf_cal_i = DEFAULT_TXID_LPF_CAP_I, g_lpf_cal_q = DEFAULT_TXID_LPF_CAP_Q;
UINT32 g_xtal = DEFAULT_TXID_XTAL;
static unsigned char ble_cal_bit = 0;
static UINT32 ble_diff_to_wifi = 36;

#define CALI_STATUS_PASS 1
#define CALI_STATUS_FAIL 0

typedef enum {
    CALI_STA_UNINITIALIZED = -1,
    CALI_STA_NOFOUND = 0,
    CALI_STA_OK,
    CALI_STA_EVM_FAIL,
    CALI_STA_PWR_FAIL,
} E_CALI_STATUS;
UINT32 g_cali_status = CALI_STA_UNINITIALIZED;

#if (CFG_TEMP_DETECT_VERSION == CFG_TEMP_DETECT_VERSION1)
typedef struct xtal_cali_st {
    INT8 xtal_c_delta;
    INT16 init_xtal;
    INT16 last_xtal;
} XTAL_CALI_ST, *XTAL_CALI_PTR;

XTAL_CALI_ST g_xcali = {0};
#endif

extern UINT32 ble_in_dut_mode(void);
extern uint8 is_rf_switch_to_ble(void);
static int manual_cal_fit_txpwr_tab_n_20(UINT32 differ);
static int manual_cal_fit_txpwr_tab_n_40(UINT32 differ);
static void manual_cal_adjust_fitting(TXPWR_PTR ptr, UINT16 dist);
UINT32 manual_cal_g_rfcali_status(void);

extern void manual_cal_init_xtal_cali(UINT16 init_temp, UINT16 init_xtal);

#define RF_CALI_FLAG_SET0 (0x1)
#define RF_CALI_FLAG_SET1 (0x2)
#define RF_CALI_MODE_SET0 (0x4)
#define RF_CALI_FLAG (RF_CALI_FLAG_SET0 | RF_CALI_FLAG_SET1)
#define RF_CALI_MASK (0x7)
UINT32 g_rfcali_mode = 0;

void manual_cal_set_setp0(void) {
    g_rfcali_mode |= RF_CALI_FLAG_SET0;
}

void manual_cal_set_setp1(void) {
    g_rfcali_mode |= RF_CALI_FLAG_SET1;
}

void manual_cal_set_rfcal_step0(void) {
    g_rfcali_mode &= ~RF_CALI_MASK;
    g_rfcali_mode |= RF_CALI_MODE_SET0;
}

void manual_cal_clear_setp(void) {
    g_rfcali_mode &= ~RF_CALI_MASK;
}

static UINT32 manual_cal_is_in_rfcali_mode(void) {
    // include rf calibration and test
    return ((g_rfcali_mode & RF_CALI_FLAG) == RF_CALI_FLAG) ? 1 : 0;
}

UINT32 manual_cal_is_in_rftest_mode(void) {
    if (g_rfcali_mode == 0) {
        // no thing happened, do "txevm" to send pkt, use vitual
        return 1;
    } else if ((g_rfcali_mode & RF_CALI_MASK) == RF_CALI_MASK) {
        // rfcali mode after save cali result, do test
        return 1;
    } else {
        // rfcali mode, before save result,need use ture value
        // such as txevm -g 0,
        return 0;
    }
}

UINT32 g_cmtag_flag = LOAD_FROM_CALI;
int manual_set_cmtag(UINT32 status) {
    g_cmtag_flag = status;
    return g_cmtag_flag;
}

int manual_cal_need_load_cmtag_from_flash(void) {
    return (g_cmtag_flag == LOAD_FROM_FLASH) ? 1 : 0;
}

void manual_cal_save_txpwr(UINT32 rate, UINT32 channel, UINT32 pwr_gain) {
    TXPWR_PTR txpwr_tab_ptr = NULL;
    UINT8 is_g_rate = 0;

    // for ble
    if (rate == EVM_DEFUALT_BLE_RATE) {
        if (channel > BLE_2_4_G_CHANNEL_NUM) {
            debug_print("ble wrong channel:%d\n", channel);
            return;
        }

        txpwr_tab_ptr = &gtxpwr_tab_ble[channel];

        SET_TXPWR_GAIN(txpwr_tab_ptr, pwr_gain);
        SET_TXPWR_FLAG(txpwr_tab_ptr, TXPWR_ELEM_INUSED);

        if (channel == 0) {
            ble_cal_bit |= 0x1;
        } else if (channel == 19) {
            ble_cal_bit |= 0x2;
        } else if (channel == 39) {
            ble_cal_bit |= 0x4;
        }

        debug_print("save c:%d, gain:%d\n", channel, pwr_gain);

        return;
    }

    // for wlan wifi
    if ((channel == 0) || (channel > WLAN_2_4_G_CHANNEL_NUM)) {
        debug_print("Manual cal wrong channel:%d\n", channel);
        return;
    }

    channel -= 1;

    if ((rate <= MAX_RATE_FOR_B_5M) || (rate == MAX_RATE_FOR_B)) {
        txpwr_tab_ptr = &gtxpwr_tab_b[channel];
    } else if (rate <= MAX_RATE_FOR_G) {
        txpwr_tab_ptr = &gtxpwr_tab_g[channel];
        is_g_rate = 1;
    } else if (rate <= MAX_RATE_FOR_N && rate >= MIN_RATE_FOR_N) {
        txpwr_tab_ptr = &gtxpwr_tab_n_40[channel];
    } else {
        debug_print("Manual cal wrong rate:%d\n", rate);
        return;
    }

    SET_TXPWR_GAIN(txpwr_tab_ptr, pwr_gain);
    SET_TXPWR_FLAG(txpwr_tab_ptr, TXPWR_ELEM_INUSED);

    if (is_g_rate) {
        // need save n20
        os_memcpy(&gtxpwr_tab_n[channel], &gtxpwr_tab_g[channel], sizeof(TXPWR_ST));
        manual_cal_adjust_fitting(&gtxpwr_tab_n[channel], g_dif_g_n20);
        debug_print("fit n20 tab with dist:%d\n", g_dif_g_n20);
    }
}

#if CFG_SUPPORT_LOW_MAX_CURRENT                          //Max default work current<250mA@3.3V
const UINT16 shift_tab_b[4] = {0, 0, 0, 0};              // 11M base,5.5M,2M,1M
const UINT16 shift_tab_g[8] = {0, 2, 4, 4, 4, 4, 4, 4};  // 54M base -  6M
const UINT16 shift_tab_n[1] = {0};                       // for MCS7
#elif (CFG_SOC_NAME == SOC_BK7231N)
const UINT16 shift_tab_b[4] = {0, 0, 0, 0};  // 11M base,5.5M,2M,1M
// 54M base -                 54M,48M,36M,24M,18M,12M,9M,6M
const UINT16 shift_tab_g[8] = {0, 2, 2, 2, 3, 3, 4, 4 /*4*/};  // 54M base -  12M,9M,6M//do
const UINT16 shift_tab_n20[8] = {0, 2, 2, 2, 3, 3, 4, 4 /*4*/};
;  // n20 mcs7base -  mcs0

const UINT16 shift_tab_n40[8] = {0, 2, 2, 2, 3, 3, 4, 4 /*4*/};  // n40 mcs7base -  mcs0
#else
const UINT16 shift_tab_b[4] = {0, 0, 0, 0};  // 11M base,5.5M,2M,1M
// 54M base -                 54M,48M,36M,24M,18M,12M,9M,6M
const UINT16 shift_tab_g[8] = {0, 1, 1, 1, 2, 2, 4, 6 /*4*/};  // 54M base -  12M,9M,6M
const UINT16 shift_tab_n20[8] = {0, 1, 1, 2, 2, 4, 4, 6 /*4*/};
;  // n20 mcs7base -  mcs0

const UINT16 shift_tab_n40[8] = {0, 1, 1, 2, 2, 4, 4, 6 /*4*/};  // n40 mcs7base -  mcs0
#endif

#define PWR_INIT_VAL 17
const INT8 pwr_idx_2_dbm[] = {
    PWR_INIT_VAL,  // 00  1Mbps
    17,            // 01  2Mbps
    17,            // 02  5.5Mbps
    17,            // 03  11Mbps

    17,  // 04  6Mbps
    17,  // 05  9Mbps
    17,  // 06  12Mbps
    17,  // 07  18Mbps
    16,  // 08  24Mbps
    15,  // 09  36Mbps
    14,  // 10  48Mbps
    14,  // 11  54Mbps

    17,  // 12  mcs0 N20
    17,  // 13  mcs1 N20
    17,  // 14  mcs2 N20
    16,  // 15  mcs3 N20
    15,  // 16  mcs4 N20
    14,  // 17  mcs5 N20
    13,  // 18  mcs6 N20
    13,  // 19  mcs7 N20

    15,  // 20  mcs0 N40
    15,  // 21  mcs1 N40
    15,  // 22  mcs2 N40
    14,  // 23  mcs3 N40
    13,  // 24  mcs4 N40
    13,  // 25  mcs5 N40
    12,  // 26  mcs6 N40
    12   // 27  mcs7 N40
};

INT8 g_cur_txpwr_dbm = PWR_INIT_VAL;

INT8 manual_cal_get_dbm_by_rate(UINT32 rate, UINT32 bandwidth) {
    const INT8* dbm_ptr;

    if (bandwidth == PHY_CHNL_BW_20) {
        if (rate <= 3) {
            dbm_ptr = &pwr_idx_2_dbm[rate];
        } else if (rate <= 11) {  // for g: 6-54M -- 4-11
            dbm_ptr = &pwr_idx_2_dbm[rate];
        } else if ((rate >= 128) && (rate <= 135)) {  // for n20 MCS0-7
            dbm_ptr = &pwr_idx_2_dbm[12 + rate - 128];
        } else {
            debug_print("\n manual_cal_get_dbm_by_rate err %d\n", rate);
            return 0;
        }
    } else {
        if ((rate >= 128) && (rate <= 135)) {
            dbm_ptr = &pwr_idx_2_dbm[20 + rate - 128];
        } else {
            debug_print("\n manual_cal_get_dbm_by_rate err %d\n", rate);
            return 0;
        }
    }

    g_cur_txpwr_dbm = *dbm_ptr;

    return *dbm_ptr;
}

INT8 manual_cal_get_cur_txpwr_dbm(void) {
    return g_cur_txpwr_dbm;
}

UINT32 manual_cal_get_pwr_idx_shift(UINT32 rate, UINT32 bandwidth, UINT32* pwr_gain) {
    const UINT16* shift;
    UINT32 idex, ret = 2;

    if (bandwidth == PHY_CHNL_BW_20) {
        if (rate <= 3) {
            shift = shift_tab_b;
            idex = 3 - rate;
            ret = 1;
        } else if (rate <= 11) {  // for g: 6-54M -- 3-11
            shift = shift_tab_g;
            idex = 11 - rate;
        } else if ((rate >= 128) && (rate <= 135)) {  // for n20 MCS0-7
            shift = shift_tab_n20;
            idex = 135 - rate;
        } else {
            debug_print("\nget_pwr_idx_shift wrong rate:%d\n", rate);
            return 0;
        }
    } else {
        if ((rate >= 128) && (rate <= 135)) {
            shift = shift_tab_n40;
            idex = 135 - rate;
        } else {
            debug_print("\nget_pwr_idx_shift wrong rate:%d\n", rate);
            return 0;
        }
    }

    //debug_print("get_pwr_info: idx: %d, pwr:%d\n", shift[idex], *pwr_gain);remove for midea

    idex = shift[idex] + *pwr_gain;
#if (CFG_SOC_NAME == SOC_BK7231N)
    *pwr_gain = (idex > 79) ? 79 : idex;
#else
    *pwr_gain = (idex > 31) ? 31 : idex;
#endif
    //debug_print("--pwr:%d\n", *pwr_gain);remove for midea

    return ret;
}

int manual_cal_get_txpwr(UINT32 rate, UINT32 channel, UINT32 bandwidth, UINT32* pwr_gain) {
    TXPWR_PTR txpwr_tab_ptr = NULL;

    // for ble
    if (rate == EVM_DEFUALT_BLE_RATE) {
        if (channel > BLE_2_4_G_CHANNEL_NUM) {
            debug_print("ble wrong channel:%d\n", channel);
            return 0;
        }

        txpwr_tab_ptr = &gtxpwr_tab_ble[channel];

        *pwr_gain = GET_TXPWR_GAIN(txpwr_tab_ptr);
        return 1;
    }

    // for wlan wifi
    if ((channel == 0) || (channel > WLAN_2_4_G_CHANNEL_NUM)) {
        debug_print("Manual cal wrong channel:%d\n", channel);
        return 0;
    }

    if ((bandwidth != PHY_CHNL_BW_20) && (bandwidth != PHY_CHNL_BW_40)) {
        debug_print("Manual cal wrong bandwidth:%d\n", bandwidth);
        return 0;
    }

    channel -= 1;

    if (bandwidth == PHY_CHNL_BW_20) {
        if ((rate <= MAX_RATE_FOR_B_5M) || (rate == MAX_RATE_FOR_B)) {
            txpwr_tab_ptr = &gtxpwr_tab_b[channel];
        } else if (rate <= MAX_RATE_FOR_G) {
            txpwr_tab_ptr = &gtxpwr_tab_g[channel];
        } else if ((rate <= MAX_RATE_FOR_N) && (rate >= MIN_RATE_FOR_N)) {
            txpwr_tab_ptr = &gtxpwr_tab_n[channel];
        } else {
            debug_print("\nManual cal wrong rate:%d\n", rate);
            return 0;
        }
    } else {
        if ((rate <= MAX_RATE_FOR_N) && (rate >= MIN_RATE_FOR_N)) {
            txpwr_tab_ptr = &gtxpwr_tab_n_40[channel];
        } else {
            debug_print("\nManual cal wrong rate with BW40? %d:%d\n", bandwidth, rate);
            txpwr_tab_ptr = &gtxpwr_tab_n[channel];
        }
    }

    *pwr_gain = GET_TXPWR_GAIN(txpwr_tab_ptr);
    //debug_print("get txpwrtab gain:%d,ch:%d\n", *pwr_gain, channel+1); ///remove for midea

    return 1;
}

uint8_t manual_cal_get_ble_pwr_idx(uint8_t channel) {
    TXPWR_PTR txpwr_tab_ptr = NULL;

    if (channel > 39)
        channel = 39;

    txpwr_tab_ptr = &gtxpwr_tab_ble[channel];

    return (GET_TXPWR_GAIN(txpwr_tab_ptr));
}

void manual_cal_show_txpwr_tab(void) {
    TXPWR_PTR txpwr_tab_ptr = NULL;
    UINT32 i;

    debug_print("txpwr table for b:\n");
    for (i = 0; i < WLAN_2_4_G_CHANNEL_NUM; i++) {
        txpwr_tab_ptr = &gtxpwr_tab_b[i];
        debug_print("ch:%2d: value:%02x, flag:%01d, gain:%02d\n",
                    i + 1, txpwr_tab_ptr->value,
                    GET_TXPWR_FLAG(txpwr_tab_ptr), GET_TXPWR_GAIN(txpwr_tab_ptr));
    }

    debug_print("txpwr table for g:\n");
    for (i = 0; i < WLAN_2_4_G_CHANNEL_NUM; i++) {
        txpwr_tab_ptr = &gtxpwr_tab_g[i];
        debug_print("ch:%2d: value:%02x, flag:%01d, gain:%02d\n",
                    i + 1, txpwr_tab_ptr->value,
                    GET_TXPWR_FLAG(txpwr_tab_ptr), GET_TXPWR_GAIN(txpwr_tab_ptr));
    }

    debug_print("txpwr table for n20:\n");
    for (i = 0; i < WLAN_2_4_G_CHANNEL_NUM; i++) {
        txpwr_tab_ptr = &gtxpwr_tab_n[i];
        debug_print("ch:%2d: value:%02x, flag:%01d, gain:%02d\n",
                    i + 1, txpwr_tab_ptr->value,
                    GET_TXPWR_FLAG(txpwr_tab_ptr), GET_TXPWR_GAIN(txpwr_tab_ptr));
    }

    debug_print("txpwr table for n40:\n");
    for (i = 0; i < WLAN_2_4_G_CHANNEL_NUM; i++) {
        txpwr_tab_ptr = &gtxpwr_tab_n_40[i];
        debug_print("ch:%2d: value:%02x, flag:%01d, gain:%02d\n",
                    i + 1, txpwr_tab_ptr->value,
                    GET_TXPWR_FLAG(txpwr_tab_ptr), GET_TXPWR_GAIN(txpwr_tab_ptr));
    }

    debug_print("txpwr table for ble:\n");
    for (i = 0; i < BLE_2_4_G_CHANNEL_NUM; i++) {
        txpwr_tab_ptr = &gtxpwr_tab_ble[i];
        debug_print("ch:%2d: value:%02x, flag:%01d, gain:%02d\n",
                    i, txpwr_tab_ptr->value,
                    GET_TXPWR_FLAG(txpwr_tab_ptr), GET_TXPWR_GAIN(txpwr_tab_ptr));
    }

    debug_print("sys temper:%d\n", g_cur_temp);
    debug_print("sys xtal:%d\n", g_xtal);
    //debug_print("TSSI:%d\n", bk7011_get_txpwr_config_reg());

    bk7011_cal_dcormod_show();
}

TXPWR_IS_RD manual_cal_txpwr_tab_is_fitted(void) {
    return (TXPWR_TAB_B_RD | TXPWR_TAB_G_RD | TXPWR_TAB_N_RD | TXPWR_TAB_BLE);
}

static void manual_cal_adjust_fitting(TXPWR_PTR ptr, UINT16 dist) {
    UINT16 gain;
    //if(GET_TXPWR_FLAG(ptr) == TXPWR_ELEM_UNUSED)
    //    return;

    gain = GET_TXPWR_GAIN(ptr);
    gain = (gain > dist) ? gain - dist : 0;

    SET_TXPWR_GAIN(ptr, gain);
}

static void manual_cal_do_fitting_base(TXPWR_PTR dst, TXPWR_PTR srclow, TXPWR_PTR srchigh) {
    UINT16 gain = 0, dist;

    if ((GET_TXPWR_FLAG(srclow) == TXPWR_ELEM_UNUSED) || (GET_TXPWR_FLAG(srchigh) == TXPWR_ELEM_UNUSED))
        return;

    if (GET_TXPWR_GAIN(srclow) > GET_TXPWR_GAIN(srchigh)) {
        dist = GET_TXPWR_GAIN(srclow) - GET_TXPWR_GAIN(srchigh);
#if (SOC_BK7231N == CFG_SOC_NAME)
        gain = (UINT16)((float)(dist) / 3.0 + 0.5) + GET_TXPWR_GAIN(srchigh) + 1;
#else
        gain = (UINT16)((float)(dist) / 2.0) + GET_TXPWR_GAIN(srchigh);
#endif
    } else {
        dist = GET_TXPWR_GAIN(srchigh) - GET_TXPWR_GAIN(srclow);
#if (SOC_BK7231N == CFG_SOC_NAME)
        gain = (UINT16)((float)(dist) / 3.0 + 0.5) + GET_TXPWR_GAIN(srclow) + 1;
#else
        gain = (UINT16)((float)(dist) / 2.0) + GET_TXPWR_GAIN(srclow);
#endif
    }

    SET_TXPWR_GAIN(dst, gain);
    SET_TXPWR_FLAG(dst, TXPWR_ELEM_INUSED);
}

static void manual_cal_do_fitting(TXPWR_PTR dst, TXPWR_PTR srclow, TXPWR_PTR srchigh) {
    UINT16 gain = 0;

    if ((GET_TXPWR_FLAG(srclow) == TXPWR_ELEM_UNUSED) || (GET_TXPWR_FLAG(srchigh) == TXPWR_ELEM_UNUSED))
        return;

    gain = (UINT16)((float)(GET_TXPWR_GAIN(srclow) + GET_TXPWR_GAIN(srchigh)) / 2 + 0.5);
    SET_TXPWR_GAIN(dst, gain);
    SET_TXPWR_FLAG(dst, TXPWR_ELEM_INUSED);
}

void manual_cal_11b_2_ble(void) {
    TXPWR_PTR tab_ptr = NULL;
    unsigned char gain;

#if (CFG_SOC_NAME == SOC_BK7231N)
    tab_ptr = gtxpwr_tab_g;
    if (ble_cal_bit == 0) {
        if (GET_TXPWR_FLAG(&tab_ptr[0]) == TXPWR_ELEM_INUSED) {
            gain = GET_TXPWR_GAIN(&tab_ptr[0]);

            // think ble power need to be 7db
            if ((gain + 12) > ble_diff_to_wifi) {
                gain = gain + 12 - ble_diff_to_wifi;
            } else {
                gain = 1;
            }

            SET_TXPWR_GAIN(&gtxpwr_tab_ble[0], gain);
            SET_TXPWR_FLAG(&gtxpwr_tab_ble[0], TXPWR_ELEM_INUSED);
        }
        if (GET_TXPWR_FLAG(&tab_ptr[6]) == TXPWR_ELEM_INUSED) {
            gain = GET_TXPWR_GAIN(&tab_ptr[6]);

            // think ble power need to be 7db
            if ((gain + 12) > ble_diff_to_wifi) {
                gain = gain + 12 - ble_diff_to_wifi;
            } else {
                gain = 1;
            }

            SET_TXPWR_GAIN(&gtxpwr_tab_ble[19], gain);
            SET_TXPWR_FLAG(&gtxpwr_tab_ble[19], TXPWR_ELEM_INUSED);
        } else if ((GET_TXPWR_FLAG(&tab_ptr[12]) == TXPWR_ELEM_INUSED) && (GET_TXPWR_FLAG(&tab_ptr[0]) == TXPWR_ELEM_INUSED)) {
            gain = (GET_TXPWR_GAIN(&tab_ptr[0]) + GET_TXPWR_GAIN(&tab_ptr[12])) / 2;

            if ((gain + 12) > ble_diff_to_wifi) {
                gain = gain + 12 - ble_diff_to_wifi;
            } else {
                gain = 1;
            }

            SET_TXPWR_GAIN(&gtxpwr_tab_ble[19], gain);
            SET_TXPWR_FLAG(&gtxpwr_tab_ble[19], TXPWR_ELEM_INUSED);
        }
        if (GET_TXPWR_FLAG(&tab_ptr[12]) == TXPWR_ELEM_INUSED) {
            gain = GET_TXPWR_GAIN(&tab_ptr[12]);

            // think ble power need to be 7db
            if ((gain + 12) > ble_diff_to_wifi) {
                gain = gain + 12 - ble_diff_to_wifi;
            } else {
                gain = 1;
            }

            SET_TXPWR_GAIN(&gtxpwr_tab_ble[39], gain);
            SET_TXPWR_FLAG(&gtxpwr_tab_ble[39], TXPWR_ELEM_INUSED);
        }
    } else if ((ble_cal_bit & 0x2) == 0x2) {
        if ((ble_cal_bit & 0x1) == 0) {
            if ((GET_TXPWR_FLAG(&tab_ptr[0]) == TXPWR_ELEM_INUSED) && (GET_TXPWR_FLAG(&tab_ptr[6]) == TXPWR_ELEM_INUSED)) {
                gain = (char)GET_TXPWR_GAIN(&gtxpwr_tab_ble[19]) - ((char)GET_TXPWR_GAIN(&tab_ptr[6]) - (char)GET_TXPWR_GAIN(&tab_ptr[0]));
                SET_TXPWR_GAIN(&gtxpwr_tab_ble[0], gain);
                SET_TXPWR_FLAG(&gtxpwr_tab_ble[0], TXPWR_ELEM_INUSED);
            }
        }
        if ((ble_cal_bit & 0x4) == 0) {
            if ((GET_TXPWR_FLAG(&tab_ptr[6]) == TXPWR_ELEM_INUSED) && (GET_TXPWR_FLAG(&tab_ptr[12]) == TXPWR_ELEM_INUSED)) {
                gain = (char)GET_TXPWR_GAIN(&gtxpwr_tab_ble[19]) - ((char)GET_TXPWR_GAIN(&tab_ptr[6]) - (char)GET_TXPWR_GAIN(&tab_ptr[12]));
                SET_TXPWR_GAIN(&gtxpwr_tab_ble[39], gain);
                SET_TXPWR_FLAG(&gtxpwr_tab_ble[39], TXPWR_ELEM_INUSED);
            }
        }
    }

#else
    if (ble_cal_bit == 0) {
        tab_ptr = gtxpwr_tab_b;
        if ((GET_TXPWR_FLAG(&tab_ptr[0]) == TXPWR_ELEM_INUSED) && (GET_TXPWR_FLAG(&tab_ptr[12]) == TXPWR_ELEM_INUSED)) {
            gain = (GET_TXPWR_GAIN(&tab_ptr[0]) + GET_TXPWR_GAIN(&tab_ptr[12])) / 2;
            if ((gain > 20) && (gain <= 31)) {
                gain -= 9;
            } else if ((gain >= 15) && (gain <= 20)) {
                gain -= 7;
            } else {
                gain -= 5;
            }
            SET_TXPWR_GAIN(&gtxpwr_tab_ble[19], gain);
            SET_TXPWR_FLAG(&gtxpwr_tab_ble[19], TXPWR_ELEM_INUSED);
        }
    }
#endif
}

UINT32 manual_cal_fitting_txpwr_tab(void) {
    UINT32 ret = 0, i;
    TXPWR_PTR tab_ptr = NULL;

    // for b, check ch1, ch7, ch13 is in used
    tab_ptr = gtxpwr_tab_b;
    if ((GET_TXPWR_FLAG(&tab_ptr[0]) == TXPWR_ELEM_UNUSED) || (GET_TXPWR_FLAG(&tab_ptr[12]) == TXPWR_ELEM_UNUSED)) {
        debug_print("txpwr table for b fitting failed!, ch1 ch13 unused\n");
    } else {
        if (GET_TXPWR_FLAG(&tab_ptr[6]) == TXPWR_ELEM_UNUSED) {
            // fitting ch7, use ch1, ch13
            manual_cal_do_fitting_base(&tab_ptr[6], &tab_ptr[0], &tab_ptr[12]);
        }
        // fitting ch4, use ch1, ch7
        manual_cal_do_fitting(&tab_ptr[3], &tab_ptr[0], &tab_ptr[6]);
        // fitting ch2, use ch1, ch4
        manual_cal_do_fitting(&tab_ptr[1], &tab_ptr[0], &tab_ptr[3]);
        // fitting ch3, use ch2, ch4
        manual_cal_do_fitting(&tab_ptr[2], &tab_ptr[1], &tab_ptr[3]);
        // fitting ch5, use ch4, ch7
        manual_cal_do_fitting(&tab_ptr[4], &tab_ptr[3], &tab_ptr[6]);
        // fitting ch6, use ch5, ch7
        manual_cal_do_fitting(&tab_ptr[5], &tab_ptr[4], &tab_ptr[6]);

        // fitting ch10, use ch7, ch13
        manual_cal_do_fitting(&tab_ptr[9], &tab_ptr[6], &tab_ptr[12]);
        // fitting ch8, use ch7, ch10
        manual_cal_do_fitting(&tab_ptr[7], &tab_ptr[6], &tab_ptr[9]);
        // fitting ch9, use ch8, ch10
        manual_cal_do_fitting(&tab_ptr[8], &tab_ptr[7], &tab_ptr[9]);
        // fitting ch11, use ch10, ch13
        manual_cal_do_fitting(&tab_ptr[10], &tab_ptr[9], &tab_ptr[12]);
        // fitting ch12, use ch11, ch13
        manual_cal_do_fitting(&tab_ptr[11], &tab_ptr[10], &tab_ptr[12]);

        // fitting ch14, the same as ch13
        manual_cal_do_fitting(&tab_ptr[13], &tab_ptr[12], &tab_ptr[12]);

        // clear flag, only base change set used
        for (i = 0; i < WLAN_2_4_G_CHANNEL_NUM; i++) {
#if (CFG_SOC_NAME == SOC_BK7231N)
            if ((i == 0) || (i == 12) || (i == 6))
#else
            if ((i == 0) || (i == 12))
#endif
                continue;
            SET_TXPWR_FLAG(&tab_ptr[i], TXPWR_ELEM_UNUSED);
        }
        ret = 1;
    }

    // for g
    tab_ptr = gtxpwr_tab_g;
    if ((GET_TXPWR_FLAG(&tab_ptr[0]) == TXPWR_ELEM_UNUSED) || (GET_TXPWR_FLAG(&tab_ptr[12]) == TXPWR_ELEM_UNUSED)) {
        debug_print("txpwr table for g fitting failed!, ch1 ch13 unused\n");
    } else {
        if (GET_TXPWR_FLAG(&tab_ptr[6]) == TXPWR_ELEM_UNUSED) {
            // fitting ch7, use ch1, ch13
            manual_cal_do_fitting_base(&tab_ptr[6], &tab_ptr[0], &tab_ptr[12]);
        }

        // fitting ch4, use ch1, ch7
        manual_cal_do_fitting(&tab_ptr[3], &tab_ptr[0], &tab_ptr[6]);
        // fitting ch2, use ch1, ch4
        manual_cal_do_fitting(&tab_ptr[1], &tab_ptr[0], &tab_ptr[3]);
        // fitting ch3, use ch2, ch4
        manual_cal_do_fitting(&tab_ptr[2], &tab_ptr[1], &tab_ptr[3]);
        // fitting ch5, use ch4, ch7
        manual_cal_do_fitting(&tab_ptr[4], &tab_ptr[3], &tab_ptr[6]);
        // fitting ch6, use ch5, ch7
        manual_cal_do_fitting(&tab_ptr[5], &tab_ptr[4], &tab_ptr[6]);

        // fitting ch10, use ch7, ch13
        manual_cal_do_fitting(&tab_ptr[9], &tab_ptr[6], &tab_ptr[12]);
        // fitting ch8, use ch7, ch10
        manual_cal_do_fitting(&tab_ptr[7], &tab_ptr[6], &tab_ptr[9]);
        // fitting ch9, use ch8, ch10
        manual_cal_do_fitting(&tab_ptr[8], &tab_ptr[7], &tab_ptr[9]);
        // fitting ch11, use ch10, ch13
        manual_cal_do_fitting(&tab_ptr[10], &tab_ptr[9], &tab_ptr[12]);
        // fitting ch12, use ch11, ch13
        manual_cal_do_fitting(&tab_ptr[11], &tab_ptr[10], &tab_ptr[12]);

        // fitting ch14, the same as ch13
        manual_cal_do_fitting(&tab_ptr[13], &tab_ptr[12], &tab_ptr[12]);

        // clear flag, only base change set used
        for (i = 0; i < WLAN_2_4_G_CHANNEL_NUM; i++) {
#if (CFG_SOC_NAME == SOC_BK7231N)
            if ((i == 0) || (i == 12) || (i == 6))
#else
            if ((i == 0) || (i == 12))
#endif
                continue;
            SET_TXPWR_FLAG(&tab_ptr[i], TXPWR_ELEM_UNUSED);
        }

        // fit n20 synch
        manual_cal_fit_txpwr_tab_n_20(g_dif_g_n20);

        ret = 1;
    }

    // for n 40
    tab_ptr = gtxpwr_tab_n_40;
    if ((GET_TXPWR_FLAG(&tab_ptr[2]) == TXPWR_ELEM_UNUSED)  // ch3
#if (CFG_SOC_NAME == SOC_BK7231N)
        || (GET_TXPWR_FLAG(&tab_ptr[6]) == TXPWR_ELEM_UNUSED)  // ch7
#endif
        || (GET_TXPWR_FLAG(&tab_ptr[10]) == TXPWR_ELEM_UNUSED)) {  // ch11
        debug_print("txpwr table for n40 ch3 ch11 unused, use g for fitting\n");

        // fit n40 synch
        manual_cal_fit_txpwr_tab_n_40(g_dif_g_n40);
    } else {
        if (GET_TXPWR_FLAG(&tab_ptr[6]) == TXPWR_ELEM_UNUSED) {
            // fitting ch7, use ch1, ch13
            manual_cal_do_fitting_base(&tab_ptr[6], &tab_ptr[2], &tab_ptr[10]);
        }

        // fitting ch4, use ch3, ch7
        manual_cal_do_fitting(&tab_ptr[3], &tab_ptr[2], &tab_ptr[6]);
        // fitting ch2, the same as ch3
        manual_cal_do_fitting(&tab_ptr[1], &tab_ptr[2], &tab_ptr[2]);
        // fitting ch1, the same as ch3
        manual_cal_do_fitting(&tab_ptr[0], &tab_ptr[2], &tab_ptr[2]);
        // fitting ch5, use ch4, ch7
        manual_cal_do_fitting(&tab_ptr[4], &tab_ptr[3], &tab_ptr[6]);
        // fitting ch6, use ch5, ch7
        manual_cal_do_fitting(&tab_ptr[5], &tab_ptr[4], &tab_ptr[6]);

        // fitting ch10, use ch7, ch11
        manual_cal_do_fitting(&tab_ptr[9], &tab_ptr[6], &tab_ptr[10]);
        // fitting ch8, use ch7, ch10
        manual_cal_do_fitting(&tab_ptr[7], &tab_ptr[6], &tab_ptr[9]);
        // fitting ch9, use ch8, ch10
        manual_cal_do_fitting(&tab_ptr[8], &tab_ptr[7], &tab_ptr[9]);

        // fitting ch12, the same as ch11
        manual_cal_do_fitting(&tab_ptr[11], &tab_ptr[10], &tab_ptr[10]);
        // fitting ch13, the same as ch11
        manual_cal_do_fitting(&tab_ptr[12], &tab_ptr[10], &tab_ptr[10]);
        // fitting ch14, the same as ch11
        manual_cal_do_fitting(&tab_ptr[13], &tab_ptr[10], &tab_ptr[10]);
        ret = 1;

        // clear flag, only base change set used
        for (i = 0; i < WLAN_2_4_G_CHANNEL_NUM; i++) {
            //n40 no need set used flag
            //because n40 is option,
            // before txevm -e 2, if save n40, then do fit, if no, fit from g
#if 0  //(CFG_SOC_NAME == SOC_BK7231N)
            if((i==2) || (i==10)||(i==6))
               continue;
#endif
            SET_TXPWR_FLAG(&tab_ptr[i], TXPWR_ELEM_UNUSED);
        }
    }

    // for ble
    tab_ptr = gtxpwr_tab_ble;
    if ((GET_TXPWR_FLAG(&tab_ptr[0]) == TXPWR_ELEM_INUSED)       // ch0
        || (GET_TXPWR_FLAG(&tab_ptr[19]) == TXPWR_ELEM_INUSED)   // ch19
        || (GET_TXPWR_FLAG(&tab_ptr[39]) == TXPWR_ELEM_INUSED))  // ch39
    {
        UINT32 flag = 0;
        TXPWR_PTR base = NULL;
        debug_print("txpwr table for ble ch0/19/39 inused\n");

        if (GET_TXPWR_FLAG(&tab_ptr[0]) == TXPWR_ELEM_INUSED) {
            flag |= 0x01;
            base = &tab_ptr[0];
        }
        if (GET_TXPWR_FLAG(&tab_ptr[19]) == TXPWR_ELEM_INUSED) {
            flag |= 0x02;
            base = &tab_ptr[19];
        }
        if (GET_TXPWR_FLAG(&tab_ptr[39]) == TXPWR_ELEM_INUSED) {
            flag |= 0x04;
            base = &tab_ptr[39];
        }
        printf("0x%x\n", flag);
        if (flag != 0x7) {
            // only ch19 do fit
            if (flag == 0x02) {
                // ch0 = ch19 + 4
                TXPWR_PTR ptr;
                base = &tab_ptr[19];
                ptr = &tab_ptr[0];
                SET_TXPWR_GAIN(ptr, (GET_TXPWR_GAIN(base) + 4));
                SET_TXPWR_FLAG(ptr, TXPWR_ELEM_INUSED);

                // fitting ch10, use ch0, ch19
                manual_cal_do_fitting(&tab_ptr[10], &tab_ptr[0], &tab_ptr[19]);

                // fitting ch5, use ch0, ch10
                manual_cal_do_fitting(&tab_ptr[5], &tab_ptr[0], &tab_ptr[10]);

                // fitting ch15, use ch10, ch19
                manual_cal_do_fitting(&tab_ptr[15], &tab_ptr[10], &tab_ptr[19]);
                for (i = 0; i < BLE_2_4_G_CHANNEL_NUM; i++) {
                    if (i == 0)
                        base = &tab_ptr[0];
                    else if (i == 5)
                        base = &tab_ptr[5];
                    else if (i == 10)
                        base = &tab_ptr[10];
                    else if (i == 15)
                        base = &tab_ptr[15];
                    else if (i == 19) {
                        base = &tab_ptr[19];
                        continue;
                    }

                    // ch19 -ch39 are same with ch20
                    os_memcpy(&tab_ptr[i], base, sizeof(TXPWR_ST));
                    SET_TXPWR_FLAG(&tab_ptr[i], TXPWR_ELEM_UNUSED);
                }
            } else {
                // clear flag to default set
                TXPWR_PTR ptr = &tab_ptr[0];
                SET_TXPWR_FLAG(ptr, TXPWR_ELEM_UNUSED);

                ptr = &tab_ptr[19];
                SET_TXPWR_FLAG(ptr, TXPWR_ELEM_INUSED);

                ptr = &tab_ptr[39];
                SET_TXPWR_FLAG(ptr, TXPWR_ELEM_UNUSED);
            }
        } else {
            // fitting ch10, use ch0, ch19
            manual_cal_do_fitting(&tab_ptr[10], &tab_ptr[0], &tab_ptr[19]);

            // fitting ch5, use ch0, ch10
            manual_cal_do_fitting(&tab_ptr[5], &tab_ptr[0], &tab_ptr[10]);

            // fitting ch15, use ch10, ch19
            manual_cal_do_fitting(&tab_ptr[15], &tab_ptr[10], &tab_ptr[19]);

            // fitting ch30, use ch19, ch39
            manual_cal_do_fitting(&tab_ptr[30], &tab_ptr[19], &tab_ptr[39]);

            // fitting ch25, use ch19, ch30
            manual_cal_do_fitting(&tab_ptr[25], &tab_ptr[19], &tab_ptr[30]);

            // fitting ch35, use ch30, ch39
            manual_cal_do_fitting(&tab_ptr[35], &tab_ptr[30], &tab_ptr[39]);

            for (i = 0; i < BLE_2_4_G_CHANNEL_NUM; i++) {
                if (i == 0)
                    base = &tab_ptr[0];
                else if (i == 5)
                    base = &tab_ptr[5];
                else if (i == 10)
                    base = &tab_ptr[10];
                else if (i == 15)
                    base = &tab_ptr[15];
                else if (i == 19) {
                    base = &tab_ptr[19];
                    continue;
                } else if (i == 25)
                    base = &tab_ptr[25];
                else if (i == 30)
                    base = &tab_ptr[30];
                else if (i == 35)
                    base = &tab_ptr[35];
                else if (i == 39)
                    base = &tab_ptr[39];

                os_memcpy(&tab_ptr[i], base, sizeof(TXPWR_ST));
                SET_TXPWR_FLAG(&tab_ptr[i], TXPWR_ELEM_UNUSED);
            }

            //adjust ble power manually after fitting
            //add 1db for ch23~24 ch31~34
            {
                UINT8 value;

                base = &tab_ptr[23];
                value = GET_TXPWR_GAIN(base);
                SET_TXPWR_GAIN(base, (value + 4));

                base = &tab_ptr[24];
                value = GET_TXPWR_GAIN(base);
                SET_TXPWR_GAIN(base, (value + 4));

                base = &tab_ptr[31];
                value = GET_TXPWR_GAIN(base);
                SET_TXPWR_GAIN(base, (value + 4));

                base = &tab_ptr[32];
                value = GET_TXPWR_GAIN(base);
                SET_TXPWR_GAIN(base, (value + 4));

                base = &tab_ptr[33];
                value = GET_TXPWR_GAIN(base);
                SET_TXPWR_GAIN(base, (value + 4));

                base = &tab_ptr[34];
                value = GET_TXPWR_GAIN(base);
                SET_TXPWR_GAIN(base, (value + 4));
            }
        }
    } else {
        debug_print("txpwr table ble ch0/19/39 none one unused, use def\n");
    }

    return ret;
}

static int manual_cal_fit_txpwr_tab_n_20(UINT32 differ) {
    TXPWR_PTR tab_ptr_temp, tab_ptr = NULL;
    UINT32 i = 0;

    tab_ptr = gtxpwr_tab_n;
    tab_ptr_temp = gtxpwr_tab_g;

    for (i = 0; i < WLAN_2_4_G_CHANNEL_NUM; i++) {
        os_memcpy(&tab_ptr[i], &tab_ptr_temp[i], sizeof(TXPWR_ST));
        manual_cal_adjust_fitting(&tab_ptr[i], differ);
    }

    return 0;
}

static int manual_cal_fit_txpwr_tab_n_40(UINT32 differ) {
    TXPWR_PTR tab_ptr_temp, tab_ptr = NULL;
    UINT32 i = 0;

    tab_ptr = gtxpwr_tab_n_40;
    tab_ptr_temp = gtxpwr_tab_g;

    for (i = 0; i < WLAN_2_4_G_CHANNEL_NUM; i++) {
        os_memcpy(&tab_ptr[i], &tab_ptr_temp[i], sizeof(TXPWR_ST));
        manual_cal_adjust_fitting(&tab_ptr[i], differ);
    }
    return 0;
}

void manual_cal_set_dif_g_n20(UINT32 diff) {
    if (diff == g_dif_g_n20) {
        debug_print("no need to do, same with g_dif_g_n20:%d\n", g_dif_g_n20);
        return;
    }

    if (diff > 31)
        diff = 31;

    g_dif_g_n20 = (diff & GAIN_MASK);
    manual_cal_fit_txpwr_tab_n_20(g_dif_g_n20);
}

void manual_cal_set_dif_g_n40(UINT32 diff) {
    if (diff == g_dif_g_n40) {
        debug_print("no need to do, same with g_dif_g_n40:%d\n", g_dif_g_n40);
        return;
    }

    if (diff > 31)
        diff = 31;

    g_dif_g_n40 = (diff & GAIN_MASK);
    manual_cal_fit_txpwr_tab_n_40(g_dif_g_n40);
}

void manual_cal_set_dif_ble(UINT32 diff) {
    ble_diff_to_wifi = diff;
}

////////////////////////////////////////////////////////////////////////////////
static UINT32 manual_cal_search_opt_tab(UINT32* len) {
    UINT32 ret = 0, status;
    DD_HANDLE flash_handle;
    TXPWR_ELEM_ST head;
#if CFG_SUPPORT_ALIOS
    hal_logic_partition_t* pt = hal_flash_get_info(HAL_PARTITION_RF_FIRMWARE);
#else
    bk_logic_partition_t* pt = bk_flash_get_info(BK_PARTITION_RF_FIRMWARE);
#endif

    *len = 0;
    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), pt->partition_start_addr);  //TXPWR_TAB_FLASH_ADDR);
    if (BK_FLASH_OPT_TLV_HEADER == head.type) {
        *len = head.len + sizeof(TAG_TXPWR_ST);
        ret = 1;
    }
    ddev_close(flash_handle);

    return ret;
}

static UINT32 manual_cal_search_txpwr_tab(UINT32 type, UINT32 start_addr) {
    UINT32 status, addr, end_addr;
    DD_HANDLE flash_handle;
    TXPWR_ELEM_ST head;

    status = manual_cal_search_opt_tab(&addr);
    if (!status) {
        return 0;
    }

    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), start_addr);
    addr = start_addr + sizeof(TXPWR_ELEM_ST);
    end_addr = addr + head.len;
    while (addr < end_addr) {
        ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
        if (type == head.type) {
            break;
        } else {
            addr += sizeof(TXPWR_ELEM_ST);
            addr += head.len;
        }
    }

    if (addr >= end_addr) {
        addr = 0;
    }
    ddev_close(flash_handle);

    return addr;
}

static UINT8 manual_cal_update_flash_area(UINT32 addr_offset, char* buf, UINT32 len) {
    DD_HANDLE flash_handle = DD_HANDLE_UNVALID;
    UINT32 param, status, write_len, write_addr, flash_len;
    UINT8 ret = 0;
    UINT8* read_buf = NULL;
    UINT8 check_times = BK_FLASH_WRITE_CHECK_TIMES;
#if CFG_SUPPORT_ALIOS
    hal_logic_partition_t* pt = hal_flash_get_info(HAL_PARTITION_RF_FIRMWARE);
#else
    bk_logic_partition_t* pt = bk_flash_get_info(BK_PARTITION_RF_FIRMWARE);
#endif

    if (addr_offset > (BK_FLASH_SECTOR_SIZE)) {
        debug_print("write flash addroffset error:%08x\n", addr_offset);
        return 1;
    }
    if (len == 0)
        return 0;

    status = manual_cal_search_opt_tab(&flash_len);
    if (status && (flash_len >= (addr_offset + len))) {
        // read all flash otp
        write_len = flash_len;
    } else {
        write_len = addr_offset + len;
    }

    write_addr = pt->partition_start_addr;  //TXPWR_TAB_FLASH_ADDR;

    read_buf = (UINT8*)os_malloc(write_len);
    if (!read_buf) {
        debug_print("cann't malloc buf for flash write\n");
        ret = 1;
        goto updata_exit;
    }

    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);

write_again:
    if (addr_offset) {
        status = ddev_read(flash_handle, (char*)read_buf, write_len, write_addr);
        if (status != FLASH_SUCCESS) {
            debug_print("cann't read flash before write\n");
            ret = 1;
            goto updata_exit;
        }
        if (0 == os_memcmp(read_buf + addr_offset, buf, len)) {
            debug_print("same value ignore update\n");
            goto updata_exit;
        }
    }

    os_memcpy(read_buf + addr_offset, buf, len);

    hal_flash_lock();

#if CFG_SUPPORT_ALIOS
    hal_flash_dis_secure(0, 0, 0);
#else
    bk_flash_enable_security(FLASH_PROTECT_NONE);
#endif

    param = write_addr;
    ddev_control(flash_handle, CMD_FLASH_ERASE_SECTOR, &param);

    status = ddev_write(flash_handle, (char*)read_buf, write_len, write_addr);

    if (status != FLASH_SUCCESS) {
        debug_print("save txpwr tab to flash failed\n");
        ret = 1;
        hal_flash_unlock();
        goto updata_exit;
    }

#if CFG_SUPPORT_ALIOS
    hal_flash_enable_secure(0, 0, 0);
#else
    bk_flash_enable_security(FLASH_PROTECT_ALL);
#endif
    hal_flash_unlock();

    {
        UINT8 *check_buf = read_buf, *org_buf = (UINT8*)buf;
        UINT32 check_addr = write_addr + addr_offset;
        UINT8 check_result;

        os_memset(check_buf, 0, len);

        status = ddev_read(flash_handle, (char*)check_buf, len, check_addr);
        if (status != FLASH_SUCCESS) {
            debug_print("cann't read flash in check\n");
            ret = 1;
            goto updata_exit;
        }

        check_result = 1;
        for (int i = 0; i < len; i++) {
            if (check_buf[i] != org_buf[i]) {
                check_result = 0;
                break;
            }
        }

        if (check_result) {
//debug_print("\nshow txpwr tags before write flash:\n");
#if 1
            for (int i = 0; i < len; i++) {
                null_prf("%02x,", org_buf[i]);
                if ((i + 1) % 16 == 0)
                    null_prf("\n");
            }
            null_prf("\n");
#endif
            debug_print("manual_cal_write_flash ok\n");

        } else {
            check_times--;
            if (check_times)
                goto write_again;
            else
                debug_print("manual_cal_write_flash failed\n");
        }
    }

updata_exit:
    if (flash_handle != DD_HANDLE_UNVALID)
        ddev_close(flash_handle);

    if (read_buf)
        os_free(read_buf);

    return ret;
}

static UINT8 manual_cal_read_flash(UINT32 addr_offset, char* buf, UINT32 len) {
    UINT32 status, addr;
    UINT8 ret = 0;
    DD_HANDLE flash_handle;
#if CFG_SUPPORT_ALIOS
    hal_logic_partition_t* pt = hal_flash_get_info(HAL_PARTITION_RF_FIRMWARE);
#else
    bk_logic_partition_t* pt = bk_flash_get_info(BK_PARTITION_RF_FIRMWARE);
#endif

    if (addr_offset > (BK_FLASH_SECTOR_SIZE)) {
        debug_print("read flash addr error:%08x\n", addr_offset);
        return 0;
    }

    addr = pt->partition_start_addr + addr_offset;  //TXPWR_TAB_FLASH_ADDR + addr_offset;

    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    status = ddev_read(flash_handle, buf, len, addr);
    ddev_close(flash_handle);

    if (status != FLASH_SUCCESS)
        ret = 0;
    else
        ret = len;

    return ret;
}

UINT32 manual_cal_load_txpwr_tab_flash(void) {
    UINT32 status, addr, addr_start;
    DD_HANDLE flash_handle;
    TXPWR_ELEM_ST head;
    TXPWR_IS_RD is_ready_flash = 0;
#if CFG_SUPPORT_ALIOS
    hal_logic_partition_t* pt = hal_flash_get_info(HAL_PARTITION_RF_FIRMWARE);
#else
    bk_logic_partition_t* pt = bk_flash_get_info(BK_PARTITION_RF_FIRMWARE);
#endif

    if (bk7011_is_rfcali_mode_auto()) {
        return TXPWR_NONE_RD;
    }

    addr_start = manual_cal_search_txpwr_tab(TXPWR_TAB_TAB, pt->partition_start_addr);  //TXPWR_TAB_FLASH_ADDR);
    if (!addr_start) {
        debug_print("NO TXPWR_TAB_TAB found in flash\n");
        return TXPWR_NONE_RD;
    }

    addr = manual_cal_search_txpwr_tab(TXPWR_ENABLE_ID, addr_start);
    if (!addr) {
        debug_print("NO TXPWR_ENABLE_ID found in flash\n");
        return TXPWR_NONE_RD;
    }

    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
    ddev_read(flash_handle, (char*)&status, head.len, addr + sizeof(TXPWR_ELEM_ST));
    is_ready_flash = status;
    debug_print("flash txpwr table:0x%x\n", is_ready_flash);

    // If txpwr tab in flash is unused, we should use auto calibration result
    if (is_ready_flash == TXPWR_NONE_RD) {
        debug_print("txpwr tabe in flash is unused\n");
        return TXPWR_NONE_RD;
    }

    // for txpwr b
    if (is_ready_flash & TXPWR_TAB_B_RD) {
        addr = manual_cal_search_txpwr_tab(TXPWR_TAB_B_ID, addr_start);
        if (addr) {
            ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
            ddev_read(flash_handle, (char*)gtxpwr_tab_b, head.len, addr + sizeof(TXPWR_ELEM_ST));
        } else {
            debug_print("txpwr tabe b in flash no found\n");
        }
    }

    // for txpwr g and n20?
    if (is_ready_flash & TXPWR_TAB_G_RD) {
        //for g first
        addr = manual_cal_search_txpwr_tab(TXPWR_TAB_G_ID, addr_start);
        if (addr) {
            ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
            ddev_read(flash_handle, (char*)gtxpwr_tab_g, head.len, addr + sizeof(TXPWR_ELEM_ST));
        } else {
            debug_print("txpwr tabe g in flash no found\n");
        }

        // for n20
        addr = manual_cal_search_txpwr_tab(TXPWR_TAB_DIF_GN20_ID, addr_start);
        if (addr) {
            ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
            ddev_read(flash_handle, (char*)&g_dif_g_n20, head.len, addr + sizeof(TXPWR_ELEM_ST));
            debug_print("dif g and n20 ID in flash:%d\n", g_dif_g_n20);
        } else {
            debug_print("dif g and n20 ID in flash no found, use def:%d\n", g_dif_g_n20);
        }
        manual_cal_fit_txpwr_tab_n_20(g_dif_g_n20);
    }

    // for txpwr N40
    if (is_ready_flash & TXPWR_TAB_N_RD) {
        addr = manual_cal_search_txpwr_tab(TXPWR_TAB_N_ID, addr_start);
        if (addr) {
            ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
            ddev_read(flash_handle, (char*)gtxpwr_tab_n_40, head.len, addr + sizeof(TXPWR_ELEM_ST));
        } else {
            debug_print("txpwr tabe n in flash no found\n");
        }
    }
    // only need load dist40
    addr = manual_cal_search_txpwr_tab(TXPWR_TAB_DIF_GN40_ID, addr_start);
    if (addr) {
        ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
        ddev_read(flash_handle, (char*)&g_dif_g_n40, head.len, addr + sizeof(TXPWR_ELEM_ST));
        debug_print("dif g and n40 ID in flash:%d\n", g_dif_g_n40);
    } else {
        debug_print("dif g and n40 ID in flash no found, use def:%d\n", g_dif_g_n40);
    }

    // for ble tx pwr
    if (is_ready_flash & TXPWR_TAB_BLE) {
        addr = manual_cal_search_txpwr_tab(TXPWR_TAB_BLE_ID, addr_start);
        if (addr) {
            ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
            ddev_read(flash_handle, (char*)gtxpwr_tab_ble, head.len, addr + sizeof(TXPWR_ELEM_ST));
        } else {
            debug_print("txpwr tabe ble in flash no found\n");
        }
    }

    ddev_close(flash_handle);
    debug_print("read txpwr tab from flash success\n");

    return is_ready_flash;
}

UINT32 manual_cal_load_default_txpwr_tab(UINT32 is_ready_flash) {
#if TXPWR_DEFAULT_TAB
    if (bk7011_is_rfcali_mode_auto()) {
        return 0;
    }

    if (!(is_ready_flash & TXPWR_TAB_B_RD)) {
        os_memcpy(gtxpwr_tab_b, gtxpwr_tab_def_b, sizeof(gtxpwr_tab_def_b));
        debug_print("Load default txpwr for b:%p\n", gtxpwr_tab_def_b);
    }

    if (!(is_ready_flash & TXPWR_TAB_G_RD)) {
        os_memcpy(gtxpwr_tab_g, gtxpwr_tab_def_g, sizeof(gtxpwr_tab_def_g));
        debug_print("Load default txpwr for g:%p\n", gtxpwr_tab_def_g);

        manual_cal_fit_txpwr_tab_n_20(g_dif_g_n20);
        debug_print("fit n20 table with dist:%d\n", g_dif_g_n20);
    }

    if (!(is_ready_flash & TXPWR_TAB_N_RD)) {
        os_memcpy(gtxpwr_tab_n_40, gtxpwr_tab_def_n_40, sizeof(gtxpwr_tab_def_n_40));
        debug_print("Load default txpwr for n40:%p\n", gtxpwr_tab_def_n_40);
    }

    if (!(is_ready_flash & TXPWR_TAB_BLE)) {
        os_memcpy(gtxpwr_tab_ble, gtxpwr_tab_def_ble, sizeof(gtxpwr_tab_def_ble));
        debug_print("Load default txpwr for ble:%p\n", gtxpwr_tab_def_ble);
    }
#endif
    return 0;
}

int manual_cal_save_txpwr_tab_to_flash(void) {
    TXPWR_IS_RD is_ready, is_ready_flash;
    UINT32 len = 0, txpwr_len = 0, flash_len = 0;
    UINT8 *buf = NULL, *txpwr_buf = NULL;
    ;
    TAG_TXPWR_ST tag_txpwr;
    TAG_TXPWR_PTR tag_txpwr_ptr = NULL;
    TAG_ENABLE_ST tag_enable;
    TAG_ENABLE_PTR tag_enable_ptr = NULL;
    TAG_TXPWR_TAB_ST tag_tab;
    TAG_TXPWR_TAB_PTR tag_tab_ptr = NULL;
    TAG_DIF_ST tag_dif;
    TAG_DIF_PTR tag_dif_ptr = NULL;

    UINT32 status, addr, addr_start;
    DD_HANDLE flash_handle;
    TXPWR_ELEM_ST head;
#if CFG_SUPPORT_ALIOS
    hal_logic_partition_t* pt = hal_flash_get_info(HAL_PARTITION_RF_FIRMWARE);
#else
    bk_logic_partition_t* pt = bk_flash_get_info(BK_PARTITION_RF_FIRMWARE);
#endif

    is_ready = manual_cal_txpwr_tab_is_fitted();
    debug_print("current txpwr table:0x%x\n", is_ready);
    if (is_ready == TXPWR_NONE_RD) {
        debug_print("TXPWR_NONE_RD, Cann't save txpwr tabe in flash\n");
        return 0;
    }

    // alloc all memery at onece, so we no need to change the size of buf in combin function
    txpwr_len = sizeof(TAG_TXPWR_ST) + sizeof(TAG_ENABLE_ST) + 3 * sizeof(TAG_TXPWR_TAB_ST) + 2 * sizeof(TAG_DIF_ST) + sizeof(TAG_TXPWR_TAB_BLE_ST) + sizeof(TAG_ENABLE_ST) + sizeof(TAG_TXPWR_TAB_ST);

    // read flash, then combin the table in flash
    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    status = manual_cal_search_opt_tab(&flash_len);
    // Has TLV Header?
    if (status) {
        addr_start = manual_cal_search_txpwr_tab(TXPWR_TAB_TAB, pt->partition_start_addr);  //TXPWR_TAB_FLASH_ADDR);
        if (!addr_start) {
            // not TXPWR_TAB_TAB, but has any other id, so attch TXPWR_TAB_TAB after the table
            len = flash_len + txpwr_len;
            buf = (UINT8*)os_malloc(len);
            if (!buf) {
                debug_print("no memory for chipinfo save to flash\n");
                return 0;
            }
            // copy flash
            ddev_read(flash_handle, (char*)buf, flash_len, pt->partition_start_addr);  //TXPWR_TAB_FLASH_ADDR);
            ddev_close(flash_handle);

            // updata new TLV header
            tag_txpwr_ptr = (TAG_TXPWR_PTR)buf;
            tag_txpwr.head.type = BK_FLASH_OPT_TLV_HEADER;
            tag_txpwr.head.len = len - sizeof(TAG_TXPWR_ST);
            ;
            os_memcpy(tag_txpwr_ptr, &tag_txpwr, sizeof(TAG_TXPWR_ST));

            txpwr_buf = (UINT8*)(buf + flash_len);
            addr_start = pt->partition_start_addr;  //TXPWR_TAB_FLASH_ADDR;
        } else {
            // txpwr is exist, and found start addr.
            len = txpwr_len;
            txpwr_buf = buf = (UINT8*)os_malloc(txpwr_len);
            if (!buf) {
                debug_print("no memory for txpwr tab save to flash\n");
                return 0;
            }
        }
    } else {
        // nothing in flash, write TLV with chipid
        debug_print("NO BK_FLASH_OPT_TLV_HEADER found, save_txpwr_tab_to_flash failed\n");
        return 0;
    }

    is_ready_flash = TXPWR_NONE_RD;
    addr = manual_cal_search_txpwr_tab(TXPWR_ENABLE_ID, addr_start);
    if (addr) {
        ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
        ddev_read(flash_handle, (char*)&is_ready_flash, head.len, addr + sizeof(TXPWR_ELEM_ST));
        debug_print("flash txpwr table:0x%x\n", is_ready_flash);
    }

    // for tag TXPW
    tag_txpwr_ptr = (TAG_TXPWR_PTR)txpwr_buf;
    tag_txpwr.head.type = TXPWR_TAB_TAB;
    tag_txpwr.head.len = txpwr_len - sizeof(TAG_TXPWR_ST);
    os_memcpy(tag_txpwr_ptr, &tag_txpwr, sizeof(TAG_TXPWR_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_TXPWR_ST));

    // for tag TXPWR_ENABLE_ID
    tag_enable_ptr = (TAG_ENABLE_PTR)(txpwr_buf);
    tag_enable.head.type = TXPWR_ENABLE_ID;
    tag_enable.head.len = sizeof(tag_enable.flag);

    tag_enable.flag = (UINT32)TXPWR_NONE_RD;
    os_memcpy(tag_enable_ptr, &tag_enable, sizeof(TAG_ENABLE_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_ENABLE_ST));

    // for tag TXPWR_TAB_B_ID
    tag_tab_ptr = (TAG_TXPWR_TAB_PTR)(txpwr_buf);
    tag_tab.head.type = TXPWR_TAB_B_ID;
    tag_tab.head.len = sizeof(TXPWR_ST) * WLAN_2_4_G_CHANNEL_NUM;
    if (is_ready & TXPWR_TAB_B_RD) {
        os_memcpy(&tag_tab.tab[0], gtxpwr_tab_b, tag_tab.head.len);
    } else if (is_ready_flash & TXPWR_TAB_B_RD) {
        addr = manual_cal_search_txpwr_tab(TXPWR_TAB_B_ID, addr_start);
        if (addr) {
            ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
            ddev_read(flash_handle, (char*)&tag_tab.tab[0], head.len, addr + sizeof(TXPWR_ELEM_ST));
        } else {
            debug_print("txpwr tabe b in flash no found\n");
        }
    }
    os_memcpy(tag_tab_ptr, &tag_tab, sizeof(TAG_TXPWR_TAB_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_TXPWR_TAB_ST));

    // for tag TXPWR_TAB_G_ID
    tag_tab_ptr = (TAG_TXPWR_TAB_PTR)(txpwr_buf);
    tag_tab.head.type = TXPWR_TAB_G_ID;
    tag_tab.head.len = sizeof(TXPWR_ST) * WLAN_2_4_G_CHANNEL_NUM;
    if (is_ready & TXPWR_TAB_G_RD) {
        os_memcpy(&tag_tab.tab[0], gtxpwr_tab_g, tag_tab.head.len);
    } else if (is_ready_flash & TXPWR_TAB_G_RD) {
        addr = manual_cal_search_txpwr_tab(TXPWR_TAB_G_ID, addr_start);
        if (addr) {
            ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
            ddev_read(flash_handle, (char*)&tag_tab.tab[0], head.len, addr + sizeof(TXPWR_ELEM_ST));
        } else {
            debug_print("txpwr tabe g in flash no found\n");
        }
    }
    os_memcpy(tag_tab_ptr, &tag_tab, sizeof(TAG_TXPWR_TAB_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_TXPWR_TAB_ST));

    // for tag TXPWR_TAB_N_ID
    tag_tab_ptr = (TAG_TXPWR_TAB_PTR)(txpwr_buf);
    tag_tab.head.type = TXPWR_TAB_N_ID;
    tag_tab.head.len = sizeof(TXPWR_ST) * WLAN_2_4_G_CHANNEL_NUM;
    if (is_ready & TXPWR_TAB_N_RD) {
        os_memcpy(&tag_tab.tab[0], gtxpwr_tab_n_40, tag_tab.head.len);
    } else if (is_ready_flash & TXPWR_TAB_N_RD) {
        addr = manual_cal_search_txpwr_tab(TXPWR_TAB_N_ID, addr_start);
        if (addr) {
            ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
            ddev_read(flash_handle, (char*)&tag_tab.tab[0], head.len, addr + sizeof(TXPWR_ELEM_ST));
        } else {
            debug_print("txpwr tabe N in flash no found\n");
        }
    }
    os_memcpy(tag_tab_ptr, &tag_tab, sizeof(TAG_TXPWR_TAB_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_TXPWR_TAB_ST));

    // for tag TXPWR_TAB_DIF_GN20_ID
    tag_dif_ptr = (TAG_DIF_PTR)(txpwr_buf);
    tag_dif.head.type = TXPWR_TAB_DIF_GN20_ID;
    tag_dif.head.len = sizeof(UINT32);
    tag_dif.differ = g_dif_g_n20;
    os_memcpy(tag_dif_ptr, &tag_dif, sizeof(TAG_DIF_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_DIF_ST));

    // for tag TXPWR_TAB_DIF_GN40_ID
    tag_dif_ptr = (TAG_DIF_PTR)(txpwr_buf);
    tag_dif.head.type = TXPWR_TAB_DIF_GN40_ID;
    tag_dif.head.len = sizeof(UINT32);
    tag_dif.differ = g_dif_g_n40;
    os_memcpy(tag_dif_ptr, &tag_dif, sizeof(TAG_DIF_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_DIF_ST));

    // for tag TXPWR_TAB_BLE_ID
    TAG_TXPWR_TAB_BLE_PTR tag_tab_ble_ptr = (TAG_TXPWR_TAB_BLE_PTR)(txpwr_buf);
    TAG_TXPWR_TAB_BLE_ST tag_tab_ble;
    tag_tab_ble.head.type = TXPWR_TAB_BLE_ID;
    tag_tab_ble.head.len = sizeof(TXPWR_ST) * BLE_2_4_G_CHANNEL_NUM;
    if (is_ready & TXPWR_TAB_BLE) {
        os_memcpy(&tag_tab_ble.tab[0], gtxpwr_tab_ble, tag_tab_ble.head.len);
    } else if (is_ready_flash & TXPWR_TAB_BLE) {
        addr = manual_cal_search_txpwr_tab(TXPWR_TAB_BLE_ID, addr_start);
        if (addr) {
            ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
            ddev_read(flash_handle, (char*)&tag_tab_ble.tab[0], head.len, addr + sizeof(TXPWR_ELEM_ST));
        } else {
            debug_print("txpwr tabe ble in flash no found\n");
        }
    }
    os_memcpy(tag_tab_ble_ptr, &tag_tab_ble, sizeof(TAG_TXPWR_TAB_BLE_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_TXPWR_TAB_BLE_ST));

    // for tag TXPWR_TAB_CALI_STATUTS
    tag_enable_ptr = (TAG_ENABLE_PTR)(txpwr_buf);
    tag_enable.head.type = TXPWR_TAB_CALI_STATUTS;
    tag_enable.head.len = sizeof(tag_enable.flag);
    // wlays write 0, only save position in flash,.
    // txevm -e 4 0/1 can write it again
    tag_enable.flag = CALI_STA_NOFOUND;  //manual_cal_g_rfcali_status();
    os_memcpy(tag_enable_ptr, &tag_enable, sizeof(TAG_ENABLE_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_ENABLE_ST));

    ddev_close(flash_handle);

    addr_start -= pt->partition_start_addr;  //TXPWR_TAB_FLASH_ADDR;
    manual_cal_update_flash_area(addr_start, (char*)buf, len);
    os_free(buf);

    return 1;
}

int manual_cal_save_chipinfo_tab_to_flash(void) {
    UINT32 len = 0, chipinfo_len = 0, flash_len = 0;
    UINT8 *buf = NULL, *info_buf = NULL;
    TAG_TXPWR_ST tag_txpwr;
    TAG_TXPWR_PTR tag_txpwr_ptr = NULL;
    TAG_COMM_ST tag_com;
    TAG_COMM_PTR tag_com_ptr = NULL;
    TAG_CHIPMAC_ST tag_mac;
    TAG_CHIPMAC_PTR tag_mac_ptr = NULL;
    TAG_LPF_IQ_ST tag_lpf_iq;
    TAG_LPF_IQ_PTR tag_lpf_iq_ptr = NULL;
    UINT32 status, addr_start;
    DD_HANDLE flash_handle;
#if CFG_SUPPORT_ALIOS
    hal_logic_partition_t* pt = hal_flash_get_info(HAL_PARTITION_RF_FIRMWARE);
#else
    bk_logic_partition_t* pt = bk_flash_get_info(BK_PARTITION_RF_FIRMWARE);
#endif

    chipinfo_len = sizeof(TAG_TXPWR_ST) + sizeof(TAG_COMM_ST)
#if MCAL_SAVE_MAC_ADDR
                   + sizeof(TAG_CHIPMAC_ST)
#endif
                   + sizeof(TAG_COMM_ST) + sizeof(TAG_COMM_ST) + sizeof(TAG_COMM_ST) + sizeof(TAG_COMM_ST) + sizeof(TAG_LPF_IQ_ST);

    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    status = manual_cal_search_opt_tab(&flash_len);
    // Has TLV Header?
    if (status) {
        addr_start = manual_cal_search_txpwr_tab(TXID, pt->partition_start_addr);  //TXPWR_TAB_FLASH_ADDR);
        if (!addr_start) {
            // not txid, but has any other id, so attch txid after the table
            len = flash_len + chipinfo_len;
            buf = (UINT8*)os_malloc(len);
            if (!buf) {
                debug_print("no memory for chipinfo save to flash\n");
                return 0;
            }
            // copy flash
            ddev_read(flash_handle, (char*)buf, flash_len, pt->partition_start_addr);  //TXPWR_TAB_FLASH_ADDR);
            ddev_close(flash_handle);

            // updata new TLV header
            tag_txpwr_ptr = (TAG_TXPWR_PTR)buf;
            tag_txpwr.head.type = BK_FLASH_OPT_TLV_HEADER;
            tag_txpwr.head.len = len - sizeof(TAG_TXPWR_ST);
            os_memcpy(tag_txpwr_ptr, &tag_txpwr, sizeof(TAG_TXPWR_ST));

            info_buf = (UINT8*)(buf + flash_len);
        } else {
#if 0
            // not need to write, return;
            ddev_close(flash_handle);
            return 0;
#else
            // updata txid
            len = flash_len;
            buf = (UINT8*)os_malloc(len);
            if (!buf) {
                debug_print("no memory for chipinfo save to flash\n");
                return 0;
            }
            // copy flash
            ddev_read(flash_handle, (char*)buf, flash_len, pt->partition_start_addr);  //TXPWR_TAB_FLASH_ADDR);
            ddev_close(flash_handle);

            // point to txid
            info_buf = (UINT8*)(buf + sizeof(TAG_TXPWR_ST));
#endif
        }
    } else {
        // nothing in flash, write TLV with chipid
        len = chipinfo_len + sizeof(TAG_TXPWR_ST);

        buf = (UINT8*)os_malloc(len);
        if (!buf) {
            debug_print("no memory for chipinfo save to flash\n");
            return 0;
        }

        // updata new TLV header
        tag_txpwr_ptr = (TAG_TXPWR_PTR)buf;
        tag_txpwr.head.type = BK_FLASH_OPT_TLV_HEADER;
        tag_txpwr.head.len = len - sizeof(TAG_TXPWR_ST);
        os_memcpy(tag_txpwr_ptr, &tag_txpwr, sizeof(TAG_TXPWR_ST));
        info_buf = (UINT8*)(buf + sizeof(TAG_TXPWR_ST));
    }

    //////////////////////////////////////////////
    // for tag TXID
    tag_txpwr_ptr = (TAG_TXPWR_PTR)(info_buf);
    tag_txpwr.head.type = TXID;
    tag_txpwr.head.len = chipinfo_len - sizeof(TAG_TXPWR_ST);
    os_memcpy(tag_txpwr_ptr, &tag_txpwr, sizeof(TAG_TXPWR_ST));

    // for tag TXID_ID
    info_buf = (UINT8*)(info_buf + sizeof(TAG_TXPWR_ST));
    tag_com_ptr = (TAG_COMM_PTR)(info_buf);
    tag_com.head.type = TXID_ID;
    tag_com.head.len = sizeof(tag_com.value);
    tag_com.value = (UINT32)DEFAULT_TXID_ID;
    os_memcpy(tag_com_ptr, &tag_com, sizeof(TAG_COMM_ST));

#if MCAL_SAVE_MAC_ADDR
    // for tag TXID_MAC
    info_buf = (UINT8*)(info_buf + sizeof(TAG_COMM_ST));
    tag_mac_ptr = (TAG_CHIPMAC_PTR)(info_buf);
    tag_mac.head.type = TXID_MAC;
    tag_mac.head.len = 6;
    os_memcpy(&tag_mac.arry[0], system_mac, 6);
    os_memcpy(tag_mac_ptr, &tag_mac, sizeof(TAG_CHIPMAC_ST));
    info_buf = (UINT8*)(info_buf + sizeof(TAG_CHIPMAC_ST));
#else
    info_buf = (UINT8*)(info_buf + sizeof(TAG_COMM_ST));
#endif
    // for tag TXID_THERMAL
    //manual_cal_get_current_temperature();
    debug_print("save sys temper:%d\n", g_cur_temp);
    tag_com_ptr = (TAG_COMM_PTR)(info_buf);
    tag_com.head.type = TXID_THERMAL;
    tag_com.head.len = sizeof(tag_com.value);
    tag_com.value = (UINT32)g_cur_temp;
    os_memcpy(tag_com_ptr, &tag_com, sizeof(TAG_COMM_ST));

    // for tag TXID_CHANNEL
    info_buf = (UINT8*)(info_buf + sizeof(TAG_COMM_ST));
    tag_com_ptr = (TAG_COMM_PTR)(info_buf);
    tag_com.head.type = TXID_CHANNEL;
    tag_com.head.len = sizeof(tag_com.value);
    tag_com.value = (UINT32)DEFAULT_TXID_CHANNEL;
    os_memcpy(tag_com_ptr, &tag_com, sizeof(TAG_COMM_ST));

    // for tag TXID_XTAL
    info_buf = (UINT8*)(info_buf + sizeof(TAG_COMM_ST));
    tag_com_ptr = (TAG_COMM_PTR)(info_buf);
    tag_com.head.type = TXID_XTAL;
    tag_com.head.len = sizeof(tag_com.value);
    tag_com.value = (UINT32)g_xtal;
    os_memcpy(tag_com_ptr, &tag_com, sizeof(TAG_COMM_ST));

    // for tag TXID_ADC
    info_buf = (UINT8*)(info_buf + sizeof(TAG_COMM_ST));
    tag_com_ptr = (TAG_COMM_PTR)(info_buf);
    tag_com.head.type = TXID_ADC;
    tag_com.head.len = sizeof(tag_com.value);
    UINT32* p_temp = (UINT32*)&saradc_val;
    tag_com.value = *p_temp;
    ///tag_com.value = *((UINT32 *)&saradc_val);
    os_memcpy(tag_com_ptr, &tag_com, sizeof(TAG_COMM_ST));

    // for tag lpf i&q
    info_buf = (UINT8*)(info_buf + sizeof(TAG_COMM_ST));
    tag_lpf_iq_ptr = (TAG_LPF_IQ_PTR)(info_buf);
    tag_lpf_iq.head.type = TXID_LPFCAP;
    tag_lpf_iq.head.len = sizeof(TAG_LPF_IQ_ST) - sizeof(TXPWR_ELEM_ST);

    tag_lpf_iq.lpf_i = g_lpf_cal_i;
    tag_lpf_iq.lpf_q = g_lpf_cal_q;
    debug_print("%x, %d, %d, %d\n", TXID_LPFCAP, tag_lpf_iq.head.len, g_lpf_cal_i, g_lpf_cal_q);
    os_memcpy(tag_lpf_iq_ptr, &tag_lpf_iq, sizeof(TAG_LPF_IQ_ST));

    manual_cal_update_flash_area(0, (char*)buf, len);
    os_free(buf);

    return 1;
}

int manual_cal_get_macaddr_from_flash(UINT8* mac_ptr) {
#if MCAL_SAVE_MAC_ADDR
    UINT32 flash_len = 0;
    UINT32 status, addr, addr_start;
    DD_HANDLE flash_handle;
    TXPWR_ELEM_ST head;
#if CFG_SUPPORT_ALIOS
    hal_logic_partition_t* pt = hal_flash_get_info(HAL_PARTITION_RF_FIRMWARE);
#else
    bk_logic_partition_t* pt = bk_flash_get_info(BK_PARTITION_RF_FIRMWARE);
#endif
    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    status = manual_cal_search_opt_tab(&flash_len);
    // Has TLV Header?
    if (status) {
        addr_start = manual_cal_search_txpwr_tab(TXID, pt->partition_start_addr);  //TXPWR_TAB_FLASH_ADDR);
        if (!addr_start) {
            // not txid, but has any other id, so attch txid after the table
            debug_print("No txid header found in flash\n");
            return 0;
        }
    } else {
        debug_print("No TLV header found in flash\n");
        return 0;
    }

    addr = manual_cal_search_txpwr_tab(TXID_MAC, addr_start);
    if (addr) {
        ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
        ddev_read(flash_handle, (char*)mac_ptr, head.len, addr + sizeof(TXPWR_ELEM_ST));
        //debug_print("read MAC ADDR from flash\n");
    } else {
        debug_print("No MAC id found in txid header \n");
        return 0;
    }

    return 1;
#else
    return 0;
#endif
}

int manual_cal_write_macaddr_to_flash(UINT8* mac_ptr) {
#if MCAL_SAVE_MAC_ADDR
    UINT32 flash_len = 0;
    UINT32 status, addr, addr_start;
    DD_HANDLE flash_handle;
    TXPWR_ELEM_ST head;
#if CFG_SUPPORT_ALIOS
    hal_logic_partition_t* pt = hal_flash_get_info(HAL_PARTITION_RF_FIRMWARE);
#else
    bk_logic_partition_t* pt = bk_flash_get_info(BK_PARTITION_RF_FIRMWARE);
#endif

    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    status = manual_cal_search_opt_tab(&flash_len);
    // Has TLV Header?
    if (!status) {
        if (!manual_cal_save_chipinfo_tab_to_flash()) {
            debug_print("No TLV header found in flash, init partition failed\n");
            return 0;
        } else {
            debug_print("No TLV header found in flash, init partition success\n");
        }
    }

    addr_start = manual_cal_search_txpwr_tab(TXID, pt->partition_start_addr);  //TXPWR_TAB_FLASH_ADDR);
    if (!addr_start) {
        // not txid, but has any other id, so attch txid after the table
        debug_print("No txid header found in flash\n");
        return 0;
    }

    addr = manual_cal_search_txpwr_tab(TXID_MAC, addr_start);
    if (addr) {
        ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
        addr += sizeof(TXPWR_ELEM_ST);
        addr -= pt->partition_start_addr;                       //TXPWR_TAB_FLASH_ADDR;
        manual_cal_update_flash_area(addr, (char*)mac_ptr, 6);  //0: sucess, 1 failed
    } else {
        debug_print("No MAC id found in txid header \n");
        return 0;
    }

    return 1;
#else
    return 0;
#endif
}

UINT8 manual_cal_wirte_otp_flash(UINT32 addr, UINT32 len, UINT8* buf) {
    UINT8 ret = 0;
#if CFG_SUPPORT_ALIOS
    hal_logic_partition_t* pt = hal_flash_get_info(HAL_PARTITION_RF_FIRMWARE);
#else
    bk_logic_partition_t* pt = bk_flash_get_info(BK_PARTITION_RF_FIRMWARE);
#endif
    //debug_print("wirte_otp_flash:addr:%08x, len:%d, buf:%p\n", addr, len, buf);
    addr -= pt->partition_start_addr;                           //TXPWR_TAB_FLASH_ADDR;
    ret = manual_cal_update_flash_area(addr, (char*)buf, len);  //0: sucess, 1 failed
    return ret;
}

UINT8 manual_cal_read_otp_flash(UINT32 addr, UINT32 len, UINT8* buf) {
    UINT8 ret = 0;
    //debug_print("read_otp_flash:addr:%08x, len:%d, buf:%p\n",addr, len, buf);
    ret = manual_cal_read_flash(addr, (char*)buf, len);  //0:failed, others: len
    return ret;
}

void manual_cal_get_current_temperature(void) {
    UINT32 temp_value;

    if (!temp_single_get_current_temperature(&temp_value))
        g_cur_temp = temp_value;

    debug_print("system temperature:%0d\n", g_cur_temp);
}

void manual_cal_show_otp_flash(void) {
    UINT32 status, flash_len, addr_start;
    DD_HANDLE flash_handle;
#if CFG_SUPPORT_ALIOS
    hal_logic_partition_t* pt = hal_flash_get_info(HAL_PARTITION_RF_FIRMWARE);
#else
    bk_logic_partition_t* pt = bk_flash_get_info(BK_PARTITION_RF_FIRMWARE);
#endif

    flash_len = 0;
    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    status = manual_cal_search_opt_tab(&flash_len);

    if (!status) {
        status = 1;
        flash_len = 1024;
    }

    if (status) {
        UINT8* buf = (UINT8*)os_malloc(flash_len);
        if (!buf) {
            debug_print("no memory for show_otp_flash\n");
            ddev_close(flash_handle);
            return;
        }

        os_memset(buf, 0, sizeof(flash_len));
        addr_start = pt->partition_start_addr;
        // copy flash
        ddev_read(flash_handle, (char*)buf, flash_len, addr_start);  //TXPWR_TAB_FLASH_ADDR);
        for (int i = 0; i < flash_len; i++) {
            debug_print("%02x,", buf[i]);
            if ((i + 1) % 16 == 0)
                debug_print("\n");
        }
        debug_print("\n");

        os_free(buf);
    } else {
        debug_print("No TLV header found in flash\n");
    }

    ddev_close(flash_handle);
}

void manual_cal_clear_otp_flash(void) {
    UINT32 flash_len = 1024;
    UINT8* buf = (UINT8*)os_malloc(flash_len);

    os_memset(buf, 0xff, sizeof(flash_len));
    manual_cal_update_flash_area(0, (char*)buf, flash_len);
    os_free(buf);
}

UINT32 manual_cal_g_rfcali_status(void) {
    UINT32 ret, flash_status;

    if (g_cali_status != CALI_STA_UNINITIALIZED) {
        return g_cali_status;
    }

    if (bk7011_is_rfcali_mode_auto()) {
        g_cali_status = CALI_STA_OK;
        return g_cali_status;
    }

    ret = manual_cal_get_rfcali_status_inflash(&flash_status);
    if (ret == 0) {
        // not found in flash
        g_cali_status = CALI_STA_NOFOUND;
    } else {
        if (manual_cal_is_in_rfcali_mode() == 1) {
            // only clear it while in rfcali mode
            g_cali_status = CALI_STA_NOFOUND;
        } else
            g_cali_status = flash_status;
    }
    //debug_print("%d, %d\n", g_cali_status, g_rfcali_mode);

    return g_cali_status;
}

int manual_cal_rfcali_status(void) {
    UINT32 status = manual_cal_g_rfcali_status();

    if (status != CALI_STA_OK)
        return CALI_STATUS_FAIL;
    else
        return CALI_STATUS_PASS;
}

int manual_cal_get_rfcali_status_inflash(UINT32* rf_status) {
    UINT32 flash_len = 0;
    UINT32 status, addr, addr_start, status_bak;
    DD_HANDLE flash_handle;
    TXPWR_ELEM_ST head;
#if CFG_SUPPORT_ALIOS
    hal_logic_partition_t* pt = hal_flash_get_info(HAL_PARTITION_RF_FIRMWARE);
#else
    bk_logic_partition_t* pt = bk_flash_get_info(BK_PARTITION_RF_FIRMWARE);
#endif
    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    status = manual_cal_search_opt_tab(&flash_len);
    // Has TLV Header?
    if (status) {
        addr_start = manual_cal_search_txpwr_tab(TXPWR_TAB_TAB, pt->partition_start_addr);  //TXPWR_TAB_FLASH_ADDR);
        if (!addr_start) {
            // not txid, but has any other id, so attch txid after the table
            debug_print("No txpwr header found in flash\n");
            return 0;
        }
    } else {
        debug_print("No TLV header found in flash\n");
        return 0;
    }

    addr = manual_cal_search_txpwr_tab(TXPWR_TAB_CALI_STATUTS, addr_start);
    if (addr) {
        ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
        ddev_read(flash_handle, (char*)&status_bak, head.len, addr + sizeof(TXPWR_ELEM_ST));
    } else {
        debug_print("No RFCALI STATUS found in txid header \n");
        return 0;
    }

    if (rf_status)
        *rf_status = status_bak;

    return 1;
}

int manual_cal_set_rfcali_status_inflash(UINT32 rf_status) {
    if (manual_cal_is_in_rfcali_mode() == 0) {
        debug_print("not in rfcali mode\n");
        return 0;
    }

    UINT32 flash_len = 0;
    UINT32 status, addr, addr_start;
    DD_HANDLE flash_handle;
    TXPWR_ELEM_ST head;
#if CFG_SUPPORT_ALIOS
    hal_logic_partition_t* pt = hal_flash_get_info(HAL_PARTITION_RF_FIRMWARE);
#else
    bk_logic_partition_t* pt = bk_flash_get_info(BK_PARTITION_RF_FIRMWARE);
#endif

    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    status = manual_cal_search_opt_tab(&flash_len);
    // Has TLV Header?
    if (status) {
        addr_start = manual_cal_search_txpwr_tab(TXPWR_TAB_TAB, pt->partition_start_addr);  //TXPWR_TAB_FLASH_ADDR);
        if (!addr_start) {
            // not txid, but has any other id, so attch txid after the table
            debug_print("No txpwr header found in flash\n");
            return 0;
        }
    } else {
        debug_print("No TLV header found in flash\n");
        return 0;
    }

    addr = manual_cal_search_txpwr_tab(TXPWR_TAB_CALI_STATUTS, addr_start);
    if (addr) {
        ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
        addr += sizeof(TXPWR_ELEM_ST);
        addr -= pt->partition_start_addr;                                       //TXPWR_TAB_FLASH_ADDR;
        manual_cal_update_flash_area(addr, (char*)&rf_status, sizeof(UINT32));  //0: sucess, 1 failed
    } else {
        debug_print("No RFCALI STATUS found in txid header \n");
        return 0;
    }

    addr = manual_cal_search_txpwr_tab(TXPWR_ENABLE_ID, addr_start);
    if (addr) {
        UINT32 is_ready;
        ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
        addr += sizeof(TXPWR_ELEM_ST);
        addr -= pt->partition_start_addr;  //TXPWR_TAB_FLASH_ADDR;
        if (rf_status == 0)
            is_ready = TXPWR_NONE_RD;
        else
            is_ready = manual_cal_txpwr_tab_is_fitted();
        manual_cal_update_flash_area(addr, (char*)&is_ready, sizeof(UINT32));  //0: sucess, 1 failed
    } else {
        debug_print("No TXPWR_ENABLE_ID found in txid header \n");
        return 0;
    }

    return 1;
}

void manual_cal_set_xtal(UINT32 xtal) {
    UINT32 param = xtal;
    if (xtal > PARAM_XTALH_CTUNE_MASK)
        param = PARAM_XTALH_CTUNE_MASK;

    debug_print("xtal_cali:%d\n", xtal);
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_SET_XTALH_CTUNE, &param);
    g_xtal = param;
}

UINT32 manual_cal_get_xtal(void) {
    return g_xcali.last_xtal;
}

void manual_cal_set_lpf_iq(UINT32 lpf_i, UINT32 lpf_q) {
#if (CFG_SOC_NAME == SOC_BK7231N)
    if (lpf_q > 0x7f)
        lpf_q = 0x7f;
    if (lpf_i > 0x7f)
        lpf_i = 0x7f;
#else
    if (lpf_q > 0x3f)
        lpf_q = 0x3f;
    if (lpf_i > 0x3f)
        lpf_i = 0x3f;
#endif

    g_lpf_cal_i = lpf_i;
    g_lpf_cal_q = lpf_q;
    rwnx_cal_set_lpfcap_iq(lpf_i, lpf_q);
}

void manual_cal_load_lpf_iq_tag_flash(void) {
    UINT32 status, addr, addr_start;
    DD_HANDLE flash_handle;
#if CFG_SUPPORT_ALIOS
    hal_logic_partition_t* pt = hal_flash_get_info(HAL_PARTITION_RF_FIRMWARE);
#else
    bk_logic_partition_t* pt = bk_flash_get_info(BK_PARTITION_RF_FIRMWARE);
#endif
    TAG_LPF_IQ_ST lpf;

    bk7011_get_tx_filter_corner((INT32*)&g_lpf_cal_i, (INT32*)&g_lpf_cal_q);

#if (CFG_SOC_NAME != SOC_BK7231N)
    addr_start = manual_cal_search_txpwr_tab(TXID, pt->partition_start_addr);  //TXPWR_TAB_FLASH_ADDR);
    if (!addr_start) {
        debug_print("NO TXID found in flash, use lpf i&q:%d, %d\n",
                    g_lpf_cal_i, g_lpf_cal_q);
        goto init_lpf;
    }

    addr = manual_cal_search_txpwr_tab(TXID_LPFCAP, addr_start);
    if (!addr) {
        debug_print("NO TXID_LPFCAP found in flash, use def %d, %d\n",
                    g_lpf_cal_i, g_lpf_cal_q);
        goto init_lpf;
    }

    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    ddev_read(flash_handle, (char*)&lpf, sizeof(TXPWR_ELEM_ST), addr);
    ddev_read(flash_handle, (char*)&lpf.lpf_i, lpf.head.len, addr + sizeof(TXPWR_ELEM_ST));
    debug_print("lpf_i & q in flash is:%d, %d\n", lpf.lpf_i, lpf.lpf_q);

    if (DEFAULT_TXID_LPF_CAP_I != lpf.lpf_i) {
        g_lpf_cal_i = lpf.lpf_i;
    }
    if (DEFAULT_TXID_LPF_CAP_Q != lpf.lpf_q) {
        g_lpf_cal_q = lpf.lpf_q;
    }
init_lpf:
#endif  // (CFG_SOC_NAME != SOC_BK7231N)

    rwnx_cal_set_lpfcap_iq(g_lpf_cal_i, g_lpf_cal_q);
}

void manual_cal_load_differ_tag_from_flash(void) {
    UINT32 status, addr, addr_start;
    DD_HANDLE flash_handle;
    TXPWR_ELEM_ST head;
#if CFG_SUPPORT_ALIOS
    hal_logic_partition_t* pt = hal_flash_get_info(HAL_PARTITION_RF_FIRMWARE);
#else
    bk_logic_partition_t* pt = bk_flash_get_info(BK_PARTITION_RF_FIRMWARE);
#endif
    UINT32 dif_n20 = MOD_DIST_G_BW_N20;
    UINT32 dif_n40 = MOD_DIST_G_BW_N40;

    addr_start = manual_cal_search_txpwr_tab(TXPWR_TAB_TAB, pt->partition_start_addr);  //TXPWR_TAB_FLASH_ADDR);
    if (!addr_start) {
        debug_print("NO TXPWR_TAB_TAB found in flash, use def:%d, %d\n",
                    MOD_DIST_G_BW_N20, MOD_DIST_G_BW_N40);
        goto load_diff;
    }

    addr = manual_cal_search_txpwr_tab(TXPWR_TAB_DIF_GN20_ID, addr_start);
    if (!addr) {
        debug_print("NO TXPWR_TAB_DIF_GN20_ID found in flash, use def:%d\n", MOD_DIST_G_BW_N20);
    } else {
        flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
        ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
        ddev_read(flash_handle, (char*)&status, head.len, addr + sizeof(TXPWR_ELEM_ST));
        dif_n20 = status;
    }

    addr = manual_cal_search_txpwr_tab(TXPWR_TAB_DIF_GN40_ID, addr_start);
    if (!addr) {
        debug_print("NO TXPWR_TAB_DIF_GN40_ID found in flash, use def:%d\n", TXPWR_TAB_DIF_GN40_ID);
    } else {
        flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
        ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
        ddev_read(flash_handle, (char*)&status, head.len, addr + sizeof(TXPWR_ELEM_ST));
        dif_n40 = status;
    }

load_diff:
    debug_print("diff in flash:n20-%d, n40-%d\n", dif_n20, dif_n40);
    return;
}

int manual_cal_load_xtal_tag_from_flash(void) {
    UINT32 status, addr, addr_start;
    DD_HANDLE flash_handle;
    TXPWR_ELEM_ST head;
#if CFG_SUPPORT_ALIOS
    hal_logic_partition_t* pt = hal_flash_get_info(HAL_PARTITION_RF_FIRMWARE);
#else
    bk_logic_partition_t* pt = bk_flash_get_info(BK_PARTITION_RF_FIRMWARE);
#endif
    UINT32 xtal = DEFAULT_TXID_XTAL;

    addr_start = manual_cal_search_txpwr_tab(TXID, pt->partition_start_addr);  //TXPWR_TAB_FLASH_ADDR);
    if (!addr_start) {
        debug_print("NO TXID found in flash, use def xtal:%d\n", DEFAULT_TXID_XTAL);
        goto init_xtal;
    }

    addr = manual_cal_search_txpwr_tab(TXID_XTAL, addr_start);
    if (!addr) {
        debug_print("NO TXID_THERMAL found in flash, use def xtal:%d\n", DEFAULT_TXID_XTAL);
        goto init_xtal;
    }

    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
    ddev_read(flash_handle, (char*)&status, head.len, addr + sizeof(TXPWR_ELEM_ST));
    xtal = status;

init_xtal:
    debug_print("xtal in flash is:%d\n", xtal);
    return xtal;
}

void manual_cal_load_xtal_tag_flash(void) {
    g_xtal = manual_cal_load_xtal_tag_from_flash();
    manual_cal_set_xtal(g_xtal);
#if CFG_USE_TEMPERATURE_DETECT
    manual_cal_init_xtal_cali(g_cur_temp, g_xtal);
#endif
}

#if CFG_TEMP_DETECT_VERSION == CFG_TEMP_DETECT_VERSION1
void manual_cal_do_xtal_temp_delta_set(INT8 shift) {
    g_xcali.xtal_c_delta = shift;
}
#endif

int manual_cal_load_temp_tag_from_flash(void) {
    UINT32 status, addr, addr_start;
    DD_HANDLE flash_handle;
    TXPWR_ELEM_ST head;
#if CFG_SUPPORT_ALIOS
    hal_logic_partition_t* pt = hal_flash_get_info(HAL_PARTITION_RF_FIRMWARE);
#else
    bk_logic_partition_t* pt = bk_flash_get_info(BK_PARTITION_RF_FIRMWARE);
#endif
    UINT32 tem_in_flash = DEFAULT_TXID_THERMAL;

    addr_start = manual_cal_search_txpwr_tab(TXID, pt->partition_start_addr);  //TXPWR_TAB_FLASH_ADDR);
    if (!addr_start) {
        debug_print("NO TXID found in flash, use def temp:%d\n", DEFAULT_TXID_THERMAL);
        goto init_temp;
    }

    addr = manual_cal_search_txpwr_tab(TXID_THERMAL, addr_start);
    if (!addr) {
        debug_print("NO TXID_THERMAL found in flash, use def temp:%d\n", DEFAULT_TXID_THERMAL);
        goto init_temp;
    }

    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
    ddev_read(flash_handle, (char*)&status, head.len, addr + sizeof(TXPWR_ELEM_ST));
    tem_in_flash = status;

init_temp:
    debug_print("temp in flash is:%d\n", tem_in_flash);
    return tem_in_flash;
}

int manual_cal_save_cailmain_tx_tab_to_flash(void) {
    extern INT32 gtx_dcorMod;
    extern INT32 gtx_dcorPA;
    extern INT32 gtx_pre_gain;
    INT32 gtx_i_dc_comp;
    INT32 gtx_q_dc_comp;
    INT32 gtx_i_gain_comp;
    INT32 gtx_q_gain_comp;
    INT32 gtx_ifilter_corner;
    INT32 gtx_qfilter_corner;
    INT32 gtx_phase_comp;
    INT32 gtx_phase_ty2;

    UINT32 len = 0, txpwr_len = 0, flash_len = 0;
    UINT8 *buf = NULL, *txpwr_buf = NULL;
    ;
    TAG_TXPWR_ST tag_txpwr;
    TAG_TXPWR_PTR tag_txpwr_ptr = NULL;
    TAG_COMM_ST tag_comm;
    TAG_COMM_PTR tag_comm_ptr = NULL;

    UINT32 status, addr_start;
    DD_HANDLE flash_handle;
    /// TXPWR_ELEM_ST head;
#if CFG_SUPPORT_ALIOS
    hal_logic_partition_t* pt = hal_flash_get_info(HAL_PARTITION_RF_FIRMWARE);
#else
    bk_logic_partition_t* pt = bk_flash_get_info(BK_PARTITION_RF_FIRMWARE);
#endif

    // alloc all memery at onece, so we no need to change the size of buf in combin function
    txpwr_len = sizeof(TAG_TXPWR_ST)    // head
                + sizeof(TAG_COMM_ST)   // gtx_dcorMod
                + sizeof(TAG_COMM_ST)   // gtx_dcorPA
                + sizeof(TAG_COMM_ST)   // gtx_pre_gain
                + sizeof(TAG_COMM_ST)   // gtx_i_dc_comp
                + sizeof(TAG_COMM_ST)   // gtx_q_dc_comp
                + sizeof(TAG_COMM_ST)   // gtx_i_gain_comp
                + sizeof(TAG_COMM_ST)   // gtx_q_gain_comp
                + sizeof(TAG_COMM_ST)   // gtx_ifilter_corner
                + sizeof(TAG_COMM_ST)   // gtx_qfilter_corner
                + sizeof(TAG_COMM_ST)   // gtx_phase_comp
                + sizeof(TAG_COMM_ST);  // gtx_phase_ty2

    // read flash, then combin the table in flash
    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    status = manual_cal_search_opt_tab(&flash_len);
    // Has TLV Header?
    if (status) {
        addr_start = manual_cal_search_txpwr_tab(CALI_MAIN_TX, pt->partition_start_addr);  //TXPWR_TAB_FLASH_ADDR);
        if (!addr_start) {
            // no CALI_MAIN_TX, but has any other id, so attch CALI_MAIN_TX after the table
            len = flash_len + txpwr_len;
            buf = (UINT8*)os_malloc(len);
            if (!buf) {
                debug_print("no memory for save_cailmain_tx_tab\n");
                ddev_close(flash_handle);
                return 0;
            }
            // copy flash
            ddev_read(flash_handle, (char*)buf, flash_len, pt->partition_start_addr);  //TXPWR_TAB_FLASH_ADDR);
            ddev_close(flash_handle);

            // updata new TLV header
            tag_txpwr_ptr = (TAG_TXPWR_PTR)buf;
            tag_txpwr.head.type = BK_FLASH_OPT_TLV_HEADER;
            tag_txpwr.head.len = len - sizeof(TAG_TXPWR_ST);
            ;
            os_memcpy(tag_txpwr_ptr, &tag_txpwr, sizeof(TAG_TXPWR_ST));

            txpwr_buf = (UINT8*)(buf + flash_len);
            addr_start = pt->partition_start_addr;  //TXPWR_TAB_FLASH_ADDR;
        } else {
            // CALI_MAIN_TX is exist, and found start addr.
            len = txpwr_len;
            txpwr_buf = buf = (UINT8*)os_malloc(txpwr_len);
            if (!buf) {
                debug_print("no memory for txpwr tab save to flash\n");
                ddev_close(flash_handle);
                return 0;
            }
        }
    } else {
        // nothing in flash, write TLV with chipid
        debug_print("NO BK_FLASH_OPT_TLV_HEADER found, save_cailmain_tx_tab failed\n");
        ddev_close(flash_handle);
        return 0;
    }

    ddev_close(flash_handle);

    bk7011_get_tx_filter_corner(&gtx_ifilter_corner, &gtx_qfilter_corner);
    bk7011_get_tx_dc_comp(&gtx_i_dc_comp, &gtx_q_dc_comp);
    bk7011_get_tx_gain_comp(&gtx_i_gain_comp, &gtx_q_gain_comp);
    bk7011_get_tx_phase(&gtx_phase_comp, &gtx_phase_ty2);
    //bk7011_get_tx_tssi_thred(&gtx_tssi_thred_b, &gtx_tssi_thred_g);

    // for tag CALI_MAIN_TX
    tag_txpwr_ptr = (TAG_TXPWR_PTR)txpwr_buf;
    tag_txpwr.head.type = CALI_MAIN_TX;
    tag_txpwr.head.len = txpwr_len - sizeof(TAG_TXPWR_ST);
    os_memcpy(tag_txpwr_ptr, &tag_txpwr, sizeof(TAG_TXPWR_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_TXPWR_ST));

    // for tag gtx_dcorMod
    tag_comm_ptr = (TAG_COMM_PTR)(txpwr_buf);
    tag_comm.head.type = CM_TX_DCOR_MOD;
    tag_comm.head.len = sizeof(tag_comm.value);
    tag_comm.value = (UINT32)gtx_dcorMod;
    os_memcpy(tag_comm_ptr, &tag_comm, sizeof(TAG_COMM_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_COMM_ST));

    // for tag gtx_dcorPA
    tag_comm_ptr = (TAG_COMM_PTR)(txpwr_buf);
    tag_comm.head.type = CM_TX_DCOR_PA;
    tag_comm.head.len = sizeof(tag_comm.value);
    tag_comm.value = (UINT32)gtx_dcorPA;
    os_memcpy(tag_comm_ptr, &tag_comm, sizeof(TAG_COMM_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_COMM_ST));

    // for tag gtx_pre_gain
    tag_comm_ptr = (TAG_COMM_PTR)(txpwr_buf);
    tag_comm.head.type = CM_TX_PREGAIN;
    tag_comm.head.len = sizeof(tag_comm.value);
    tag_comm.value = (UINT32)gtx_pre_gain;
    os_memcpy(tag_comm_ptr, &tag_comm, sizeof(TAG_COMM_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_COMM_ST));

    // for tag gtx_i_dc_comp
    tag_comm_ptr = (TAG_COMM_PTR)(txpwr_buf);
    tag_comm.head.type = CM_TX_I_DC_COMP;
    tag_comm.head.len = sizeof(tag_comm.value);
    tag_comm.value = (UINT32)gtx_i_dc_comp;
    os_memcpy(tag_comm_ptr, &tag_comm, sizeof(TAG_COMM_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_COMM_ST));

    // for tag gtx_q_dc_comp
    tag_comm_ptr = (TAG_COMM_PTR)(txpwr_buf);
    tag_comm.head.type = CM_TX_Q_DC_COMP;
    tag_comm.head.len = sizeof(tag_comm.value);
    tag_comm.value = (UINT32)gtx_q_dc_comp;
    os_memcpy(tag_comm_ptr, &tag_comm, sizeof(TAG_COMM_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_COMM_ST));

    // for tag gtx_i_gain_comp
    tag_comm_ptr = (TAG_COMM_PTR)(txpwr_buf);
    tag_comm.head.type = CM_TX_I_GAIN_COMP;
    tag_comm.head.len = sizeof(tag_comm.value);
    tag_comm.value = (UINT32)gtx_i_gain_comp;
    os_memcpy(tag_comm_ptr, &tag_comm, sizeof(TAG_COMM_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_COMM_ST));

    // for tag gtx_q_gain_comp
    tag_comm_ptr = (TAG_COMM_PTR)(txpwr_buf);
    tag_comm.head.type = CM_TX_Q_GAIN_COMP;
    tag_comm.head.len = sizeof(tag_comm.value);
    tag_comm.value = (UINT32)gtx_q_gain_comp;
    os_memcpy(tag_comm_ptr, &tag_comm, sizeof(TAG_COMM_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_COMM_ST));

    // for tag gtx_ifilter_corner
    tag_comm_ptr = (TAG_COMM_PTR)(txpwr_buf);
    tag_comm.head.type = CM_TX_I_FILTER_CORNER;
    tag_comm.head.len = sizeof(tag_comm.value);
    tag_comm.value = (UINT32)gtx_ifilter_corner;
    os_memcpy(tag_comm_ptr, &tag_comm, sizeof(TAG_COMM_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_COMM_ST));

    // for tag gtx_qfilter_corner
    tag_comm_ptr = (TAG_COMM_PTR)(txpwr_buf);
    tag_comm.head.type = CM_TX_Q_FILTER_CORNER;
    tag_comm.head.len = sizeof(tag_comm.value);
    tag_comm.value = (UINT32)gtx_qfilter_corner;
    os_memcpy(tag_comm_ptr, &tag_comm, sizeof(TAG_COMM_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_COMM_ST));

    // for tag gtx_phase_comp
    tag_comm_ptr = (TAG_COMM_PTR)(txpwr_buf);
    tag_comm.head.type = CM_TX_PHASE_COMP;
    tag_comm.head.len = sizeof(tag_comm.value);
    tag_comm.value = (UINT32)gtx_phase_comp;
    os_memcpy(tag_comm_ptr, &tag_comm, sizeof(TAG_COMM_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_COMM_ST));

    // for tag gtx_phase_ty2
    tag_comm_ptr = (TAG_COMM_PTR)(txpwr_buf);
    tag_comm.head.type = CM_TX_PHASE_TY2;
    tag_comm.head.len = sizeof(tag_comm.value);
    tag_comm.value = (UINT32)gtx_phase_ty2;
    os_memcpy(tag_comm_ptr, &tag_comm, sizeof(TAG_COMM_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_COMM_ST));

    addr_start -= pt->partition_start_addr;  //TXPWR_TAB_FLASH_ADDR;
    manual_cal_update_flash_area(addr_start, (char*)buf, len);
    os_free(buf);

    return 1;
}

int manual_cal_save_cailmain_rx_tab_to_flash(void) {
    INT32 rx_dc_gain_tab[8];
    INT32 rx_amp_err_wr;
    INT32 rx_phase_err_wr;

    UINT32 len = 0, txpwr_len = 0, flash_len = 0;
    UINT8 *buf = NULL, *txpwr_buf = NULL;
    ;
    TAG_TXPWR_ST tag_txpwr;
    TAG_TXPWR_PTR tag_txpwr_ptr = NULL;
    TAG_COMM_ST tag_comm;
    TAG_COMM_PTR tag_comm_ptr = NULL;
    TAG_RX_DC_ST tag_rx_dc;
    TAG_RX_DC_PTR tag_rx_dc_ptr = NULL;

    UINT32 status, addr_start;
    DD_HANDLE flash_handle;
    /// TXPWR_ELEM_ST head;
#if CFG_SUPPORT_ALIOS
    hal_logic_partition_t* pt = hal_flash_get_info(HAL_PARTITION_RF_FIRMWARE);
#else
    bk_logic_partition_t* pt = bk_flash_get_info(BK_PARTITION_RF_FIRMWARE);
#endif

    // alloc all memery at onece, so we no need to change the size of buf in combin function
    txpwr_len = sizeof(TAG_TXPWR_ST)    // head
                + sizeof(TAG_RX_DC_ST)  // rx_dc_gain_tab 0 -7
                + sizeof(TAG_COMM_ST)   // rx_amp_err_wr
                + sizeof(TAG_COMM_ST);  // rx_phase_err_wr

    // read flash, then combin the table in flash
    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    status = manual_cal_search_opt_tab(&flash_len);
    // Has TLV Header?
    if (status) {
        addr_start = manual_cal_search_txpwr_tab(CALI_MAIN_RX, pt->partition_start_addr);  //TXPWR_TAB_FLASH_ADDR);
        if (!addr_start) {
            // no CALI_MAIN_RX, but has any other id, so attch CALI_MAIN_TX after the table
            len = flash_len + txpwr_len;
            buf = (UINT8*)os_malloc(len);
            if (!buf) {
                debug_print("no memory for save_cailmain_rx_tab\n");
                ddev_close(flash_handle);
                return 0;
            }
            // copy flash
            ddev_read(flash_handle, (char*)buf, flash_len, pt->partition_start_addr);  //TXPWR_TAB_FLASH_ADDR);
            ddev_close(flash_handle);

            // updata new TLV header
            tag_txpwr_ptr = (TAG_TXPWR_PTR)buf;
            tag_txpwr.head.type = BK_FLASH_OPT_TLV_HEADER;
            tag_txpwr.head.len = len - sizeof(TAG_TXPWR_ST);
            ;
            os_memcpy(tag_txpwr_ptr, &tag_txpwr, sizeof(TAG_TXPWR_ST));

            txpwr_buf = (UINT8*)(buf + flash_len);
            addr_start = pt->partition_start_addr;  //TXPWR_TAB_FLASH_ADDR;
        } else {
            // CALI_MAIN_RX is exist, and found start addr.
            len = txpwr_len;
            txpwr_buf = buf = (UINT8*)os_malloc(txpwr_len);
            if (!buf) {
                debug_print("no memory for txpwr tab save to flash\n");
                ddev_close(flash_handle);
                return 0;
            }
        }
    } else {
        // nothing in flash, write TLV with chipid
        debug_print("NO BK_FLASH_OPT_TLV_HEADER found, save_cailmain_rx_tab failed\n");
        ddev_close(flash_handle);
        return 0;
    }

    ddev_close(flash_handle);

    // for tag CALI_MAIN_TX
    tag_txpwr_ptr = (TAG_TXPWR_PTR)txpwr_buf;
    tag_txpwr.head.type = CALI_MAIN_RX;
    tag_txpwr.head.len = txpwr_len - sizeof(TAG_TXPWR_ST);
    os_memcpy(tag_txpwr_ptr, &tag_txpwr, sizeof(TAG_TXPWR_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_TXPWR_ST));

    bk7011_get_rx_err_wr(&rx_amp_err_wr, &rx_phase_err_wr, rx_dc_gain_tab);
    // for tag rx_dc_gain_tab
    tag_rx_dc_ptr = (TAG_RX_DC_PTR)(txpwr_buf);
    tag_rx_dc.head.type = CM_RX_DC_GAIN_TAB;
    tag_rx_dc.head.len = sizeof(tag_rx_dc.value);
    tag_rx_dc.value[0] = rx_dc_gain_tab[0];
    tag_rx_dc.value[1] = rx_dc_gain_tab[1];
    tag_rx_dc.value[2] = rx_dc_gain_tab[2];
    tag_rx_dc.value[3] = rx_dc_gain_tab[3];
    tag_rx_dc.value[4] = rx_dc_gain_tab[4];
    tag_rx_dc.value[5] = rx_dc_gain_tab[5];
    tag_rx_dc.value[6] = rx_dc_gain_tab[6];
    tag_rx_dc.value[7] = rx_dc_gain_tab[7];
    os_memcpy(tag_rx_dc_ptr, &tag_rx_dc, sizeof(TAG_RX_DC_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_RX_DC_ST));

    // for tag rx_amp_err_wr
    tag_comm_ptr = (TAG_COMM_PTR)(txpwr_buf);
    tag_comm.head.type = CM_RX_AMP_ERR_WR;
    tag_comm.head.len = sizeof(tag_comm.value);
    tag_comm.value = (UINT32)rx_amp_err_wr;
    os_memcpy(tag_comm_ptr, &tag_comm, sizeof(TAG_COMM_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_COMM_ST));

    // for tag rx_phase_err_wr
    tag_comm_ptr = (TAG_COMM_PTR)(txpwr_buf);
    tag_comm.head.type = CM_RX_PHASE_ERR_WR;
    tag_comm.head.len = sizeof(tag_comm.value);
    tag_comm.value = (UINT32)rx_phase_err_wr;
    os_memcpy(tag_comm_ptr, &tag_comm, sizeof(TAG_COMM_ST));
    txpwr_buf = (UINT8*)(txpwr_buf + sizeof(TAG_COMM_ST));

    addr_start -= pt->partition_start_addr;  //TXPWR_TAB_FLASH_ADDR;
    manual_cal_update_flash_area(addr_start, (char*)buf, len);
    os_free(buf);

    return 1;
}

int manual_cal_load_calimain_tag_from_flash(UINT32 tag, int* tag_addr, int tag_size) {
    UINT32 status, addr, addr_start;
    DD_HANDLE flash_handle;
    TXPWR_ELEM_ST head;
#if CFG_SUPPORT_ALIOS
    hal_logic_partition_t* pt = hal_flash_get_info(HAL_PARTITION_RF_FIRMWARE);
#else
    bk_logic_partition_t* pt = bk_flash_get_info(BK_PARTITION_RF_FIRMWARE);
#endif

    status = manual_cal_search_opt_tab(&addr);
    if (!status) {
        // no tlv in flash
        debug_print("NO TLV tag in flash \n");
        return -1;
    }

    if ((tag >= CM_TX_DCOR_MOD) && (tag < CM_TX_END)) {
        addr_start = manual_cal_search_txpwr_tab(CALI_MAIN_TX, pt->partition_start_addr);
    } else if ((tag >= CM_RX_DC_GAIN_TAB) && (tag < CM_RX_END)) {
        addr_start = manual_cal_search_txpwr_tab(CALI_MAIN_RX, pt->partition_start_addr);
    } else {
        debug_print("not a CALI_MAIN_tag \n");
        return 0;
    }

    if (!addr_start) {
        debug_print("NO CALI_MAIN_tag in flash \n");
        return -2;
    }

    addr = manual_cal_search_txpwr_tab(tag, addr_start);
    if (!addr) {
        debug_print("NO %x tag in flash\n", tag);
        return -3;
    } else {
        flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
        ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
        if ((tag_addr) && (tag_size >= head.len)) {
            ddev_read(flash_handle, (char*)tag_addr, head.len, addr + sizeof(TXPWR_ELEM_ST));
        }
        return 1;
    }
}

UINT32 manual_cal_txpwr_tab_ready_in_flash(void) {
    UINT32 status, addr, addr_start;
    DD_HANDLE flash_handle;
    TXPWR_ELEM_ST head;
    TXPWR_IS_RD is_ready_flash = 0;
#if CFG_SUPPORT_ALIOS
    hal_logic_partition_t* pt = hal_flash_get_info(HAL_PARTITION_RF_FIRMWARE);
#else
    bk_logic_partition_t* pt = bk_flash_get_info(BK_PARTITION_RF_FIRMWARE);
#endif

    addr_start = manual_cal_search_txpwr_tab(TXPWR_TAB_TAB, pt->partition_start_addr);  //TXPWR_TAB_FLASH_ADDR);
    if (!addr_start) {
        //debug_print("NO TXPWR_TAB_TAB found in flash\n");
        return TXPWR_NONE_RD;
    }

    addr = manual_cal_search_txpwr_tab(TXPWR_ENABLE_ID, addr_start);
    if (!addr) {
        //debug_print("NO TXPWR_ENABLE_ID found in flash\n");
        return TXPWR_NONE_RD;
    }

    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
    ddev_read(flash_handle, (char*)&status, head.len, addr + sizeof(TXPWR_ELEM_ST));
    is_ready_flash = status;
    //debug_print("flash txpwr table:0x%x\n", is_ready_flash);

    return is_ready_flash;
}

#endif  // CFG_SUPPORT_MANUAL_CALI

#if CFG_USE_TEMPERATURE_DETECT

#if CFG_TEMP_DETECT_VERSION == CFG_TEMP_DETECT_VERSION1
#define TMP_PWR_TAB_LEN 39

typedef struct tmp_set_pwr_st {
    UINT8 indx;
    UINT8 indx_cali_bias;
    UINT8 indx_cali_dpll;
    UINT8 flag;
    UINT16 temp_tab[TMP_PWR_TAB_LEN];
    TMP_PWR_PTR pwr_ptr;
} TMP_SET_PWR_ST, *TMP_SET_PWR_PTR;

const TMP_PWR_ST tmp_pwr_tab[TMP_PWR_TAB_LEN] = {
//trx0x0c[12:15], shift_b, shift_g, shift_ble, xtal_c_delta
#if (CFG_SOC_NAME == SOC_BK7231N)
    {0x00, -2, -4, 0, 0},  // 0     ,-40    -35
    {0x00, -2, -4, 0, 2},  // 1     ,-35    -30
    {0x00, -2, -4, 0, 4},  // 2     ,-30    -25
    {0x00, -1, -3, 0, 7},  // 3     ,-25    -20
    {0x00, -1, -3, 0, 7},  // 4     ,-20     -15
    {0x00, -1, -2, 0, 8},  // 5     ,-15  -10
    {0x00, 0, -1, 0, 8},   // 6     ,-10   -5
    {0x00, 0, -1, 0, 8},   // 7     ,-5     0
    {0x00, 0, -1, 0, 7},   // 8     ,0       5
    {0x00, 0, -1, 0, 7},   // 9     ,5      10
    {0x00, 0, -1, 0, 6},   // 10    ,10     15
    {0x00, 0, -1, 0, 6},   // 11    ,15    20
    {0x00, 0, 0, 0, 1},    // 12    ,20    25
    {0x00, 0, 0, 0, 0},    // 13    ,25    30
    {0x00, 0, 0, 0, -1},   // 14    ,30   35
    {0x00, 0, 0, 0, -4},   // 15    ,35   40
    {0x00, 0, 0, 0, -4},   // 16    ,40  45
    {0x00, 0, 0, 0, -4},   // 17    ,45    50
    {0x00, 0, 1, 0, -6},   // 18    ,50    55
    {0x00, 0, 1, 0, -8},   // 19    ,55   60
    {0x00, 1, 2, 0, -8},   // 20    ,60   65
    {0x00, 1, 2, 0, -8},   // 21    ,65   70
    {0x00, 1, 3, 0, -7},   // 22    ,70   75
    {0x00, 2, 4, 0, -6},   // 23    ,75   80
    {0x00, 2, 4, 0, -3},   // 24    ,80   85
    {0x00, 2, 4, 0, -1},   // 25    ,85  90
    {0x00, 2, 4, 0, 0},    // 26    ,90  95
    {0x00, 2, 4, 1, 5},    // 27    ,95  100
    {0x00, 2, 4, 1, 10},   // 28    ,100  105
    {0x00, 2, 4, 2, 20},   // 29    ,105  110
    {0x00, 2, 4, 2, 30},   // 30    ,110  115
    {0x00, 2, 4, 3, 40},   // 31    ,115
    {0x00, 2, 4, 3, 60},   // 32    ,120
    {0x00, 2, 4, 3, 80},   // 33    ,125
    {0x00, 2, 4, 3, 100},  // 34    ,130
    {0x00, 2, 4, 3, 127},  // 35    ,135
    {0x00, 2, 4, 3, 127},  // 36    ,140
    {0x00, 2, 4, 3, 127},  // 37    ,145
    {0x00, 2, 4, 3, 127},  // 38    ,150
#else
    {0x08, -4, -4, 0, 0},  // 0     ,-40
    {0x08, -4, -4, 0, 0},  // 1     ,-35
    {0x08, -4, -4, 0, 0},  // 2     ,-30
    {0x08, -4, -4, 0, 0},  // 3     ,-25
    {0x08, -4, -4, 0, 0},  // 4     ,-20
    {0x08, -3, -3, 0, 0},  // 5     ,-15
    {0x08, -3, -3, 0, 0},  // 6     ,-10
    {0x08, -3, -3, 0, 0},  // 7     ,-5
    {0x08, -2, -2, 0, 0},  // 8     ,0
    {0x08, -2, -2, 0, 0},  // 9     ,5
    {0x08, -2, -2, 0, 0},  // 10    ,10
    {0x08, -1, -1, 0, 0},  // 11    ,15
    {0x08, -1, -1, 0, 0},  // 12    ,20
    {0x08, -1, -1, 0, 0},  // 13    ,25
    {0x08, 0, 0, 0, 2},    // 14    ,30
    {0x08, 0, 0, 0, 1},    // 15    ,35
    {0x08, 0, 0, 0, 0},    // 16    ,40
    {0x08, 0, 0, 0, 0},    // 17    ,45
    {0x08, 0, 0, 0, 0},    // 18    ,50
    {0x08, 0, 0, 0, 0},    // 19    ,55
    {0x08, 0, 0, 0, 0},    // 20    ,60
    {0x08, 0, 0, 0, 0},    // 21    ,65
    {0x08, 0, 0, 0, 0},    // 22    ,70
    {0x08, 0, 0, 0, 0},    // 23    ,75
    {0x08, 0, 0, 0, 0},    // 24    ,80
    {0x08, 0, 0, 0, 5},    // 25    ,85
    {0x08, 0, 0, 0, 8},    // 26    ,90
    {0x08, 1, 1, 1, 11},   // 27    ,95
    {0x08, 1, 1, 1, 16},   // 28    ,100
    {0x08, 2, 2, 2, 22},   // 29    ,105
    {0x08, 2, 2, 2, 30},   // 30    ,110
    {0x08, 3, 3, 3, 40},   // 31    ,115
    {0x08, 3, 3, 3, 52},   // 32    ,120
    {0x08, 3, 3, 3, 63},   // 33    ,125
    {0x08, 3, 3, 3, 63},   // 34    ,130
    {0x08, 3, 3, 3, 63},   // 35    ,135
    {0x08, 3, 3, 3, 63},   // 36    ,140
    {0x08, 3, 3, 3, 63},   // 37    ,145
    {0x08, 3, 3, 3, 63},   // 38    ,150
#endif
};
#else

#define TMP_PWR_TAB_LEN 13

typedef struct tmp_set_pwr_st {
    UINT8 indx;
    UINT8 flag;
    UINT16 temp_tab[TMP_PWR_TAB_LEN];
    TMP_PWR_PTR pwr_ptr;
    UINT16 init_mod;
    UINT16 init_pa;
} TMP_SET_PWR_ST, *TMP_SET_PWR_PTR;
/*
const TMP_PWR_ST tmp_pwr_tab[TMP_PWR_TAB_LEN] = {
    { 0,  6},   // pwr index 0   5dbm
    { 0,  8},   // pwr index 1   6dbm
    { 1,  8},   // pwr index 2   7dbm
    { 2,  8},   // pwr index 3   8dbm
    { 3,  8},   // pwr index 4   9dbm
    { 4,  8},   // pwr index 5   10dbm
    { 6,  8},   // pwr index 6   11bm
    { 8,  8},   // pwr index 7   12dbm
    {11,  8},   // pwr index 8   13dbm
    {15,  8},   // pwr index 9   14dbm
    {15, 10},   // pwr index 10  15dbm
    {15, 12},   // pwr index 11  16dbm
    {15, 15},   // pwr index 12  17dbm
};
*/

const TMP_PWR_ST tmp_pwr_tab[TMP_PWR_TAB_LEN] = {
    {4, 8},   // pwr index 0   5dbm
    {4, 8},   // pwr index 1   6dbm
    {6, 8},   // pwr index 2   7dbm
    {6, 8},   // pwr index 3   8dbm
    {6, 8},   // pwr index 4   9dbm
    {8, 8},   // pwr index 5   10dbm
    {8, 8},   // pwr index 6   11bm
    {8, 8},   // pwr index 7   12dbm
    {8, 8},   // pwr index 8   13dbm
    {11, 8},  // pwr index 9   14dbm
    {11, 8},  // pwr index 10  15dbm
    {11, 8},  // pwr index 11  16dbm
    {11, 8},  // pwr index 12  17dbm
};
#endif

typedef struct xtal_st {
    UINT8 low;
    UINT8 high;
    UINT16 value;
} XTAL_ST, *XTAL_PTR;

const XTAL_ST xtal_tab[] = {
    {0, 20, 3},
    {20, 30, 5},
    {30, 40, 8},
    {40, 50, 12},
    {50, 60, 17},
    {60, 70, 23},
    {70, 80, 30},
    {80, 90, 38},
    {90, 100, 47},
    {100, 110, 57},
    {110, 0, 68},
};

TMP_SET_PWR_ST g_tmp_pwr;

extern void sctrl_cali_dpll(UINT8 flag);
extern void sctrl_dpll_int_open(void);

void manual_cal_init_xtal_cali(UINT16 init_temp, UINT16 init_xtal) {
#if (CFG_TEMP_DETECT_VERSION == CFG_TEMP_DETECT_VERSION1)
    g_xcali.init_xtal = init_xtal;
    g_xcali.last_xtal = init_xtal;
    debug_print("--init_xtal = %d\n", init_xtal);
#endif
}

void manual_cal_set_tmp_pwr_flag(UINT8 flag) {
    os_null_printf("set flag to %d\n", flag);
    g_tmp_pwr.flag = flag;
}

void manual_cal_tmp_pwr_init_reg(UINT16 reg_mod, UINT16 reg_pa) {
#if CFG_TEMP_DETECT_VERSION == CFG_TEMP_DETECT_VERSION1

#else
    TMP_DETECT_WARN("manual_cal_tmp_pwr_init_reg: mod:%d, pa:%d\n", reg_mod, reg_pa);
    g_tmp_pwr.init_mod = reg_mod;
    g_tmp_pwr.init_pa = reg_pa;
#endif
}

extern struct temp_cal_pwr_st g_temp_pwr_current;
extern struct temp_cal_pwr_st g_temp_pwr_current_tpc;
extern INT16 g_ble_pwr_indx, g_ble_pwr_shift;
extern void rwnx_cal_set_txpwr_by_tmpdetect(INT16 shift_b, INT16 shift_g);
UINT16 g_tmp_pwr_indx_bak = 255;
void manual_cal_store_cur_temp(void) {
    g_tmp_pwr_indx_bak = g_tmp_pwr.indx;

    debug_print("restore g_tmp_pwr.indx:%d\n", g_tmp_pwr_indx_bak);
}

void manual_cal_recover_cur_temp(void) {
    if (g_tmp_pwr_indx_bak == 255)
        g_tmp_pwr.indx = g_tmp_pwr.indx;
    else
        g_tmp_pwr.indx = g_tmp_pwr_indx_bak;

    debug_print("recover g_tmp_pwr.indx:%d\n", g_tmp_pwr.indx);
}

extern void rwnx_set_tpc_txpwr_by_tmpdetect(INT16 shift_b, INT16 shift_g);
extern void rwnx_cal_set_ble_txpwr_by_tmpdetect(INT16 shift_ble);
void manual_cal_temp_pwr_unint(void) {
#if CFG_TEMP_DETECT_VERSION == CFG_TEMP_DETECT_VERSION1
    rwnx_cal_set_txpwr_by_tmpdetect(0, 0);
    rwnx_set_tpc_txpwr_by_tmpdetect(0, 0);

#if (CFG_SOC_NAME != SOC_BK7231N)
    rwnx_cal_set_ble_txpwr_by_tmpdetect(0);
#endif

    manual_cal_do_xtal_temp_delta_set(0);
    manual_cal_do_xtal_cali(0, 0, 0, 0);

    debug_print("stop pwr info:     pwridx:%d, shift_b:%d, shift_g:%d, rate:%d\n",
                g_temp_pwr_current.idx,
                g_temp_pwr_current.shift,
                g_temp_pwr_current.shift_g,
                g_temp_pwr_current.mode);

    debug_print("stop pwr tpc info: pwridx:%d, shift_b:%d, shift_g:%d, rate:%d\n",
                g_temp_pwr_current_tpc.idx,
                g_temp_pwr_current_tpc.shift,
                g_temp_pwr_current_tpc.shift_g,
                g_temp_pwr_current_tpc.mode);

#if (CFG_SOC_NAME != SOC_BK7231N)
    debug_print("stop pwr ble info: pwridx:%d, shift:%d\n",
                g_ble_pwr_indx, g_ble_pwr_shift);
#endif

    debug_print("stop td-indx:%d, tab:%d,",
                g_tmp_pwr.indx,
                g_tmp_pwr.temp_tab[g_tmp_pwr.indx]);

    debug_print("stop xtal info: init:%d, c_delta:%d\n",
                g_xcali.init_xtal, g_xcali.xtal_c_delta);

    debug_print("--0xc:%02x, pidx_b:%d, pidx_g:%d, X:%d\n",
                g_tmp_pwr.pwr_ptr->trx0x0c_12_15,
                g_tmp_pwr.pwr_ptr->p_index_delta,
                g_tmp_pwr.pwr_ptr->p_index_delta_g,
                g_tmp_pwr.pwr_ptr->xtal_c_dlta);
    manual_cal_store_cur_temp();
#else
    debug_print("manual_cal_temp_pwr_unint: mod:%d, pa:%d\n", g_tmp_pwr.init_mod,
                g_tmp_pwr.init_pa);
    rwnx_cal_set_reg_mod_pa(g_tmp_pwr.init_mod, g_tmp_pwr.init_pa);
#endif

    manual_cal_set_tmp_pwr_flag(0);
    debug_print("set flag to disable, don't do pwr any more:%d\n", g_tmp_pwr.flag);
}

#if CFG_TEMP_DETECT_VERSION == CFG_TEMP_DETECT_VERSION1
void manual_cal_tmp_pwr_init(UINT16 init_temp, UINT16 init_thre, UINT16 init_dist) {
    INT32 idx = 13, i;
    INT16 temp = 0;

    os_null_printf("init temp pwr table:tmp:%d, idx:%d, dist:%d, init_thre:%d\n",
                   init_temp, idx, init_dist, init_thre);

    if (init_temp >= ADC_TEMP_VAL_MAX) {
        debug_print("init temp too large %d, failed\n");
        return;
    }

    init_temp += init_dist;
    if (init_temp > ADC_TEMP_VAL_MAX)
        init_temp = ADC_TEMP_VAL_MAX;

    os_memset(&g_tmp_pwr.temp_tab[0], 0, sizeof(UINT16) * TMP_PWR_TAB_LEN);
    g_tmp_pwr.indx = idx;
    g_tmp_pwr.indx_cali_bias = idx;
    g_tmp_pwr.indx_cali_dpll = idx;
    g_tmp_pwr.pwr_ptr = (TMP_PWR_PTR)&tmp_pwr_tab[idx];
    g_tmp_pwr.temp_tab[idx] = init_temp;

    for (i = idx + 1; i < TMP_PWR_TAB_LEN; i++) {
        //temp = g_tmp_pwr.temp_tab[i-1] + ADC_TMEP_LSB_PER_10DEGREE;
#if (CFG_SOC_NAME == SOC_BK7231N)
        temp = g_tmp_pwr.temp_tab[i - 1] - (init_thre >> 1);
        temp = (temp < ADC_TEMP_VAL_MIN) ? ADC_TEMP_VAL_MIN : temp;
#else
        temp = g_tmp_pwr.temp_tab[i - 1] + init_thre;
        temp = (temp > ADC_TEMP_VAL_MAX) ? ADC_TEMP_VAL_MAX : temp;
#endif
        g_tmp_pwr.temp_tab[i] = temp;
    }

    for (i = idx - 1; i >= 0; i--) {
        //temp = g_tmp_pwr.temp_tab[i+1] - ADC_TMEP_LSB_PER_10DEGREE;
#if (CFG_SOC_NAME == SOC_BK7231N)
        temp = g_tmp_pwr.temp_tab[i + 1] + (init_thre >> 1);
        temp = (temp > ADC_TEMP_VAL_MAX) ? ADC_TEMP_VAL_MAX : temp;
#else
        temp = g_tmp_pwr.temp_tab[i + 1] - init_thre;
        temp = (temp < ADC_TEMP_VAL_MIN) ? ADC_TEMP_VAL_MIN : temp;
#endif
        g_tmp_pwr.temp_tab[i] = temp;
    }

    os_null_printf("Temp tab\n");
    for (i = 0; i < TMP_PWR_TAB_LEN; i++) {
        os_null_printf("%02d: %03d\n", i, g_tmp_pwr.temp_tab[i]);
    }
    os_null_printf("\n");

    if (temp_detect_is_init() == 0) {
        manual_cal_set_tmp_pwr_flag(1);
        os_null_printf("open set pwr flag for the first time: %d\n", g_tmp_pwr.flag);
    }

    temp_detect_init(init_temp);
}
#else
void manual_cal_tmp_pwr_init(UINT16 init_temp, UINT16 init_thre, UINT16 init_dist) {
    INT32 idx = TMP_PWR_TAB_LEN - 1, i;
    TMP_PWR_PTR pwr_ptr_0, pwr_ptr_1;
    UINT8 reg_mod, reg_pa;
    INT16 temp = 0;

    reg_mod = g_tmp_pwr.init_mod;
    reg_pa = g_tmp_pwr.init_pa;

    for (i = 0; i < TMP_PWR_TAB_LEN - 1; i++) {
        pwr_ptr_0 = (TMP_PWR_PTR)&tmp_pwr_tab[i];
        pwr_ptr_1 = (TMP_PWR_PTR)&tmp_pwr_tab[i + 1];
        if (((pwr_ptr_0->mod <= reg_mod) && (pwr_ptr_1->mod >= reg_mod)) && ((pwr_ptr_0->pa <= reg_pa) && (pwr_ptr_1->pa >= reg_pa))) {
            if ((pwr_ptr_1->pa == reg_pa) && (pwr_ptr_1->mod >= reg_mod))
                idx = i + 1;
            else
                idx = i;

            break;
        }
    }

    debug_print("init temp pwr table: mod:%d, pa:%d, tmp:%d, idx:%d, dist:%d\n",
                reg_mod, reg_pa, init_temp, idx, init_dist);

    if (init_temp < init_dist) {
        init_temp = ADC_TEMP_VAL_MIN;
    } else {
        init_temp -= init_dist;
    }

    os_memset(&g_tmp_pwr.temp_tab[0], 0, sizeof(UINT16) * TMP_PWR_TAB_LEN);
    g_tmp_pwr.indx = idx;
    g_tmp_pwr.pwr_ptr = (TMP_PWR_PTR)&tmp_pwr_tab[idx];
    g_tmp_pwr.temp_tab[idx] = init_temp;

    for (i = idx + 1; i < TMP_PWR_TAB_LEN; i++) {
        //temp = g_tmp_pwr.temp_tab[i-1] + ADC_TMEP_LSB_PER_10DEGREE;
        temp = g_tmp_pwr.temp_tab[i - 1] + init_thre;
        temp = (temp > ADC_TEMP_VAL_MAX) ? ADC_TEMP_VAL_MAX : temp;
        g_tmp_pwr.temp_tab[i] = temp;
    }

    for (i = idx - 1; i >= 0; i--) {
        //temp = g_tmp_pwr.temp_tab[i+1] - ADC_TMEP_LSB_PER_10DEGREE;
        temp = g_tmp_pwr.temp_tab[i + 1] - init_thre;
        temp = (temp < ADC_TEMP_VAL_MIN) ? ADC_TEMP_VAL_MIN : temp;
        g_tmp_pwr.temp_tab[i] = temp;
    }

    TMP_DETECT_WARN("\n");
    for (i = 0; i < TMP_PWR_TAB_LEN; i++) {
        TMP_DETECT_WARN("%02d: %03d\n", i, g_tmp_pwr.temp_tab[i]);
    }
    TMP_DETECT_WARN("\n");

    if (temp_detect_is_init() == 0) {
        manual_cal_set_tmp_pwr_flag(1);
        debug_print("open set pwr flag for the first time: %d\n", g_tmp_pwr.flag);
    }

    temp_detect_init(init_temp);
}
#endif

int printf_count = 0;
int printf_is_allowed(void) {
    int ret = 0;

    printf_count++;
    if (10 == printf_count) {
        ret = 1;
        printf_count = 0;
    }

    return ret;
}

#define ATE_PRINT_DEBUG
#if (CFG_SOC_NAME == SOC_BK7231N)
extern void bk7011_cal_bias_low_temprature(UINT8 low_temprature);
TMP_PWR_PTR manual_cal_set_tmp_pwr(UINT16 cur_val, UINT16 thre, UINT16* last) {
#define DO_SETP (10)
    //change thre from ADC_TMEP_LSB_PER_10DEGREE to ADC_TMEP_LSB_PER_5DEGREE first
    UINT32 dist = 0;
    UINT32 thre_2_5_degree = (thre >> 2) + 2; /* -2.5 ~ +2.5 ===> 5, for tmp_pwr_tab */
    UINT32 thre_5_degree = (thre >> 1);
    UINT32 thre_50_degree = DO_SETP * thre_5_degree;
    UINT8 dist_cal_dpll = 0, dist_cal_bias = 0;
    UINT16 last_val = g_tmp_pwr.temp_tab[g_tmp_pwr.indx];

    UINT8 indx = g_tmp_pwr.indx;

    if (cur_val > last_val) {
        dist = cur_val - last_val;
        if (dist >= thre_50_degree) {
            indx = ((indx - DO_SETP) > 0) ? (indx - DO_SETP) : 0;
        } else if (dist >= thre_2_5_degree) {
            //round down to find step index
            int i = (dist - thre_2_5_degree) / thre_5_degree;
            indx = ((indx - (i + 1)) > 0) ? (indx - (i + 1)) : 0;
        }
    } else if (cur_val < last_val) {
        dist = last_val - cur_val;
        if (dist >= thre_50_degree) {
            indx = (indx + DO_SETP < TMP_PWR_TAB_LEN) ? (indx + DO_SETP) : (TMP_PWR_TAB_LEN);
        } else if (dist >= thre_2_5_degree) {
            //round down to find step index
            int i = (dist - thre_2_5_degree) / thre_5_degree;
            indx = ((indx + i + 1) < TMP_PWR_TAB_LEN) ? (indx + i + 1) : (TMP_PWR_TAB_LEN);
        }
    }

    if (g_tmp_pwr.indx == indx)
        return NULL;

    if (indx > g_tmp_pwr.indx_cali_dpll) {
        dist_cal_dpll = indx - g_tmp_pwr.indx_cali_dpll;
    } else {
        dist_cal_dpll = g_tmp_pwr.indx_cali_dpll - indx;
    }

    if (indx > g_tmp_pwr.indx_cali_bias) {
        dist_cal_bias = indx - g_tmp_pwr.indx_cali_bias;
    } else {
        dist_cal_bias = g_tmp_pwr.indx_cali_bias - indx;
    }

    /* cali bias each 4*5degree */
    if (dist_cal_bias >= 4) {
#ifdef ATE_PRINT_DEBUG
        if (!get_ate_mode_state()) {
            debug_print("cal_bias!\n");
        }
#endif
        g_tmp_pwr.indx_cali_bias = indx;
        bk7011_cal_bias();
    }

    /* cali dpll each 4*5degree */
    if (dist_cal_dpll >= 4) {
#ifdef ATE_PRINT_DEBUG
        if (!get_ate_mode_state()) {
            debug_print("cal dpll!\n");
        }
#endif

        g_tmp_pwr.indx_cali_dpll = indx;
        CHECK_OPERATE_RF_REG_IF_IN_SLEEP();
        sctrl_cali_dpll(0);
        sctrl_dpll_int_open();
        CHECK_OPERATE_RF_REG_IF_IN_SLEEP_END();
    }

    // bk7231U need dist>60, then do tx pwr.
    if (g_tmp_pwr.flag) {
        UINT8 last_idx = g_tmp_pwr.indx;
        temperature_type new_temperature_type;

        if (g_cur_temp_flash >= 7 * thre + cur_val) {
            // normal_temp_25C + 7 * ADC_TMEP_LSB_PER_10DEGREE = normal_temp_25C + 70C = 95C
            new_temperature_type = TEMPERATURE_TYPE_HIGH;
        } else if (cur_val >= 5 * thre_5_degree + g_cur_temp_flash) {
            // cur_temp >= normal_temp_25C - 5 * ADC_TMEP_LSB_PER_5DEGREE = normal_temp_25C - 25C = 0C
            new_temperature_type = TEMPERATURE_TYPE_LOW;
        } else {
            new_temperature_type = TEMPERATURE_TYPE_NORMAL;
        }
        bk7011_cal_vdddig_by_temperature(new_temperature_type);

        g_tmp_pwr.indx = indx;
        g_tmp_pwr.pwr_ptr = (TMP_PWR_PTR)&tmp_pwr_tab[indx];
        *last = g_tmp_pwr.temp_tab[g_tmp_pwr.indx];

        ///debug_print("set_tmp_pwr: indx:%d, mod:%d, pa:%d, tmp:%d\n", indx,
        //    g_tmp_pwr.pwr_ptr->mod, g_tmp_pwr.pwr_ptr->pa, *last);
#ifdef ATE_PRINT_DEBUG
        if (!get_ate_mode_state()) {
            debug_print("do td cur_t:%d--last:idx:%d,t:%d -- new:idx:%d,t:%d \n",
                        cur_val,
                        last_idx, g_tmp_pwr.temp_tab[last_idx],
                        g_tmp_pwr.indx, g_tmp_pwr.temp_tab[g_tmp_pwr.indx]);

            debug_print("--0xc:%02x, shift_b:%d, shift_g:%d, X:%d\n",
                        g_tmp_pwr.pwr_ptr->trx0x0c_12_15,
                        g_tmp_pwr.pwr_ptr->p_index_delta,
                        g_tmp_pwr.pwr_ptr->p_index_delta_g,
                        g_tmp_pwr.pwr_ptr->xtal_c_dlta);
        }
#endif

        return g_tmp_pwr.pwr_ptr;
        (void)last_idx;
    } else {
        g_tmp_pwr.indx = indx;
        return NULL;
    }
}
#else
TMP_PWR_PTR manual_cal_set_tmp_pwr(UINT16 cur_val, UINT16 thre, UINT16* last) {
#define DO_SETP (10)

    UINT32 dist = 0, thre_1 = (thre >> 1) + 2, thre_2 = thre_1 + (DO_SETP - 1) * thre;
    UINT8 need_cal_dpll = 0, need_cal_bais = 0;

    UINT16 last_val = g_tmp_pwr.temp_tab[g_tmp_pwr.indx];
    UINT8 indx = g_tmp_pwr.indx;

    if (cur_val > last_val) {
        dist = cur_val - last_val;
        if (dist >= thre_2) {
            indx = ((indx + DO_SETP) < TMP_PWR_TAB_LEN) ? (indx + DO_SETP) : TMP_PWR_TAB_LEN - 1;
            need_cal_dpll = 1;
            need_cal_bais = 1;
        } else if (dist >= thre_1) {
            int i;
            for (i = DO_SETP - 2; i >= 0; i--) {
                thre_2 = thre_1 + i * thre;
                if (dist >= thre_2) {
                    indx = ((indx + (i + 1)) < TMP_PWR_TAB_LEN) ? (indx + (i + 1)) : TMP_PWR_TAB_LEN - 1;
                    break;
                }
            }

            need_cal_bais = 1;

            if (i >= 1) {
                need_cal_dpll = 1;
            }
        }
    } else if (cur_val < last_val) {
        dist = last_val - cur_val;
        if (dist >= thre_2) {
            indx = (indx > DO_SETP) ? indx - DO_SETP : 0;
            need_cal_dpll = 1;
            need_cal_bais = 1;
        } else if (dist >= thre_1) {
            int i;
            for (i = DO_SETP - 2; i >= 0; i--) {
                thre_2 = thre_1 + i * thre;
                if (dist >= thre_2) {
                    indx = (indx > (i + 1)) ? (indx - (i + 1)) : 0;
                    break;
                }
            }

            need_cal_bais = 1;

            if (i >= 1) {
                need_cal_dpll = 1;
            }
        }
    }

    if (printf_is_allowed()
#if ATE_APP_FUN
        && (get_ate_mode_state() == 0)
#endif
    ) {
        debug_print("td cur:%d, last:%d, flag:%d\n", cur_val, last_val, g_tmp_pwr.flag);
    }

    if (g_tmp_pwr.indx == indx)
        return NULL;

    if (need_cal_dpll) {
        if (ble_in_dut_mode() == 0)
            debug_print("cal dpll!\n");

        CHECK_OPERATE_RF_REG_IF_IN_SLEEP();
        sctrl_cali_dpll(0);
        sctrl_dpll_int_open();
        CHECK_OPERATE_RF_REG_IF_IN_SLEEP_END();
    }

    if (need_cal_bais) {
        if (ble_in_dut_mode() == 0)
            debug_print("cal_bias!\n");
        bk7011_cal_bias();
    }

    // bk7231U need dist>60, then do tx pwr.
    if (g_tmp_pwr.flag) {
        UINT8 last_idx = g_tmp_pwr.indx;

        g_tmp_pwr.indx = indx;
        g_tmp_pwr.pwr_ptr = (TMP_PWR_PTR)&tmp_pwr_tab[indx];
        *last = g_tmp_pwr.temp_tab[g_tmp_pwr.indx];

        ///debug_print("set_tmp_pwr: indx:%d, mod:%d, pa:%d, tmp:%d\n", indx,
        //    g_tmp_pwr.pwr_ptr->mod, g_tmp_pwr.pwr_ptr->pa, *last);
        if (ble_in_dut_mode() == 0) {
            debug_print("do td cur_t:%d--last:idx:%d,t:%d -- new:idx:%d,t:%d \n",
                        cur_val,
                        last_idx, g_tmp_pwr.temp_tab[last_idx],
                        g_tmp_pwr.indx, g_tmp_pwr.temp_tab[g_tmp_pwr.indx]);

            debug_print("--0xc:%02x, shift_b:%d, shift_g:%d, X:%d\n",
                        g_tmp_pwr.pwr_ptr->trx0x0c_12_15,
                        g_tmp_pwr.pwr_ptr->p_index_delta,
                        g_tmp_pwr.pwr_ptr->p_index_delta_g,
                        g_tmp_pwr.pwr_ptr->xtal_c_dlta);
        }

        return g_tmp_pwr.pwr_ptr;
    } else {
        g_tmp_pwr.indx = indx;
        return NULL;
    }
}
#endif

void manual_cal_do_single_temperature(void) {
    UINT32 temp_value;

    if (!temp_single_get_current_temperature(&temp_value)) {
        UINT32 last_value = g_cur_temp;
        GLOBAL_INT_DECLARATION();

        GLOBAL_INT_DISABLE();
        //manual_cal_tmp_pwr_init(g_cur_temp, ADC_TMEP_LSB_PER_10DEGREE * ADC_TMEP_10DEGREE_PER_DBPWR, ADC_TMEP_DIST_INTIAL_VAL);
        manual_cal_recover_cur_temp();
        manual_cal_set_tmp_pwr_flag(1);
        rwnx_cal_do_temp_detect(temp_value, ADC_TMEP_LSB_PER_10DEGREE * ADC_TMEP_10DEGREE_PER_DBPWR, (UINT16*)&last_value);
        manual_cal_set_tmp_pwr_flag(0);

        GLOBAL_INT_RESTORE();

        debug_print("std set pwr: idx:%d, shift:%d, rate:%d\n",
                    g_temp_pwr_current.idx, g_temp_pwr_current.shift,
                    g_temp_pwr_current.mode);

        debug_print("std tpc set pwr: idx:%d, shift:%d, rate:%d\n",
                    g_temp_pwr_current_tpc.idx, g_temp_pwr_current_tpc.shift,
                    g_temp_pwr_current_tpc.mode);

        debug_print("std done:%0d\n", temp_value);
    } else {
        debug_print("std failed\n", temp_value);
    }
}

void manual_cal_do_xtal_cali(UINT16 cur_val, UINT16* last, UINT16 thre, UINT16 init_val) {
    if (g_tmp_pwr.flag == 0)
        return;

#if CFG_TEMP_DETECT_VERSION == CFG_TEMP_DETECT_VERSION1
    INT32 param;

    param = g_xcali.init_xtal + g_xcali.xtal_c_delta;
    if (g_xcali.last_xtal != param) {
#ifdef ATE_PRINT_DEBUG
        debug_print("init_xtal:%d, delta:%d, last_xtal:%d\n",
                    g_xcali.init_xtal,
                    g_xcali.xtal_c_delta,
                    g_xcali.last_xtal);
#endif

        if (param > PARAM_XTALH_CTUNE_MASK) {
            param = PARAM_XTALH_CTUNE_MASK;
        } else if (param < 0) {
            param = 0;
        }

        sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_SET_XTALH_CTUNE, &param);
        g_xcali.last_xtal = param;

        //debug_print("do_xtal_cali: c-d:%d-%d, l:%d, h:%d, v:%d\n", cur_val, dist,
        //    p_xtal->low, p_xtal->high, param);
    }
#else
    INT16 dist = 0;
    XTAL_PTR p_xtal;
    UINT32 param, foud = 0;
    UINT32 i, tab_len = sizeof(xtal_tab) / sizeof(XTAL_ST);

    dist = cur_val - g_cur_temp_flash;

    p_xtal = (XTAL_PTR)&xtal_tab[tab_len - 1];

    if (dist <= thre) {
        param = init_val;
        foud = 1;
    } else if (dist > p_xtal->low + thre) {
        param = p_xtal->value + init_val;
        foud = 1;
    } else {
        for (i = 0; i < tab_len - 1; i++) {
            p_xtal = (XTAL_PTR)&xtal_tab[i];
            if ((dist > p_xtal->low + thre) && (dist <= p_xtal->high + thre)) {
                param = p_xtal->value + init_val;
                foud = 1;
                break;
            }
        }
    }

    if ((*last != param) && (foud)) {
        if (param > PARAM_XTALH_CTUNE_MASK)
            param = PARAM_XTALH_CTUNE_MASK;

        sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_SET_XTALH_CTUNE, &param);
        *last = param;

        debug_print("T0:%d,T1:%d,T:%d,X:%d, CT0:%d,CT:%d\n", g_cur_temp_flash, cur_val, thre,
                    dist, init_val, param);

        //debug_print("do_xtal_cali: c-d:%d-%d, l:%d, h:%d, v:%d\n", cur_val, dist,
        //    p_xtal->low, p_xtal->high, param);
    }
#endif
}

UINT32 manual_cal_load_temp_tag_flash(void) {
    g_cur_temp = g_cur_temp_flash = manual_cal_load_temp_tag_from_flash();
    // start temp dectect
    manual_cal_tmp_pwr_init(g_cur_temp, ADC_TMEP_LSB_PER_10DEGREE * ADC_TMEP_10DEGREE_PER_DBPWR,
                            ADC_TMEP_DIST_INTIAL_VAL);

    return g_cur_temp;
}

#endif  // CFG_USE_TEMPERATURE_DETECT

#if CFG_SARADC_CALIBRATE
UINT32 manual_cal_load_adc_cali_flash(void) {
    UINT32 status, addr, addr_start;
    DD_HANDLE flash_handle;
    TXPWR_ELEM_ST head;
#if CFG_SUPPORT_ALIOS
    hal_logic_partition_t* pt = hal_flash_get_info(HAL_PARTITION_RF_FIRMWARE);
#else
    bk_logic_partition_t* pt = bk_flash_get_info(BK_PARTITION_RF_FIRMWARE);
#endif

    addr_start = manual_cal_search_txpwr_tab(TXID, pt->partition_start_addr);  //TXPWR_TAB_FLASH_ADDR);
    if (!addr_start) {
        goto FAILURE;
    }

    addr = manual_cal_search_txpwr_tab(TXID_ADC, addr_start);
    if (!addr) {
        goto FAILURE;
    }

    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    ddev_read(flash_handle, (char*)&head, sizeof(TXPWR_ELEM_ST), addr);
    ddev_read(flash_handle, (char*)&saradc_val, head.len, addr + sizeof(TXPWR_ELEM_ST));

    debug_print("calibrate low value:[%x]\n", saradc_val.low);
    debug_print("calibrate high value:[%x]\n", saradc_val.high);

    return SARADC_SUCCESS;

FAILURE:
    return SARADC_FAILURE;
}
#endif

#endif  // (CFG_SOC_NAME != SOC_BK7231)
