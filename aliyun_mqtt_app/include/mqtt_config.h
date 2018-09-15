#ifndef __MQTT_CONFIG_H__
#define __MQTT_CONFIG_H__

#define CFG_HOLDER	 	0x00FF55A3	/* Change this value to load default configurations */
#define CFG_LOCATION	0x79	/* Please don't change or if you know what you doing */
#define MQTT_SSL_ENABLE

/*DEFAULT CONFIGURATIONS*/


/* 以下配置转移到 aliyun_mqtt.h */
#if 0

#define MQTT_HOST			"192.168.10.168" //or "mqtt.yourdomain.com"
#define MQTT_PORT			61613

#define MQTT_CLIENT_ID		"DVES_%08X"
#define MQTT_USER			"admin"
#define MQTT_PASS			"password"

#endif

#define MQTT_BUF_SIZE		1024
#define MQTT_KEEPALIVE		120	 /*second*/

/* 以下配置转移到 user_config.h */
#if 0
#define STA_SSID "DVES_HOME"
#define STA_PASS "yourpassword"
#define STA_TYPE AUTH_WPA2_PSK
#endif

#define MQTT_RECONNECT_TIMEOUT  5	/*second*/

#define DEFAULT_SECURITY        0
#define QUEUE_BUFFER_SIZE       2048

//#define PROTOCOL_NAMEv31	/*MQTT version 3.1 compatible with Mosquitto v0.15*/
#define PROTOCOL_NAMEv311			/*MQTT version 3.11 compatible with https://eclipse.org/paho/clients/testing/*/

#endif // __MQTT_CONFIG_H__
