#ifndef __PAHO_MQTT_UDP_H__
#define __PAHO_MQTT_UDP_H__

#ifndef MQTT_DEBUG
#define MQTT_DEBUG 0
#endif

#if MQTT_DEBUG
#define debug_printf           os_printf("[MQTT] ");os_printf
#else
#define debug_printf
#endif


#endif // __PAHO_MQTT_UDP_H__
// eof
