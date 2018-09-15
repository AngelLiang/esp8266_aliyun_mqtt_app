/*
 * user_wifi.h
 */

#ifndef _USER_WIFI_H_
#define _USER_WIFI_H_

#include "os_type.h"
#include "user_config.h"

typedef void (*WifiCallback)(uint8_t);

extern void wifi_check_init(u16);
extern void wifi_connect(WifiCallback cb);

#endif /* _USER_WIFI_H_ */
