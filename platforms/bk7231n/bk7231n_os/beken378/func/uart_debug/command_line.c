#include "include.h"
#include "command_line.h"
#include "command_table.h"
#include "str_pub.h"
#include "uart_pub.h"
#include "mem_pub.h"
#if CFG_SUPPORT_ALIOS
#include "hal/soc/soc.h"
#else
#include "BkDriverFlash.h"
#endif
#include "wlan_ui_pub.h"
#include "ate_app.h"

#if CFG_UART_DEBUG
/* find command table entry for a command */
cmd_tbl_t *cmd_find_tbl(const char *cmd, cmd_tbl_t *table, int table_len)
{
    cmd_tbl_t *cmdtp;
    cmd_tbl_t *cmdtp_temp = table;	/* Init value */
    const char *p;
    int len;
    int n_found = 0;

    if (!cmd)
        return NULL;
    /*
     * Some commands allow length modifiers (like "cp.b");
     * compare command name only until first dot.
     */
    len = ((p = os_strchr(cmd, '.')) == NULL) ? os_strlen (cmd) : (p - cmd);

    for (cmdtp = table; cmdtp != table + table_len; cmdtp++)
    {
        if (os_strncmp(cmd, cmdtp->name, len) == 0)
        {
            if (len == os_strlen(cmdtp->name))
                return cmdtp;	/* full match */

            cmdtp_temp = cmdtp;	/* abbreviated command ? */
            n_found++;
        }
    }
    if (n_found == 1)  			/* exactly one match */
    {
        return cmdtp_temp;
    }

    return NULL;	/* not found or ambiguous command */
}

cmd_tbl_t *cmd_find(const char *cmd)
{
    cmd_tbl_t *start = entry_get_start();
    const int len = entry_get_count();

    return cmd_find_tbl(cmd, start, len);
}

int cmd_usage(const cmd_tbl_t *cmdtp)
{
    CLI_PRT("%s\r\n", cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
    if (!cmdtp->help)
    {
        CLI_PRT ("- No additional help available.\n");
        return 1;
    }

    CLI_PRT(cmdtp->help);
    CLI_PRT("\r\n");
#endif	/* CONFIG_SYS_LONGHELP */

    return 0;
}

/**
 * Call a command function. This should be the only route in U-Boot to call
 * a command, so that we can track whether we are waiting for input or
 * executing a command.
 *
 * @param cmdtp		Pointer to the command to execute
 * @param flag		Some flags normally 0 (see CMD_FLAG_.. above)
 * @param argc		Number of arguments (arg 0 must be the command text)
 * @param argv		Arguments
 * @return 0 if command succeeded, else non-zero (CMD_RET_...)
 */
static int cmd_call(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
    int result;

    result = (cmdtp->cmd)(cmdtp, flag, argc, argv);
    if (result)
        CLI_PRT("Command failed, result=%d\n", result);

    return result;
}

int cmd_process(int flag, int argc, char *const argv[],
                int *repeatable, unsigned int *ticks)
{
    int rc = CMD_RET_SUCCESS;
    cmd_tbl_t *cmdtp;

    /* Look up command in command table */
    cmdtp = cmd_find(argv[0]);
    if (cmdtp == NULL)
    {
        CLI_PRT("Unknown command '%s' - try 'help'\n", argv[0]);
        return CMD_RET_FAILURE;
    }

    if(DEBUG_PARSER)
    {
        CLI_PRT("    cmd_find\r\n");
    }

    CLI_PRT("\r\n    -------------------------------------------------------------------\r\n");

    /* found - check max args */
    if (argc > cmdtp->maxargs)
        rc = CMD_RET_USAGE;

    /* If OK so far, then do the command */
    if (!rc)
    {
        rc = cmd_call(cmdtp, flag, argc, argv);

        *repeatable &= cmdtp->repeatable;
    }
    if (rc == CMD_RET_USAGE)
        rc = cmd_usage(cmdtp);

    CLI_PRT("\r\n    -------------------------------------------------------------------\r\n");
    return rc;
}

int cmd_parse_line(char *line, char *argv[])
{
    int nargs = 0;

    while (nargs < CONFIG_SYS_MAXARGS)
    {
        /* skip any white space */
        while (isblank(*line))
            ++line;

        if (*line == '\0')  	/* end of line, no more args	*/
        {
            argv[nargs] = NULL;
            return nargs;
        }

        argv[nargs++] = line;	/* begin of argument string	*/

        /* find end of string */
        while (*line && !isblank(*line))
            ++line;

        if (*line == '\0')  	/* end of line, no more args	*/
        {
            argv[nargs] = NULL;
            return nargs;
        }

        *line++ = '\0';		/* terminate current arg	 */
    }

    CLI_PRT("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

    return nargs;
}

void cmd_process_macros(const char *input, char *output)
{
    char c, prev;
    int inputcnt = os_strlen(input);
    int outputcnt = CONFIG_SYS_CBSIZE;
    int state = 0;		/* 0 = waiting for '$'  */

    /* 1 = waiting for '(' or '{' */
    /* 2 = waiting for ')' or '}' */
    /* 3 = waiting for '''  */

    prev = '\0';		/* previous character   */

    while (inputcnt && outputcnt)
    {
        c = *input++;
        inputcnt--;

        if (state != 3)
        {
            /* remove one level of escape characters */
            if ((c == '\\') && (prev != '\\'))
            {
                if (inputcnt-- == 0)
                    break;
                prev = c;
                c = *input++;
            }
        }

        switch (state)
        {
        case 0:	/* Waiting for (unescaped) $    */
            if ((c == '\'') && (prev != '\\'))
            {
                state = 3;
                break;
            }
            if ((c == '$') && (prev != '\\'))
            {
                state++;
            }
            else
            {
                *(output++) = c;
                outputcnt--;
            }
            break;
        case 1:	/* Waiting for (        */
            if (c == '(' || c == '{')
            {
                state++;
            }
            else
            {
                state = 0;
                *(output++) = '$';
                outputcnt--;

                if (outputcnt)
                {
                    *(output++) = c;
                    outputcnt--;
                }
            }
            break;
        case 2:	/* Waiting for )        */
            if (c == ')' || c == '}')
            {
                char *envval;
                /* Varname # of chars */

                /* Get the varname */

                /* Get its value */
                envval = 0; // getenv(envname);

                /* Copy into the line if it exists */
                if (envval != NULL)
                    while ((*envval) && outputcnt)
                    {
                        *(output++) = *(envval++);
                        outputcnt--;
                    }
                /* Look for another '$' */
                state = 0;
            }
            break;
        case 3:	/* Waiting for '        */
            if ((c == '\'') && (prev != '\\'))
            {
                state = 0;
            }
            else
            {
                *(output++) = c;
                outputcnt--;
            }
            break;
        }
        prev = c;
    }

    if (outputcnt)
        *output = 0;
    else
        *(output - 1) = 0;
}


/*
 * Run a command using the selected parser.
 *
 * @param cmd	Command to run
 * @param flag	Execution flags (CMD_FLAG_...)
 * @return 0 on success, or != 0 on error.
 */
int run_command(const char *cmd, int flag)
{
    char cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
    char *token;			/* start of token in cmdbuf	*/
    char *sep;			/* end of token (separator) in cmdbuf */
    char finaltoken[CONFIG_SYS_CBSIZE];
    char *str = cmdbuf;
    char *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
    int argc, inquotes;
    int repeatable = 1;
    int rc = 0;

    if (DEBUG_PARSER
            && (cmd[0]))
    {
        /* use CLI_PRT - string may be loooong */
        CLI_PRT("run_command: ");
        CLI_PRT(cmd ? cmd : "NULL");
    }

    /* forget any previous Control C */
    if (!cmd || !*cmd)
        return -1;	/* empty command */

    if (os_strlen(cmd) >= CONFIG_SYS_CBSIZE)
    {
        CLI_PRT("## Command too long!\n");
        return -1;
    }

    os_strcpy(cmdbuf, cmd);

    /* Process separators and check for invalid
     * repeatable commands
     */

    while (*str)
    {
        /*
         * Find separator, or string end
         * Allow simple escape of ';' by writing "\;"
         */
        for (inquotes = 0, sep = str; *sep; sep++)
        {
            if ((*sep == '\'') &&
                    (*(sep - 1) != '\\'))
                inquotes = !inquotes;

            if (!inquotes &&
                    (*sep == ';') &&	/* separator		*/
                    (sep != str) &&	/* past string start	*/
                    (*(sep - 1) != '\\'))	/* and NOT escaped */
                break;
        }

        /*
         * Limit the token to data between separators
         */
        token = str;
        if (*sep)
        {
            str = sep + 1;	/* start of command for next pass */
            *sep = '\0';
        }
        else
        {
            str = sep;	/* no more commands for next pass */
        }

        /* find macros in this token and replace them */
        cmd_process_macros(token, finaltoken);

        /* Extract arguments */
        argc = cmd_parse_line(finaltoken, argv);
        if (argc == 0)
        {
            rc = -1;	/* no command at all */
            continue;
        }

        if (cmd_process(flag, argc, argv, &repeatable, NULL))
            rc = -1;

        /* Did the user stop this? */
    }

    return rc ? rc : repeatable;
}

#elif CFG_ENABLE_ATE_FEATURE
extern void bk_send_byte(UINT8 uport, UINT8 data);

void bkreg_tx(HCI_EVENT_PACKET *pHCItxBuf)
{
#ifndef KEIL_SIMULATOR
    char *tmp;
    unsigned int i;
    unsigned int tx_len = HCI_EVENT_HEAD_LENGTH + pHCItxBuf->total;

    pHCItxBuf->code  = TRA_HCIT_EVENT;
    pHCItxBuf->event = HCI_COMMAND_COMPLETE_EVENT;

    tmp = (char *)pHCItxBuf;
    for(i = 0; i < tx_len; i ++)
    {
        bk_send_byte(0,tmp[i]);//BK_UART_1
    }
#endif
}

cmd_tbl_t *cmd_find_tbl(const char *cmd, cmd_tbl_t *table, int table_len)
{
    return 0;
}

int cmd_usage(const cmd_tbl_t *cmdtp)
{
    return 0;
}
int run_command(const char *cmd, int flag)
{
    return 0;
}

#if CFG_SUPPORT_BKREG

#if CFG_USE_TUYA_CCA_TEST
/**
 * @name RWNXAGCCCA1 register definitions
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  28:20     INBDCCAPOWMINDBM   0x1B5
 *  19:12        CCAFALLTHRDBM   0xBF
 *     09             DISCCAEN   1
 *     08             SATCCAEN   1
 *  07:00        CCARISETHRDBM   0xC2
 * </pre>
 *
 * @{
 */

#define AGC_RWNXAGCCCA1_ADDR_dbg          (0x01002074)

#define AGC_INBDCCAPOWMINDBM_MASK_dbg   ((UINT32)0x1FF00000)
/// INBDCCAPOWMINDBM field LSB position
#define AGC_INBDCCAPOWMINDBM_LSB_dbg    20
/// INBDCCAPOWMINDBM field width
#define AGC_INBDCCAPOWMINDBM_WIDTH_dbg  ((UINT32)0x00000009)
/// CCAFALLTHRDBM field mask
#define AGC_CCAFALLTHRDBM_MASK_dbg      ((UINT32)0x000FF000)
/// CCAFALLTHRDBM field LSB position
#define AGC_CCAFALLTHRDBM_LSB_dbg       12
/// CCAFALLTHRDBM field width
#define AGC_CCAFALLTHRDBM_WIDTH_dbg     ((UINT32)0x00000008)
/// DISCCAEN field bit
#define AGC_DISCCAEN_BIT_dbg            ((UINT32)0x00000200)
/// DISCCAEN field position
#define AGC_DISCCAEN_POS_dbg            9
/// SATCCAEN field bit
#define AGC_SATCCAEN_BIT_dbg            ((UINT32)0x00000100)
/// SATCCAEN field position
#define AGC_SATCCAEN_POS_dbg            8
/// CCARISETHRDBM field mask
#define AGC_CCARISETHRDBM_MASK_dbg      ((UINT32)0x000000FF)
/// CCARISETHRDBM field LSB position
#define AGC_CCARISETHRDBM_LSB_dbg       0
/// CCARISETHRDBM field width
#define AGC_CCARISETHRDBM_WIDTH_dbg     ((UINT32)0x00000008)

#endif

#include "bk7011_cal_pub.h"
int bkreg_run_command(const char *content, int cnt)
{
    char tx_buf[BKREG_TX_FIFO_THRD];
    UINT32 uart_rx_index;
    REGISTER_PARAM *rx_param;
    REGISTER_PARAM *tx_param;
    HCI_EVENT_PACKET   *pHCItxBuf = (HCI_EVENT_PACKET *)tx_buf;
    HCI_COMMAND_PACKET *pHCIrxBuf = (HCI_COMMAND_PACKET *)content;

    uart_rx_index = cnt;
    pHCItxBuf->total = 1;
    pHCItxBuf->param[0] = pHCIrxBuf->cmd;
    switch(pHCIrxBuf->cmd)
    {
    case BEKEN_UART_REGISTER_WRITE_CMD:
        rx_param        = (REGISTER_PARAM *)pHCIrxBuf->param;
        REG_WRITE(rx_param->addr, rx_param->value);

        pHCItxBuf->total = uart_rx_index - 1;
        os_memcpy(pHCItxBuf->param, pHCIrxBuf, HCI_EVENT_HEAD_LENGTH);
        pHCItxBuf->param[3] = pHCIrxBuf->cmd;

        tx_param = (REGISTER_PARAM *)&pHCItxBuf->param[HCI_COMMAND_HEAD_LENGTH];
        tx_param->addr      = rx_param->addr;
        tx_param->value     = rx_param->value;

#if CFG_SUPPORT_CALIBRATION
        {
            extern INT32 rwnx_cal_save_trx_rcbekn_reg_val(void);
            // when write trx and rc beken regs, updata registers save.
            if( (rx_param->addr & 0xfff0000) == 0x1050000)
                rwnx_cal_save_trx_rcbekn_reg_val();
        }
#endif
        break;

    case BEKEN_UART_REGISTER_READ_CMD:
        rx_param        = (REGISTER_PARAM *)pHCIrxBuf->param;
        pHCItxBuf->total         = HCI_EVENT_HEAD_LENGTH + uart_rx_index; /*may err, porting from bk3260*/
        os_memcpy(pHCItxBuf->param, pHCIrxBuf, HCI_EVENT_HEAD_LENGTH);
        pHCItxBuf->param[3]      = pHCIrxBuf->cmd;

        tx_param = (REGISTER_PARAM *)&pHCItxBuf->param[HCI_COMMAND_HEAD_LENGTH];
        tx_param->addr  = rx_param->addr;

        if(rx_param->addr == 0x00800014) {
            #if ATE_APP_FUN
            if(get_ate_mode_state())
                tx_param->value = 0x1B190104;   // testmode flag[31:28] | bk7231U:B [27:24] | date
            else
            #endif
                tx_param->value = 0x0B190104;
        }
        else
            tx_param->value = REG_READ(rx_param->addr);
        break;

    case BEKEN_WRITE_OTP_CMD: {
        UINT8 *flag_ptr = NULL, *write_buf = NULL;
        UINT8 flag = 0;
        rx_param        = (REGISTER_PARAM *)pHCIrxBuf->param;
        write_buf       = (UINT8 *)(rx_param + 1);
        
        flag = manual_cal_wirte_otp_flash(rx_param->addr, rx_param->value, write_buf);
        
        pHCItxBuf->total = OTP_CMD_RET_LEN + 1;
        os_memcpy(pHCItxBuf->param, pHCIrxBuf, HCI_EVENT_HEAD_LENGTH);
        pHCItxBuf->param[3] = pHCIrxBuf->cmd;

        tx_param = (REGISTER_PARAM *)&pHCItxBuf->param[HCI_COMMAND_HEAD_LENGTH];
        tx_param->addr      = rx_param->addr;
        tx_param->value     = rx_param->value;

        flag_ptr = (UINT8 *)&pHCItxBuf->param[OTP_CMD_RET_LEN];
        *flag_ptr = flag;
        }
        break;

    case BEKEN_READ_OTP_CMD: { 
        UINT32 len, len_left = 0, addr;
        UINT8 *read_buf = (UINT8 *)&pHCItxBuf->param[OTP_CMD_RET_LEN];
        
        rx_param        = (REGISTER_PARAM *)pHCIrxBuf->param;

#if CFG_SUPPORT_ALIOS
		hal_logic_partition_t *pt = hal_flash_get_info(HAL_PARTITION_RF_FIRMWARE);
#else
		bk_logic_partition_t *pt = bk_flash_get_info(BK_PARTITION_RF_FIRMWARE);
#endif

        len_left = rx_param->value;
        addr = rx_param->addr - pt->partition_start_addr;//0xFA000;
        
        while(len_left) {
            len = (len_left > OTP_READ_MAX_LEN)? OTP_READ_MAX_LEN : len_left;
            len = manual_cal_read_otp_flash(addr, len, read_buf);
            if(len == 0)
                return 0;

            pHCItxBuf->total = OTP_CMD_RET_LEN + len;
            os_memcpy(pHCItxBuf->param, pHCIrxBuf, HCI_EVENT_HEAD_LENGTH);
            pHCItxBuf->param[3] = pHCIrxBuf->cmd;

            tx_param = (REGISTER_PARAM *)&pHCItxBuf->param[HCI_COMMAND_HEAD_LENGTH];
            tx_param->addr      = rx_param->addr + addr;
            tx_param->value     = len;
            bkreg_tx(pHCItxBuf);
            len_left -= len;
            addr += len;
        }
        return 0;
        }
        break;

    case BEKEN_TEMP_CMD: {
        #if CFG_USE_AUD_DAC
        extern void audio_intf_dac_pause(void);
        audio_intf_dac_pause();
        #endif
        
        #if CFG_USE_AUD_ADC
        extern void audio_intf_adc_pause(void);
        audio_intf_adc_pause();
        #endif
        
        }
        break;

    case BEKEN_TEMP_TCP: {
        #if CFG_USE_AUD_DAC
        extern void audio_intf_dac_play(void);
        audio_intf_dac_play();
        #endif

        #if CFG_USE_AUD_ADC
        extern void audio_intf_adc_play(void);
        audio_intf_adc_play();
        #endif
        }
        break;

    case BEKEN_TEST_UDP: {
        #if CFG_SUPPORT_MANUAL_CALI
        manual_cal_show_txpwr_tab();
        #endif
        }
        break;

    case BEKEN_SD_CLOSE: {
        #if CFG_SUPPORT_MANUAL_CALI
        manual_cal_fitting_txpwr_tab();
        #endif
        }
        break;
#if (CFG_SOC_NAME == SOC_BK7231U) || (CFG_SOC_NAME == SOC_BK7231N)
    case BEKEN_TUYA_PWM: {
        UINT32 u32_end_value;
        UINT32 u32_duty_cycle1, u32_duty_cycle2;
        UINT32 u32_dead_band_1;
        extern void bk_cw_pwm_init(uint8 channel_num, uint32 end_value, uint32 duty_cycle);
        extern void bk_cw_pwm_stop(uint8 channel_num_1, uint8 channel_num_2);
        extern void bk_cw_pwm_reset_duty_cycle(uint8 channel_num_1, uint8 channel_num_2,
                                           uint32 duty_cycle_1, uint32 duty_cycle_2,
                                           uint32 end_value, uint32 dead_band_1);
        os_printf("BEKEN_TUYA_PWM\r\n");
        switch (pHCIrxBuf->param[0])
            {
            case 0x01:
                u32_end_value   = 0 | ( pHCIrxBuf->param[2] & 0x000000FFUL)
                                    | ((pHCIrxBuf->param[3]<<8) & 0x0000FF00UL)
                                    | ((pHCIrxBuf->param[4]<<16) & 0x00FF0000UL)
                                    | ((pHCIrxBuf->param[5]<<24) & 0xFF000000UL);
                u32_duty_cycle1 = 0 | ( pHCIrxBuf->param[6] & 0x000000FFUL)
                                    | ((pHCIrxBuf->param[7]<<8) & 0x0000FF00UL)
                                    | ((pHCIrxBuf->param[8]<<16) & 0x00FF0000UL)
                                    | ((pHCIrxBuf->param[9]<<24) & 0xFF000000UL);
                os_printf("tuya_pwm_init\r\n");
                bk_cw_pwm_init(pHCIrxBuf->param[1], u32_end_value, u32_duty_cycle1);
                break;
            case 0x02:
                os_printf("tuya_pwm_stop\r\n");
                bk_cw_pwm_stop(pHCIrxBuf->param[1], pHCIrxBuf->param[2]);
                break;
            case 0x03:
                u32_duty_cycle1 = 0 | ( pHCIrxBuf->param[3] & 0x000000FFUL)
                                    | ((pHCIrxBuf->param[4]<<8) & 0x0000FF00UL)
                                    | ((pHCIrxBuf->param[5]<<16) & 0x00FF0000UL)
                                    | ((pHCIrxBuf->param[6]<<24) & 0xFF000000UL);
                u32_duty_cycle2 = 0 | ( pHCIrxBuf->param[7] & 0x000000FFUL)
                                    | ((pHCIrxBuf->param[8]<<8) & 0x0000FF00UL)
                                    | ((pHCIrxBuf->param[9]<<16) & 0x00FF0000UL)
                                    | ((pHCIrxBuf->param[10]<<24) & 0xFF000000UL);
                u32_end_value   = 0 | ( pHCIrxBuf->param[11] & 0x000000FFUL)
                                    | ((pHCIrxBuf->param[12]<<8) & 0x0000FF00UL)
                                    | ((pHCIrxBuf->param[13]<<16) & 0x00FF0000UL)
                                    | ((pHCIrxBuf->param[14]<<24) & 0xFF000000UL);
                u32_dead_band_1 = 0 | ( pHCIrxBuf->param[15] & 0x000000FFUL)
                                    | ((pHCIrxBuf->param[16]<<8) & 0x0000FF00UL)
                                    | ((pHCIrxBuf->param[17]<<16) & 0x00FF0000UL)
                                    | ((pHCIrxBuf->param[18]<<24) & 0xFF000000UL);
                os_printf("tuya_pwm_reset_duty_cycle\r\n");
                bk_cw_pwm_init(pHCIrxBuf->param[1], u32_end_value, u32_duty_cycle1);
                bk_cw_pwm_init(pHCIrxBuf->param[2], u32_end_value, u32_duty_cycle2);
                bk_cw_pwm_reset_duty_cycle(pHCIrxBuf->param[1], pHCIrxBuf->param[2],
                                          u32_duty_cycle1, u32_duty_cycle2,
                                          u32_end_value, u32_dead_band_1);
                break;

            default:
                break;
            }
        }
        break;
#endif

    case LOOP_MODE_CMD: 
		{
            #if CFG_USE_AUDIO
            extern void audio_intf_uninit(void);
			
            audio_intf_uninit();
            #endif
        }
        break;
        
    case BEKEN_DUMP_ENV_CMD:
        {
            #if CFG_USE_AUDIO
            extern UINT32 audio_intf_init(void);
            
            audio_intf_init();
            #endif
        }
        break;
        
#if CFG_USE_TEMPERATURE_DETECT
    case BEKEN_TEMP_DETECT_CONFIG_CMD:		// 01 E0 FC 04 ec 02 01 05
    {
        extern void temp_detect_change_configuration(UINT32 intval, UINT32 thre, UINT32 dist);
		
        int intval, thre, dist;
		
        intval = (int)pHCIrxBuf->param[0];
        thre   = (int)pHCIrxBuf->param[1];
        dist   = (int)pHCIrxBuf->param[2];
		
        temp_detect_change_configuration(intval, thre, dist);
    }
    break;
#endif

    case BEKEN_SHOW_BT_STATUS:
    {
#if CFG_USE_TUYA_CCA_TEST
        UINT32 val, reg;
        val = (UINT32)pHCIrxBuf->param[0];

        reg = REG_READ(AGC_RWNXAGCCCA1_ADDR_dbg);
        
        os_printf("0 set cca val:0x%08x, 0x%08x\r\n", val, reg);

        reg &= ~(AGC_CCARISETHRDBM_MASK_dbg << AGC_CCARISETHRDBM_LSB_dbg);
        reg |= ((val &AGC_CCARISETHRDBM_MASK_dbg)  << AGC_CCARISETHRDBM_LSB_dbg);
        REG_WRITE(AGC_RWNXAGCCCA1_ADDR_dbg, reg);

        reg = REG_READ(AGC_RWNXAGCCCA1_ADDR_dbg);
        os_printf("1 set cca val:0x%08x, 0x%08x\r\n", val, reg);  
#endif
    }
    break;
        
    case BEKEN_SHOW_BT_DEBUG:
    {
#if CFG_USE_TUYA_CCA_TEST
        UINT32 val;

        val = REG_READ(AGC_RWNXAGCCCA1_ADDR_dbg);

        os_printf("cca val:0x%08x\r\n", val);
#endif
    }
    break;

    case BEKEN_DO_REBOOT:
    {
        if((pHCIrxBuf->param[0] == 0x95)
            && (pHCIrxBuf->param[1] == 0x27)
            && (pHCIrxBuf->param[2] == 0x95)
            && (pHCIrxBuf->param[3] == 0x27))
        {
           /// reboot(NULL,NULL,1,NULL);
           extern void bk_reboot(void);
           bk_reboot(); 
        }
    }
    break;

    default:
        pHCItxBuf->total = 1;
        pHCItxBuf->param[0] = pHCIrxBuf->cmd;
        break;
    }

    bkreg_tx(pHCItxBuf);

    return 0;
}
#endif // CFG_SUPPORT_BKREG
#else

int cmd_usage(const cmd_tbl_t *cmdtp)
{
    return 0;
}

cmd_tbl_t *cmd_find_tbl(const char *cmd, cmd_tbl_t *table, int table_len)
{
    return 0;
}

int bkreg_run_command(const char *content, int cnt)
{
	return 0;
}
#endif // CFG_UART_DEBUG
// eof

