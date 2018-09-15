/*
 * user_wifi.c
 */

#include "user_interface.h"
#include "osapi.h"
#include "espconn.h"
#include "os_type.h"
#include "mem.h"

#include "user_wifi.h"

//*****************************************************************************/

// 如果不需要debug，则注释下面的语句
#define DEBUG	1

// 如果连接成功，每隔2s检查一次wifi状态
#define WIFI_CHECK_TIMER_INTERVAL	(2*1000)
// 如果连接失败，每500ms检测一次
#define WIFI_CLOSE_CHECK_TIMER_INTERVAL		(500)

//*****************************************************************************/
// debug
#define PR	os_printf

#ifdef DEBUG
#define debug(fmt, args...) PR(fmt, ##args)
#define debugX(level, fmt, args...) if(DEBUG>=level) PR(fmt, ##args);
#else
#define debug(fmt, args...)
#define debugX(level, fmt, args...)
#endif	/* DEBUG */

//*****************************************************************************/
// gloabl variable
static os_timer_t g_wifi_check_timer;



// 用户wifi回调函数
WifiCallback wifiCb = NULL;
// 记录wifi状态
static u8 wifiStatus = STATION_IDLE, lastWifiStatus = STATION_IDLE;

//*****************************************************************************/
/*
 * function: wifi_handle_event_cb
 * description: wifi事件处理回调函数，由esp8266自动触发
 */
static void ICACHE_FLASH_ATTR
wifi_handle_event_cb(System_Event_t *evt) {
	// 下面是官方示例代码
	// 但是，我们不需要这些代码来检查wifi状态
#if 0
	debug("event %x\n", evt->event);
	switch (evt->event) {
		case EVENT_STAMODE_CONNECTED:
		debug("connect to ssid %s, channel %d\n",
				evt->event_info.connected.ssid,
				evt->event_info.connected.channel);
		break;
		case EVENT_STAMODE_DISCONNECTED:
		debug("disconnect from ssid %s, reason %d\n",
				evt->event_info.disconnected.ssid,
				evt->event_info.disconnected.reason);
		break;
		case EVENT_STAMODE_AUTHMODE_CHANGE:
		debug("mode: %d -> %d\n", evt->event_info.auth_change.old_mode,
				evt->event_info.auth_change.new_mode);
		break;
		case EVENT_STAMODE_GOT_IP:
		debug("ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR,
				IP2STR(&evt->event_info.got_ip.ip),
				IP2STR(&evt->event_info.got_ip.mask),
				IP2STR(&evt->event_info.got_ip.gw));
		debug("\n");
		break;
		case EVENT_SOFTAPMODE_STACONNECTED:
		debug("station:	" MACSTR "join,	AID	=	%d\n",
				MAC2STR(evt->event_info.sta_connected.mac),
				evt->event_info.sta_connected.aid);
		break;
		case EVENT_SOFTAPMODE_STADISCONNECTED:
		debug("station:	" MACSTR "leave, AID	=	%d\n",
				MAC2STR(evt->event_info.sta_disconnected.mac),
				evt->event_info.sta_disconnected.aid);
		break;
		default:
		break;
	}
#endif
}

//*****************************************************************************/
// 有关wifi定时检查的函数

/*
 * function: wifi_check_timer_cb
 * description: wiif检查回调函数
 *              wiif连接成功时每 2s 检查一次
 *              wiif断开时每 500ms 检查一次
 */
static void ICACHE_FLASH_ATTR
wifi_check_timer_cb(void) {
	struct ip_info ipConfig;

	wifi_get_ip_info(STATION_IF, &ipConfig);
	wifiStatus = wifi_station_get_connect_status();

	if (wifiStatus == STATION_GOT_IP && ipConfig.ip.addr != 0) {
		// 重新初始化 wifi 检查定时器
		wifi_check_init(WIFI_CHECK_TIMER_INTERVAL);

	} else {	// wifi断开
		debug("wifi connect fail!\r\n");
		// 重新初始化 wifi 检查定时器
		wifi_check_init(WIFI_CLOSE_CHECK_TIMER_INTERVAL);
		//wifi_station_disconnect();
		//wifi_station_connect();		// 尝试重连
	}

	// user callback
	if (wifiStatus != lastWifiStatus) {
		lastWifiStatus = wifiStatus;
		if (wifiCb) {
			wifiCb(wifiStatus);
		}
	}
}

/*
 * function: wifi_check_init
 * parameter: u16 interval - 定时回调时间
 * description: wifi检查初始化
 */
void ICACHE_FLASH_ATTR
wifi_check_init(u16 interval) {
	/*
	 * 如果调用 wifi_station_set_reconnect_policy 关闭重连功能，
	 * 且未调用 wifi_set_event_handler_cb 注册 Wi-Fi 事件回调，
	 * 则 wifi_station_get_connect_status 接口失效，无法准确获得连接状态。
	 */

	// 设置 ESP8266 Station 连接 AP 失败或断开后是否重连。
	wifi_station_set_reconnect_policy(TRUE);
	wifi_set_event_handler_cb(wifi_handle_event_cb);

	os_timer_disarm(&g_wifi_check_timer);
	os_timer_setfn(&g_wifi_check_timer, (os_timer_func_t *) wifi_check_timer_cb, NULL);
	os_timer_arm(&g_wifi_check_timer, interval, 0);
}

//*****************************************************************************/

/*
 * function: user_set_station_config
 * parameter: u8* ssid - WiFi SSID
 *            u8 password - WiFi password
 * return: void
 * description: 设置wifi信息
 */
void ICACHE_FLASH_ATTR
user_set_station_config(u8* ssid, u8* password) {
	struct station_config stationConf;
	stationConf.bssid_set = 0;		//need not check MAC address of AP
	os_memcpy(&stationConf.ssid, ssid, 32);
	os_memcpy(&stationConf.password, password, 64);
	wifi_station_set_config(&stationConf);
}


//*****************************************************************************/
// 下面是主要对外的接口函数

/*
 * function: wifi_connect
 * parameter: WifiCallback cb - wifi回调函数
 * return: void
 * description: 连接wifi，需要传递一个回调函数
 */
void ICACHE_FLASH_ATTR
wifi_connect(WifiCallback cb) {
	wifi_set_opmode(STATION_MODE);		// set wifi mode
	wifiCb = cb;						// wifi callback

	// 上电是否自动连接
	// TRUE - 执行完 user_init 函数后则自动连接
	wifi_station_set_auto_connect(TRUE);

	user_set_station_config(WIFI_SSID, WIFI_PASS);
	wifi_station_disconnect();
	wifi_station_connect();

	// wifi timer check
	wifi_check_init(WIFI_CHECK_TIMER_INTERVAL);

}

