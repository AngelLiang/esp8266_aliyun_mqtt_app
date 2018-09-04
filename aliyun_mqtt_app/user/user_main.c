/* main.c -- MQTT client example
 *
 * Copyright (c) 2014-2015, Tuan PM <tuanpm at live dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * * Neither the name of Redis nor the names of its contributors may be used
 * to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "user_interface.h"
#include "mem.h"

#include "driver/uart.h"

#include "mqtt/mqtt.h"
#include "mqtt/debug.h"

#include "user_config.h"
#include "user_wifi.h"
#include "aliyun_mqtt.h"

MQTT_Client mqttClient;

void wifiConnectCb(uint8_t status) {
	if (status == STATION_GOT_IP) {
		MQTT_Connect(&mqttClient);
	} else {
		MQTT_Disconnect(&mqttClient);
	}
}

void mqttConnectedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*) args;
	INFO("MQTT: Connected\r\n");

	MQTT_Subscribe(client, "/"PRODUCT_KEY"/"DEVICE_NAME"/get", 0);

	/*
	 * MQTT_Publish函数参数说明
	 * @param  client: 	    MQTT_Client reference
	 * @param  topic: 		string topic will publish to
	 * @param  data: 		buffer data send point to
	 * @param  data_length: length of data
	 * @param  qos:		    qos
	 * @param  retain:      retain
	 */
	MQTT_Publish(client, "/"PRODUCT_KEY"/"DEVICE_NAME"/update", "hello", 6, 0, 0);
}

void mqttDisconnectedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*) args;
	INFO("MQTT: Disconnected\r\n");
}

void mqttPublishedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*) args;
	INFO("MQTT: Published\r\n");
}

void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len,
		const char *data, uint32_t data_len) {
	char *topicBuf = (char*) os_zalloc(topic_len + 1), *dataBuf =
			(char*) os_zalloc(data_len + 1);

	MQTT_Client* client = (MQTT_Client*) args;

	os_memcpy(topicBuf, topic, topic_len);
	topicBuf[topic_len] = 0;

	os_memcpy(dataBuf, data, data_len);
	dataBuf[data_len] = 0;

	INFO("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);
	os_free(topicBuf);
	os_free(dataBuf);
}

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
 *******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void) {
	enum flash_size_map size_map = system_get_flash_size_map();
	uint32 rf_cal_sec = 0;

	switch (size_map) {
	case FLASH_SIZE_4M_MAP_256_256:
		rf_cal_sec = 128 - 5;
		break;

	case FLASH_SIZE_8M_MAP_512_512:
		rf_cal_sec = 256 - 5;
		break;

	case FLASH_SIZE_16M_MAP_512_512:
	case FLASH_SIZE_16M_MAP_1024_1024:
		rf_cal_sec = 512 - 5;
		break;

	case FLASH_SIZE_32M_MAP_512_512:
	case FLASH_SIZE_32M_MAP_1024_1024:
		rf_cal_sec = 1024 - 5;
		break;

	default:
		rf_cal_sec = 0;
		break;
	}

	return rf_cal_sec;
}

/*****************************************************************************/

/*
 * function: user_sntp_init
 * description: sntp初始化，暂时用不上
 */
void ICACHE_FLASH_ATTR
user_sntp_init(void) {
	// set sntp server
	sntp_setservername(0, "0.cn.pool.ntp.org");
	sntp_setservername(1, "1.cn.pool.ntp.org");
	sntp_setservername(2, "2.cn.pool.ntp.org");
	sntp_init();
}

void ICACHE_FLASH_ATTR
init_done_cb_init(void) {

#ifdef SMARTCONFIG_ENABLE

	/*
	 * smartconfig_connect 只能在 init_done_cb_init 调用才正常
	 * 先进行smartconfig，没有配网信息则自动连接上次的wifi
	 */
	smartconfig_connect(wifiConnectCb);

#else	/* OR */

	// 直接使用 WIFI_SSID 和 WIFI_PASS 宏定义连接wifi
	wifi_connect(wifiConnectCb);

#endif /* SMARTCONFIG_ENABLE */
}

void user_init(void) {
	//uart_init(BIT_RATE_115200, BIT_RATE_115200);

	// 暂时没用到sntp，可删掉
	user_sntp_init();

	/* 测试 hmacmd5 生成mqtt passwrod */
	//test_hmac_md5();

	aliyun_mqtt_init();

	//MQTT_InitConnection(&mqttClient, "192.168.11.122", 1880, 0);
	MQTT_InitConnection(&mqttClient, g_aliyun_mqtt.host, g_aliyun_mqtt.port, 0);

	//MQTT_InitClient(&mqttClient, "client_id", "user", "pass", 120, 1);
	MQTT_InitClient(&mqttClient, g_aliyun_mqtt.client_id,
			g_aliyun_mqtt.username, g_aliyun_mqtt.password,
			g_aliyun_mqtt.keepalive, 1);

	// 遗愿消息
	// 阿里云mqtt不需要设置遗愿消息
	//MQTT_InitLWT(&mqttClient, "/lwt", "offline", 0, 0);

	// 设置mqtt的回调函数
	MQTT_OnConnected(&mqttClient, mqttConnectedCb);
	MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
	MQTT_OnPublished(&mqttClient, mqttPublishedCb);
	MQTT_OnData(&mqttClient, mqttDataCb);

	//wifi_connect(wifiConnectCb);
	system_init_done_cb(init_done_cb_init);

	INFO("\r\nSystem started ...\r\n");
}
