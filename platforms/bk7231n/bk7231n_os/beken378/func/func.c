#include "include.h"
#include "func_pub.h"
#include "intc.h"
#include "rwnx.h"
#include "uart_pub.h"
#include "lwip_intf.h"
#include "param_config.h"
#include "saradc_pub.h"
#include "bk7011_cal_pub.h"

#ifndef FUNC_DEBUG
#define FUNC_DEBUG 0
#endif
#define debug_print(...)  do { if (FUNC_DEBUG) os_printf("[FUNC]"__VA_ARGS__); } while (0);


#if CFG_ROLE_LAUNCH
#include "role_launch.h"
#endif

#if CFG_SUPPORT_CALIBRATION
#include "bk7011_cal_pub.h"
#endif

#if CFG_UART_DEBUG 
#include "uart_debug_pub.h"
#endif

#if CFG_SDIO
#include "sdio_intf_pub.h"
#endif

#if CFG_USB
#include "fusb_pub.h"
#endif
#include "start_type_pub.h"
#include "BkDriverFlash.h"



UINT32 func_init_extended(void)
{
    char temp_mac[6];
    
	cfg_param_init();
    // load mac, init mac first
    wifi_get_mac_address(temp_mac, CONFIG_ROLE_NULL);
	
    manual_cal_load_bandgap_calm();
    debug_print("rwnxl_init\n");
    rwnxl_init();

#if CFG_UART_DEBUG 
	#ifndef KEIL_SIMULATOR
    debug_print("uart_debug_init\n");   
    uart_debug_init();
	#endif
#endif

#if (!CFG_SUPPORT_RTT)
    debug_print("intc_init\n");
    intc_init();
#endif

#if CFG_SUPPORT_CALIBRATION
	UINT32 is_tab_inflash = 0;
    debug_print("calibration_main\n");
    calibration_main();
    #if CFG_SUPPORT_MANUAL_CALI
	is_tab_inflash = manual_cal_load_txpwr_tab_flash();
    manual_cal_load_default_txpwr_tab(is_tab_inflash);
    #endif
    #if CFG_SARADC_CALIBRATE
    manual_cal_load_adc_cali_flash();
    #endif
    #if CFG_USE_TEMPERATURE_DETECT
    manual_cal_load_temp_tag_flash();
    #endif
	
    #if (CFG_SOC_NAME != SOC_BK7231)
    manual_cal_load_lpf_iq_tag_flash();
    manual_cal_load_xtal_tag_flash();
    #endif // (CFG_SOC_NAME != SOC_BK7231)
	
    rwnx_cal_initial_calibration();

	#if CFG_SUPPORT_MANUAL_CALI
	if (0)//(is_tab_inflash == 0)
	{
		manual_cal_fitting_txpwr_tab();
		manual_cal_save_chipinfo_tab_to_flash();
		manual_cal_save_txpwr_tab_to_flash();
	}
	#endif // CFG_SUPPORT_MANUAL_CALI
#if (CFG_SUPPORT_BLE && (CFG_SOC_NAME == SOC_BK7231N))
	extern void ble_update_tx_pwr(void);
	ble_update_tx_pwr();
#endif
#endif    

#if CFG_SDIO
    debug_print("sdio_intf_init\n");
    sdio_intf_init();
#endif

#if CFG_SDIO_TRANS
    debug_print("sdio_intf_trans_init\n");
    sdio_trans_init();
#endif


#if CFG_USB
    debug_print("fusb_init\n");
    fusb_init();
#endif

#if  CFG_USE_STA_PS
    debug_print("ps_init\n");
#endif

#if CFG_ROLE_LAUNCH
    rl_init();
#endif

	#if CFG_ENABLE_BUTTON
	key_initialization();
	#endif

#ifdef BEKEN_START_WDT
	bk_wdg_initialize(10000);
#endif

    debug_print("func_init_extended OVER!!!\n");
    debug_print("start_type:%d\n",bk_misc_get_start_type());
    return 0;
}

UINT32 func_init_basic(void)
{
    intc_init();
    hal_flash_init();

    return 0;
}

// eof
