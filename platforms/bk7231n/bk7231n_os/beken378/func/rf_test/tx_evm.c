#include "include.h"
#include "tx_evm_pub.h"
#include "tx_evm.h"

#include "mac_phy_bypass_pub.h"
#include "uart_pub.h"
#if ((CFG_SOC_NAME != SOC_BK7231) && (CFG_SUPPORT_BLE == 1))
#include "ble_pub.h"
#endif
#include "icu_pub.h"
#include "sys_ctrl_pub.h"
#include "reg_rc.h"

#include "mac.h"
#include "phy.h"
#include "hal_machw.h"
#include "me.h"
#include "mm.h"
#include "ke_task.h"
#include "vif_mgmt.h"

#include "drv_model_pub.h"
#include "target_util_pub.h"
#include "ke_event.h"

#include "arm_arch.h"
#include "rtos_pub.h"

#if CFG_TX_EVM_TEST
#define TX_2_4_G_CHANNEL_NUM (14)
#define EVM_MAC_PKT_CNT_UNLIMITED (0xFFFFFFFF)

UINT32 evm_mac_pkt_count = 0;
UINT32 evm_mac_pkt_max = EVM_MAC_PKT_CNT_UNLIMITED;
UINT32 evm_channel = EVM_DEFAULT_CHANNEL;
UINT32 evm_bandwidth = PHY_CHNL_BW_20;
UINT32 evm_rate = HW_RATE_1MBPS;
UINT32 evm_modul_format = FORMATMOD_NON_HT;
UINT32 evm_guard_i_tpye = 0; // LONG_GI;
UINT32 evm_pwr_idx = 0;
UINT32 evm_test_via_mac_flag = 0;

struct mac_addr const evm_mac_addr = {
    {0x7112, 0x7111, 0x7111}};

const UINT16 tx_freq_2_4_G[TX_2_4_G_CHANNEL_NUM] = {
    2412,
    2417,
    2422,
    2427,
    2432,
    2437,
    2442,
    2447,
    2452,
    2457,
    2462,
    2467,
    2472,
    2484};

void evm_bypass_set_single_carrier(SC_TYPE_T type, UINT32 rate)
{
    UINT32 reg;

    reg = REG_READ((REG_RC_BASE_ADDR + 0x00 * 4)); // RC_BEKEN_0x0 [31] : 1
    reg |= (1u << 31);
    REG_WRITE((REG_RC_BASE_ADDR + 0x00 * 4), reg);

    reg = REG_READ((REG_RC_BASE_ADDR + 0x4c * 4)); // RC_BEKEN_0x4c [31:30] : 1
    reg &= ~(0x3u << 30);
    reg |= (0x1u << 30);
#if (CFG_SOC_NAME == SOC_BK7231N)
    reg &= ~(0xFFFu << 0);
    reg &= ~(0xFFFu << 16);
#else
    reg &= ~(0x3FFu << 0);
    reg &= ~(0x3FFu << 16);
#endif
    if (type == SINGLE_CARRIER_11B)
    {
#if (CFG_SOC_NAME == SOC_BK7231N)
        reg |= (0x380u << 0);
        reg |= (0x380u << 16);
#else
        reg |= (0x118u << 0);
        reg |= (0x118u << 16);
#endif
    }
    else if (type == SINGLE_CARRIER_11G)
    {
#if (CFG_SOC_NAME == SOC_BK7231N)
        if ((rate >= 128) && (evm_bandwidth == PHY_CHNL_BW_40))
        {
            reg |= (0x658u << 0);
            reg |= (0x658u << 16);
        }
        else
        {
            reg |= (0x627u << 0);
            reg |= (0x627u << 16);
        }
#else
        reg |= (0x1A8u << 0);
        reg |= (0x1A8u << 16);
#endif
    }
    else
    {
        reg |= (0xDDu << 0);
        reg |= (0xDDu << 16);
    }
    REG_WRITE((REG_RC_BASE_ADDR + 0x4c * 4), reg);
}

void evm_bypass_mac_init(UINT32 channel, UINT32 bandwidth)
{
    struct phy_cfg_tag cfg;
    /*reset mm*/
    EVM_PRT("[EVM]reset_mm\r\n");
    hal_machw_stop();
    phy_stop();
    me_init();
    mm_init();
    ke_state_set(TASK_MM, MM_IDLE);

    /*start mm*/
    EVM_PRT("[EVM]phy init\r\n");
    cfg.parameters[0] = 1;
    cfg.parameters[1] = 0;
    phy_init(&cfg);

    EVM_PRT("[EVM]set channel:%d\r\n", channel);
    phy_set_channel(PHY_BAND_2G4, bandwidth, channel, channel, 0, PHY_PRIM);

    /* Put the HW in active state*/
    mm_active();

    /*disable rx*/
    nxmac_rx_cntrl_set(0);
}

void evm_init(UINT32 channel, UINT32 bandwidth)
{
    BOOL p2p = 0;
    UINT8 vif_idx;
    UINT8 vif_type = 2;
    struct phy_cfg_tag cfg;

    /*reset mm*/
    EVM_PRT("[EVM]reset_mm\r\n");
    hal_machw_stop();
    phy_stop();
    me_init();
    mm_init();
    ke_state_set(TASK_MM, MM_IDLE);

    /*config me*/
    EVM_PRT("[EVM]config_me\r\n");

    /*config me channel*/
    EVM_PRT("[EVM]config_me_channel\r\n");

    /*start mm*/
    EVM_PRT("[EVM]start_mm\r\n");
    cfg.parameters[0] = 1;
    cfg.parameters[1] = 0;
    phy_init(&cfg);

    phy_set_channel(PHY_BAND_2G4, bandwidth, channel, channel, 0, PHY_PRIM);
    //if(bandwidth == PHY_CHNL_BW_40)
    //   rs_set_trx_regs_extern();

    /*add mm interface*/
    EVM_PRT("[EVM]add_mm_interface\r\n");
    vif_mgmt_register(&evm_mac_addr, vif_type, p2p, &vif_idx);

    /* Put the HW in active state*/
    mm_active();

    /*disable rx*/
    nxmac_rx_cntrl_set(0);
}

UINT32 evm_bypass_mac_set_tx_data_length(UINT32 modul_format, UINT32 len, UINT32 rate, UINT32 bandwidth, UINT32 need_change)
{
    UINT32 ret, is_legacy_mode = 1;
    UINT32 param;

    if (0) //(need_change)
    {
        if (bandwidth == 0)
        {
            if ((1 == rate) || (2 == rate) || (5 == rate) || (6 == rate) || (128 == rate))
            {
                len = 1024;
            }
        }
        else
        {
            if ((128 == rate) || (129 == rate) || (130 == rate) || (131 == rate))
            {
                len = 1024;
            }
        }
    }

    if (modul_format >= 0x02) // 0x2: HT-MM;  0x3: HT-GF
        is_legacy_mode = 0;

    if (is_legacy_mode)
    {
        if (len > TX_LEGACY_DATA_LEN_MASK)
            len = TX_LEGACY_DATA_LEN_MASK;

        param = len;
        ret = sddev_control(MPB_DEV_NAME, MCMD_TX_LEGACY_SET_LEN, &param);
    }
    else
    {
        if (len > TX_HT_VHT_DATA_LEN_MASK)
            len = TX_HT_VHT_DATA_LEN_MASK;

        param = len;
        ret = sddev_control(MPB_DEV_NAME, MCMD_TX_HT_VHT_SET_LEN, &param);
    }

    EVM_PRT("[EVM]tx_mode_bypass_mac_set_length:%d, %d\r\n", modul_format, len);

    return ret;
}

UINT32 evm_bypass_mac_set_rate_mformat(UINT32 ppdu_rate, UINT32 m_format)
{
    UINT32 ret;
    MBPS_TXS_MFR_ST param;

    param.mod_format = m_format;
    param.rate = ppdu_rate;

    ret = sddev_control(MPB_DEV_NAME, MCMD_BYPASS_TX_SET_RATE_MFORMAT, &param);

    EVM_PRT("[EVM]tx_mode_bypass_mac_set_rate:%d, modf:%d\r\n", ppdu_rate, m_format);

    return ret;
}

UINT32 evm_bypass_mac_set_txdelay(UINT32 delay_us)
{
    UINT32 ret, param;

    param = delay_us;
    ret = sddev_control(MPB_DEV_NAME, MCMD_SET_TXDELAY, &param);

    EVM_PRT("[EVM]tx_mode_bypass_mac_set_txdelay:%d us\r\n", param);

    return ret;
}

void evm_bypass_mac_set_channel(UINT32 channel)
{
    channel = tx_freq_2_4_G[channel - 1];

    evm_channel = channel;
}

void evm_set_bandwidth(UINT32 bandwidth)
{
    UINT32 param;

    if (bandwidth >= PHY_CHNL_BW_80)
        return;

    evm_bandwidth = bandwidth;
    param = evm_bandwidth;
    sddev_control(MPB_DEV_NAME, MCMD_SET_BANDWIDTH, &param);
}

void evm_bypass_mac_set_guard_i_type(UINT32 gi_tpye)
{
    UINT32 param;

    if (gi_tpye > 2)
        return;

    param = gi_tpye;
    sddev_control(MPB_DEV_NAME, MCMD_SET_GI, &param);
}

void evm_bypass_mac(void)
{
    sddev_control(MPB_DEV_NAME, MCMD_TX_MODE_BYPASS_MAC, 0);
    EVM_PRT("[EVM]tx_mode_bypass_mac\r\n");
}

void evm_stop_bypass_mac(void)
{
    UINT32 reg;

    reg = REG_READ((REG_RC_BASE_ADDR + 0x00 * 4)); // RC_BEKEN_0x0 [31] : 0
    reg &= ~(1u << 31);
    REG_WRITE((REG_RC_BASE_ADDR + 0x00 * 4), reg);

    reg = REG_READ((REG_RC_BASE_ADDR + 0x4c * 4)); // RC_BEKEN_0x4c [31:30] : 0
    reg &= ~(0x3u << 30);
    REG_WRITE((REG_RC_BASE_ADDR + 0x4c * 4), reg);
    sddev_control(MPB_DEV_NAME, MCMD_STOP_BYPASS_MAC, 0);
    EVM_PRT("[EVM]tx_mode_stop_bypass_mac\r\n");
}

void evm_start_bypass_mac(void)
{
    //EVM_PRT("[EVM]tx_mode_stop_bypass_mac\r\n");
    sddev_control(MPB_DEV_NAME, MCMD_START_BYPASS_MAC, 0);
}

void evm_bypass_mac_test(void)
{
    evm_bypass_mac_init(evm_channel, evm_bandwidth);

    evm_bypass_mac();
    EVM_PRT("[EVM]test_bypass_mac\r\n");
}

void evm_bypass_ble_test_start(UINT32 channel)
{
#if ((CFG_SOC_NAME != SOC_BK7231) && (CFG_SUPPORT_BLE == 1))
    UINT32 param;
    param = PWD_BLE_CLK_BIT;

    UINT32 reg;

    reg = REG_READ((REG_RC_BASE_ADDR + 0x00 * 4)); // RC_BEKEN_0x0 [31] : 0
    reg &= ~(1u << 31);
    REG_WRITE((REG_RC_BASE_ADDR + 0x00 * 4), reg);

    reg = REG_READ((REG_RC_BASE_ADDR + 0x4c * 4)); // RC_BEKEN_0x4c [31:30] : 0
    reg &= ~(0x3u << 30);
    REG_WRITE((REG_RC_BASE_ADDR + 0x4c * 4), reg);

    sddev_control(SCTRL_DEV_NAME, CMD_BLE_RF_BIT_SET, NULL);
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BLE_POWERUP, NULL);
    sddev_control(ICU_DEV_NAME, CMD_TL410_CLK_PWR_UP, &param);

    param = 0x3ba7a940;
    sddev_control(BLE_DEV_NAME, CMD_BLE_SET_GFSK_SYNCWD, &param);
    sddev_control(BLE_DEV_NAME, CMD_BLE_AUTO_CHANNEL_DISABLE, NULL);
    sddev_control(BLE_DEV_NAME, CMD_BLE_AUTO_SYNCWD_DISABLE, NULL);

    if (channel < 2400)
    {
        channel = 2400;
    }
    param = (channel - 2400) & 0x7F;
    sddev_control(BLE_DEV_NAME, CMD_BLE_SET_CHANNEL, &param);
    param = PN9_TX;
    sddev_control(BLE_DEV_NAME, CMD_BLE_SET_PN9_TRX, &param);
#endif
}

void evm_bypass_ble_test_stop(void)
{
#if ((CFG_SOC_NAME != SOC_BK7231) && (CFG_SUPPORT_BLE == 1))
    UINT32 reg;

    reg = REG_READ((REG_RC_BASE_ADDR + 0x00 * 4)); // RC_BEKEN_0x0 [31] : 0
    reg &= ~(1u << 31);
    REG_WRITE((REG_RC_BASE_ADDR + 0x00 * 4), reg);

    reg = REG_READ((REG_RC_BASE_ADDR + 0x4c * 4)); // RC_BEKEN_0x4c [31:30] : 0
    reg &= ~(0x3u << 30);
    REG_WRITE((REG_RC_BASE_ADDR + 0x4c * 4), reg);

    sddev_control(SCTRL_DEV_NAME, CMD_BLE_RF_BIT_CLR, NULL);
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BLE_POWERDOWN, NULL);
#endif
}

void evm_via_mac_evt(int dummy)
{
    ke_evt_clear(KE_EVT_EVM_MAC_BIT);

    if (evm_mac_pkt_max == EVM_MAC_PKT_CNT_UNLIMITED)
    {
        rtos_delay_milliseconds(10);
        // un limited, send forever
        evm_req_tx(&evm_mac_addr);
    }
    else
    {
        if (evm_mac_pkt_count < evm_mac_pkt_max)
        {
            evm_req_tx(&evm_mac_addr);
        }
        else
        {
            evm_test_via_mac_flag = 0;
            EVM_PRT("[EVM]test by mac cnt to max, stop:%d\r\n", evm_mac_pkt_max);
        }
        evm_mac_pkt_count++;
    }
}

uint32_t evm_via_mac_is_start(void)
{
    return evm_test_via_mac_flag;
}

void evm_via_mac_init(void)
{
    evm_init(evm_channel, evm_bandwidth);
}

void evm_via_mac_begin(void)
{
    evm_test_via_mac_flag = 1;
    evm_via_mac_evt(0);
}

void evm_via_mac_continue(void)
{
    if (0 == evm_test_via_mac_flag)
    {
        return;
    }

    ke_evt_set(KE_EVT_EVM_MAC_BIT);
}

void evm_via_mac_set_rate(HW_RATE_E rate, uint32_t modul_format, uint32_t guard_i_tpye)
{
    evm_rate = rate;
    evm_modul_format = modul_format;
    evm_guard_i_tpye = guard_i_tpye;

    EVM_PRT("[EVM]test by mac, rate:%d, m:%d, gi:%d\r\n", rate, modul_format, guard_i_tpye);
}

void evm_via_mac_set_channel(UINT32 channel)
{
    channel = tx_freq_2_4_G[channel - 1];
    evm_channel = channel;

    //evm_via_mac_begin();
}

void evm_via_mac_set_power(UINT32 pwr_idx)
{
    evm_pwr_idx = pwr_idx;
}

void evm_via_mac_set_bandwidth(UINT32 bandwidth)
{
    if (bandwidth >= PHY_CHNL_BW_80)
        return;

    evm_bandwidth = bandwidth;
}

#else

void evm_via_mac_evt(int dummy)
{
}

#endif // CFG_TX_EVM_TEST
// eof
