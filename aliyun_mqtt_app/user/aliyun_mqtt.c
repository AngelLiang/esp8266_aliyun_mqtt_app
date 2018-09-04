/*
 * aliyun_mqtt.c
 */

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "mem.h"

#include "driver/uart.h"

//#include "user_config.h"
#include "aliyun_mqtt.h"

#include "md5.h"

/****************************************************************/
#define DEBUG	1

#ifdef DEBUG
#define debug(fmt, args...) os_printf(fmt, ##args)
#define debugX(level, fmt, args...) if(DEBUG>=level) os_printf(fmt, ##args);
#else
#define debug(fmt, args...)
#define debugX(level, fmt, args...)
#endif /* DEBUG */

/****************************************************************/
// global variable
s_mqtt g_aliyun_mqtt;

/****************************************************************/

/*
 * function: gen_mqtt_password
 * parameter: u8 output_pass[] - 生成的密码
 * return: u16 - output_pass[]字符串长度
 * description: 生成阿里云的mqtt密码
 *              因为密码需要根据DEVICE_ID、DEVICE_NAME、PRODUCT_KEY和DEVICE_SECRET
 *              动态生成
 */
static u16 ICACHE_FLASH_ATTR
gen_mqtt_password(u8 input_timestamp_3[], u8 output_pass[]) {

#define HASH_STR	"clientId"DEVICE_ID"deviceName"DEVICE_NAME"productKey"PRODUCT_KEY"timestamp%s"
#define HASH_KEY	DEVICE_SECRET

	int i;
	u8 temp[8];
	u8 output[16];
	u8 *p = output_pass;
	u8 *pStr = os_malloc(os_strlen(HASH_STR)+os_strlen(input_timestamp_3));

	os_sprintf(pStr, HASH_STR, input_timestamp_3);
	debug("pStr:%s\r\n", pStr);

	HMAC_MD5(pStr, os_strlen(pStr), HASH_KEY, output);

	for (i = 0; i < 16; i++) {
		if (output[i] < 0x10) {
			os_memset(temp, 0, sizeof(temp));
			os_sprintf(temp, "%d", 0);
			os_strcat(p, temp);
		}
		os_memset(temp, 0, sizeof(temp));
		os_sprintf(temp, "%x", output[i]);
		os_strcat(p, temp);
	}
	debug("pass:%s\r\n", output_pass);

	os_free(pStr);

	return os_strlen(output_pass);
}

/*
 * function: aliyun_mqtt_init
 *
 */
void ICACHE_FLASH_ATTR
aliyun_mqtt_init(void) {

/* TODO:
 * 三种生成timestamp的方式，目前默认是写死在代码里。
 * - SNTP_TIMESTAMP：使用sntp生成，暂时未实现
 * - RAMDOM_TIMESTAMP：使用随机函数生成
 * - defalut：使用宏定义方式
 */
#ifdef SNTP_TIMESTAMP
	u8 *timestamp_str = TIMESTAMP_STR;
#elif defined(RAMDOM_TIMESTAMP)
	u8 timestamp_str[4] = { 0 };
	u32 number = os_random();
	os_sprintf(timestamp_str, "%d", number % 1000);
	timestamp_str[3] = '\0';
#else
	u8 *timestamp_str = TIMESTAMP_STR;
#endif

	debug("timestamp_str:%s\r\n", timestamp_str);

	// mqtt host
	os_strncpy(g_aliyun_mqtt.host, MQTT_HOST, BUF_SIZE);
	// mqtt port
	g_aliyun_mqtt.port = MQTT_PORT;
	// mqtt client id，拼接timestamp
	os_sprintf(g_aliyun_mqtt.client_id, MQTT_CLIENT_ID, timestamp_str);
	// mqtt username
	os_strncpy(g_aliyun_mqtt.username, MQTT_USERNAME, BUF_SIZE);
	// mqtt password
	gen_mqtt_password(timestamp_str, g_aliyun_mqtt.password);
	// mqtt keepalive
	g_aliyun_mqtt.keepalive = 120;

	debug("mqtt host:%s\r\n"
			"mqtt port:%d\r\n"
			"mqtt client id:%s\r\n"
			"mqtt username:%s\r\n"
			"mqtt password:%s\r\n", g_aliyun_mqtt.host, g_aliyun_mqtt.port,
			g_aliyun_mqtt.client_id, g_aliyun_mqtt.username,
			g_aliyun_mqtt.password);

}

/*
 * function: test_hmac_md5
 * description: hmacmd5测试，成功
 */
void ICACHE_FLASH_ATTR
test_hmac_md5(void) {

#define TEST_KEY	"secret"
#define TEST_MSG	"clientId12345deviceNamedeviceproductKeypktimestamp789"

	int i;
	u8 output[16];
	HMAC_MD5(TEST_MSG, os_strlen(TEST_MSG), TEST_KEY, output);
	/* It should be output: 14B198324FE55E1D3C88F2E705E201EE */
	/* 对比 */
	/* Digest: 14b198324fe55e1d3c88f2e705e201ee */

	debug("Digest: ");
	for (i = 0; i < 16; i++) {
		if (output[i] < 0x10) {
			debug("%d", 0);
		}
		debug("%x", output[i]);
	}
	debug("\n");

}

