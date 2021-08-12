/*
*      Пример поиска WiFi сетей
*/

#include "wifi_scan.h"
#include "wlan_ui_pub.h"

static beken_semaphore_t scan_handle = NULL;

// Колбэк
// Попадаем сюда по окончании сканирования
void scan_ap_cb(void *ctxt, uint8_t param)
{
    if (scan_handle != NULL)
        rtos_set_semaphore(&scan_handle);
}

void show_scan_ap_result(void)
{
    struct scanu_rst_upload *scan_rst = sr_get_scan_results();
    if (scan_rst == NULL)
    {
        os_printf("NO AP FOUND\r\n");
        return;
    }

    os_printf("Scan ap count:%d\r\n", scan_rst->scanu_num);

    for (int i = 0; i < scan_rst->scanu_num; i++)
    {
        struct sta_scan_res *scan_rst_table = scan_rst->res[i];
        os_printf("%d: %s, ", i + 1, scan_rst_table->ssid);
        os_printf("Channel:%d, ", scan_rst_table->channel);
        switch (scan_rst_table->security)
        {
        case SECURITY_TYPE_NONE:
            os_printf(" %s, ", "Open");
            break;
        case SECURITY_TYPE_WEP:
            os_printf(" %s, ", "CIPHER_WEP");
            break;
        case SECURITY_TYPE_WPA_TKIP:
            os_printf(" %s, ", "CIPHER_WPA_TKIP");
            break;
        case SECURITY_TYPE_WPA_AES:
            os_printf(" %s, ", "CIPHER_WPA_AES");
            break;
        case SECURITY_TYPE_WPA2_TKIP:
            os_printf(" %s, ", "CIPHER_WPA2_TKIP");
            break;
        case SECURITY_TYPE_WPA2_AES:
            os_printf(" %s, ", "CIPHER_WPA2_AES");
            break;
        case SECURITY_TYPE_WPA2_MIXED:
            os_printf(" %s, ", "CIPHER_WPA2_MIXED");
            break;
        case SECURITY_TYPE_AUTO:
            os_printf(" %s, ", "CIPHER_AUTO");
            break;
        default:
            os_printf(" %s(%d), ", "security type unknown", scan_rst_table->security);
            break;
        }
        os_printf("RSSI=%d \r\n", scan_rst_table->level);
    }

    // IMPORTANT to release results
    sr_release_scan_results(scan_rst);
}

void wifi_scan_thread(beken_thread_arg_t arg)
{
    (void)arg;
    OSStatus err = kNoErr;

    while (true)
    {
        err = rtos_init_semaphore(&scan_handle, 1);
        if (err == kNoErr)
        {
            bk_wlan_scan_ap_reg_cb(scan_ap_cb);
            bk_wlan_start_scan();

            err = rtos_get_semaphore(&scan_handle, BEKEN_WAIT_FOREVER);
            if (err == kNoErr)
            {
                show_scan_ap_result();
            }

            if (scan_handle)
            {
                rtos_deinit_semaphore(&scan_handle);
            }
        }
        else
        {
            os_printf("scan_handle init failed!\r\n");
        }
    }
    rtos_delete_thread(NULL);
}

OSStatus user_main(void)
{
    OSStatus err = kNoErr;

    os_printf("\r\n\r\nwifi scan demo............\r\n\r\n");
    err = rtos_create_thread(NULL, BEKEN_APPLICATION_PRIORITY,
                             "wifiscan",
                             (beken_thread_function_t)wifi_scan_thread,
                             0x800,
                             NULL);

    return err;
}
