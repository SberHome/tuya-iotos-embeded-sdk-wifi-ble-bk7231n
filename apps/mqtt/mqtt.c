#include "rtos_pub.h"
#include "paho_mqtt.h"
#include "mqtt.h"
#include "wlan_ui_pub.h"
#include "net.h"

#ifndef APP_DEBUG
#define APP_DEBUG 0
#endif
#define debug_print(...)                   \
    do                                     \
    {                                      \
        if (APP_DEBUG)                     \
            os_printf("[APP]"__VA_ARGS__); \
    } while (0);


static uint32_t pub_count = 0;
static uint32_t sub_count = 0;
static int recon_count = -1;
static int test_start_tm = 0;
static uint32_t g_mqtt_wifi_flag = 0;

static uint8_t mqtt_buf[MQTT_PUB_SUB_BUF_SIZE];
static uint8_t mqtt_read_buf[MQTT_PUB_SUB_BUF_SIZE];

// Forward declarations
static void mqtt_connect_callback(MQTT_CLIENT_T *c);
static void mqtt_online_callback(MQTT_CLIENT_T *c);
static void mqtt_offline_callback(MQTT_CLIENT_T *c);
static void mqtt_sub_callback(MQTT_CLIENT_T *c, MessageData *msg_data);
static void mqtt_sub_default_callback(MQTT_CLIENT_T *c, MessageData *msg_data);

static network_InitTypeDef_st wNetConfig = {
    .wifi_ssid = WIFI_SSID,
    .wifi_key = WIFI_PASSWORD,
    .wifi_mode = STATION,
    .dhcp_mode = DHCP_CLIENT,
    .wifi_retry_interval = 100};

static MQTT_CLIENT_T mqtt_client = {
    .uri = MQTT_TEST_SERVER_URI,
    .condata = MQTTPacket_connectData_initializer,
    .condata.clientID.cstring = MQTT_CLIENTID,
    .condata.keepAliveInterval = 60,
    .condata.cleansession = 1,
    .condata.username.cstring = MQTT_USERNAME,
    .condata.password.cstring = MQTT_PASSWORD,

    // config MQTT will param.
    .condata.willFlag = 1,
    .condata.will.qos = MQTT_TEST_QOS,
    .condata.will.retained = 0,
    .condata.will.topicName.cstring = MQTT_PUBTOPIC,
    .condata.will.message.cstring = MQTT_WILLMSG,

    .buf_size = MQTT_PUB_SUB_BUF_SIZE,
    .readbuf_size = MQTT_PUB_SUB_BUF_SIZE,
    .buf = mqtt_buf,
    .readbuf = mqtt_read_buf,

    // set event callback function
    .connect_callback = mqtt_connect_callback,
    .online_callback = mqtt_online_callback,
    .offline_callback = mqtt_offline_callback,

    // set subscribe table and event callback
    .messageHandlers[0].topicFilter = MQTT_SUBTOPIC,
    .messageHandlers[0].callback = mqtt_sub_callback,
    .messageHandlers[0].qos = MQTT_TEST_QOS,

    // set default subscribe event callback
    .defaultMessageHandler = mqtt_sub_default_callback};

void mqtt_wifi_connect_cb(void)
{
    debug_print("mqtt_wifi_connect_cb\n");
    g_mqtt_wifi_flag = 1;
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
    debug_print("mqtt_sub_default_callback\n");
}

static void mqtt_sub_callback(MQTT_CLIENT_T *c, MessageData *msg_data)
{
    debug_print("mqtt_sub_callback\n");
    sub_count++;
}

static void mqtt_connect_callback(MQTT_CLIENT_T *c)
{
    debug_print("mqtt_connect_callback\n");
}

static void mqtt_online_callback(MQTT_CLIENT_T *c)
{
    debug_print("mqtt_online_callback\n");
    recon_count++;
}

static void mqtt_offline_callback(MQTT_CLIENT_T *c)
{
    debug_print("mqtt_offline_callback\n");
}

static int mqtt_test_publish()
{
    static int counter = 0;
    char msg[20];
    int written = snprintf(msg, sizeof msg, "Hello: %d", counter++);

    MQTTMessage message = {
        .qos = MQTT_TEST_QOS,
        .retained = 0,
        .payload = msg,
        .payloadlen = (written < sizeof msg) ? written : 0};

    return mqtt_publish_with_topic(&mqtt_client, MQTT_PUBTOPIC, &message);
}

static void test_show_info(void)
{
    debug_print("==== MQTT Stability test ====\n");
    debug_print("Server: " MQTT_TEST_SERVER_URI "\n");
    debug_print("QoS   : %d\n", MQTT_TEST_QOS);

    debug_print("Test duration(tick)           : %d\n", rtos_get_time() - test_start_tm);
    debug_print("Number of published  packages : %d\n", pub_count);
    debug_print("Number of subscribed packages : %d\n", sub_count);
    debug_print("Number of reconnections       : %d\n", recon_count);
}

static void mqtt_pub_handler(void *parameter)
{
    test_start_tm = rtos_get_time();
    debug_print("test start at %d\n", test_start_tm);

    while (true)
    {
        if (!mqtt_test_publish())
        {
            ++pub_count;
        }

        rtos_delay_milliseconds(PUB_CYCLE_TM);
        test_show_info();
    }
}



OSStatus wifi_station_init()
{
    OSStatus ret = kNoErr;

    debug_print("ssid:%s key:%s\n", wNetConfig.wifi_ssid, wNetConfig.wifi_key);
    ret = bk_wlan_start(&wNetConfig);

    if (ret != kNoErr)
        debug_print("bk_wlan_start failed: %d\n", ret);

    return ret;
}

OSStatus user_main(void)
{
    OSStatus ret = kNoErr;
    extended_app_waiting_for_launch(); // need to wait for rl_init() to finish
    net_set_sta_ipup_callback(mqtt_wifi_connect_cb);
    //user_connected_callback(mqtt_wifi_connect_cb);

    wifi_station_init();

    //mqtt_waiting_for_wifi_connected();
    paho_mqtt_start(&mqtt_client);

    while (!mqtt_client.is_connected)
    {
        debug_print("Waiting for mqtt connection...\n");
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