#ifndef __MQTT_H__
#define __MQTT_H__
/**
 * MQTT URI farmat:
 * domain mode
 * tcp://iot.eclipse.org:1883
 * tcp://broker.mqttdashboard.com:1883
 * ipv4 mode
 * tcp://192.168.10.1:1883
 * ssl://192.168.10.1:1884
 *
 * ipv6 mode
 * tcp://[fe80::20c:29ff:fe9a:a07e]:1883
 * ssl://[fe80::20c:29ff:fe9a:a07e]:1884
 */ 
//#define MQTT_TEST_SERVER_URI    "tcp://broker.emqx.io:1883"
//#define MQTT_TEST_SERVER_URI    "tcp://mqtt.cloud.yandex.net:8883"
#define MQTT_TEST_SERVER_URI      "tcp://o8b215e4.us-east-1.emqx.cloud:15292"
#define MQTT_CLIENTID           "beken-mqtt"
#define MQTT_USERNAME           "test"
#define MQTT_PASSWORD           "test"
#define MQTT_SUBTOPIC           "/mqtt/test"
#define MQTT_PUBTOPIC           "/mqtt/test"
#define MQTT_WILLMSG            "Goodbye!"
#define MQTT_TEST_QOS           1
#define MQTT_PUB_SUB_BUF_SIZE   1024

#define TEST_DATA_SIZE          256
#define PUB_CYCLE_TM            10000

#endif /*__MQTT_H__*/
