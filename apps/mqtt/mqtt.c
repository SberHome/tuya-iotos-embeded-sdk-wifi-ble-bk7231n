#include "rtos_pub.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "paho_mqtt.h"
#include "mqtt.h"
#include "rw_pub.h"
#include "str_pub.h"
#include "wlan_ui_pub.h"
#include "ieee802_11_defs.h"


#ifndef APP_DEBUG
#define APP_DEBUG 0
#endif

#define app_print(...)  do { if (APP_DEBUG) os_printf("[APP]"__VA_ARGS__); } while (0);


static char *test_pub_data = NULL;
static MQTT_CLIENT_T mqtt_client;
static uint32_t pub_count = 0;
static uint32_t sub_count = 0;
static int recon_count = -1;
static int test_start_tm = 0;
static uint32_t g_mqtt_wifi_flag = 0;

void mqtt_wifi_connect_cb(void)
{
    app_print("mqtt_wifi_connect_cb\r\n");
    g_mqtt_wifi_flag = 1;
}

void mqtt_wifi_disconnect_cb(rw_evt_type evt_type, void *data)
{
    g_mqtt_wifi_flag = 0;
}

uint32_t mqtt_is_wifi_connected(void)
{
    return (1 == g_mqtt_wifi_flag);
}

void mqtt_waiting_for_wifi_connected(void)
{
    while (0 == mqtt_is_wifi_connected())
    {
        rtos_delay_milliseconds(5000);
    }
}

static void mqtt_sub_default_callback(MQTT_CLIENT_T *c, MessageData *msg_data)
{
    app_print("mqtt_sub_default_callback\r\n");
}

static void mqtt_sub_callback(MQTT_CLIENT_T *c, MessageData *msg_data)
{
    app_print("mqtt_sub_callback\r\n");
    sub_count++;
}

static void mqtt_connect_callback(MQTT_CLIENT_T *c)
{
    app_print("mqtt_connect_callback\r\n");
}

static void mqtt_online_callback(MQTT_CLIENT_T *c)
{
    app_print("mqtt_online_callback\r\n");
    recon_count++;
}

static void mqtt_offline_callback(MQTT_CLIENT_T *c)
{
    app_print("mqtt_offline_callback\r\n");
}
/**
 * This function publish message to specific mqtt topic.
 *
 * @param send_str publish message
 *
 * @return none
 */
static int mqtt_test_publish(const char *send_str)
{
    static int counter = 0;
    MQTTMessage message;
    const char *msg_str = send_str;
    const char *topic = MQTT_PUBTOPIC;

    message.qos = MQTT_TEST_QOS;
    message.retained = 0;
    char msg[20];
    snprintf(msg, 20, "Hello: %d", counter++);
    message.payload = msg;
    message.payloadlen = os_strlen(message.payload);

    return mqtt_publish_with_topic(&mqtt_client, topic, &message);
}

/**
 * This function create and config a mqtt client.
 *
 * @param void
 *
 * @return none
 */
static void mqtt_start(void)
{
    app_print("mqtt_start\r\n");

    /* init condata param by using MQTTPacket_connectData_initializer */
    MQTTPacket_connectData condata = MQTTPacket_connectData_initializer;

    os_memset(&mqtt_client, 0, sizeof(MQTT_CLIENT_T));

    /* config MQTT context param */
    mqtt_client.uri = MQTT_TEST_SERVER_URI;

    /* config connect param */
    memcpy(&mqtt_client.condata, &condata, sizeof(condata));
    mqtt_client.condata.clientID.cstring = MQTT_CLIENTID;
    mqtt_client.condata.keepAliveInterval = 60;
    mqtt_client.condata.cleansession = 1;
    mqtt_client.condata.username.cstring = MQTT_USERNAME;
    mqtt_client.condata.password.cstring = MQTT_PASSWORD;

    /* config MQTT will param. */
    mqtt_client.condata.willFlag = 1;
    mqtt_client.condata.will.qos = MQTT_TEST_QOS;
    mqtt_client.condata.will.retained = 0;
    mqtt_client.condata.will.topicName.cstring = MQTT_PUBTOPIC;
    mqtt_client.condata.will.message.cstring = MQTT_WILLMSG;

    /* malloc buffer. */
    mqtt_client.buf_size = mqtt_client.readbuf_size = MQTT_PUB_SUB_BUF_SIZE;
    mqtt_client.buf = os_malloc(mqtt_client.buf_size);
    mqtt_client.readbuf = os_malloc(mqtt_client.readbuf_size);
    if (!(mqtt_client.buf && mqtt_client.readbuf))
    {
        app_print("no memory for MQTT mqtt_client buffer!\n");
        goto _exit;
    }

    /* set event callback function */
    mqtt_client.connect_callback = mqtt_connect_callback;
    mqtt_client.online_callback = mqtt_online_callback;
    mqtt_client.offline_callback = mqtt_offline_callback;

    /* set subscribe table and event callback */
    mqtt_client.messageHandlers[0].topicFilter = os_strdup(MQTT_SUBTOPIC);
    mqtt_client.messageHandlers[0].callback = mqtt_sub_callback;
    mqtt_client.messageHandlers[0].qos = MQTT_TEST_QOS;

    /* set default subscribe event callback */
    mqtt_client.defaultMessageHandler = mqtt_sub_default_callback;

    /* run mqtt client */
    app_print("paho_mqtt_start\r\n");
    paho_mqtt_start(&mqtt_client);

    return;

_exit:
    if (mqtt_client.buf)
    {
        os_free(mqtt_client.buf);
        mqtt_client.buf = NULL;
    }

    if (mqtt_client.readbuf)
    {
        os_free(mqtt_client.readbuf);
        mqtt_client.readbuf = NULL;
    }

    return;
}

static void test_show_info(void)
{
    app_print("==== MQTT Stability test ====\n");
    app_print("Server: " MQTT_TEST_SERVER_URI "\n");
    app_print("QoS   : %d\n", MQTT_TEST_QOS);

    app_print("Test duration(tick)           : %d\n", rtos_get_time() - test_start_tm);
    app_print("Number of published  packages : %d\n", pub_count);
    app_print("Number of subscribed packages : %d\n", sub_count);
    app_print("Number of reconnections       : %d\n", recon_count);
}

static void mqtt_pub_handler(void *parameter)
{

    test_pub_data = os_malloc(TEST_DATA_SIZE * sizeof(char));
    if (!test_pub_data)
    {
        app_print("no memory for test_pub_data\n");
        return;
    }
    os_memset(test_pub_data, '*', TEST_DATA_SIZE * sizeof(char));

    test_start_tm = rtos_get_time();
    app_print("test start at '%d'\r\n", test_start_tm);

    while (1)
    {
        if (!mqtt_test_publish(test_pub_data))
        {
            ++pub_count;
        }

        rtos_delay_milliseconds(PUB_CYCLE_TM);

        test_show_info();
    }
}

OSStatus wifi_station_init(char *oob_ssid, char *connect_key)
{
    OSStatus ret = kNoErr;
    network_InitTypeDef_st wNetConfig = {0};

    int len = os_strlen(oob_ssid);
    if (SSID_MAX_LEN < len)
    {
        bk_printf("ssid name more than 32 Bytes\r\n");
        return kParamErr;
    }

    os_strcpy((char *)wNetConfig.wifi_ssid, oob_ssid);
    os_strcpy((char *)wNetConfig.wifi_key, connect_key);

    wNetConfig.wifi_mode = STATION;
    wNetConfig.dhcp_mode = DHCP_CLIENT;
    wNetConfig.wifi_retry_interval = 100;

    app_print("ssid:%s key:%s\r\n", wNetConfig.wifi_ssid, wNetConfig.wifi_key);
    ret = bk_wlan_start(&wNetConfig);

    if (ret != kNoErr)
        app_print("bk_wlan_start failed: %d\r\n", ret);

    return ret;
}

OSStatus user_main(void)
{
    OSStatus ret = kNoErr;
    extended_app_waiting_for_launch();  // need to wait for rl_init() to finish
    net_set_sta_ipup_callback(mqtt_wifi_connect_cb);
    //user_connected_callback(mqtt_wifi_connect_cb);

    wifi_station_init("HONOR_KIW-L21_EEE9", "1234567890");

    mqtt_waiting_for_wifi_connected();
    mqtt_start();

    while (!mqtt_client.is_connected)
    {
        app_print("Waiting for mqtt connection...\r\n");
        rtos_delay_milliseconds(1000);
    }

    ret = rtos_create_thread(NULL,
                             8,
                             "pub_thread",
                             mqtt_pub_handler,
                             1024 * 4,
                             NULL);
    return ret;
}