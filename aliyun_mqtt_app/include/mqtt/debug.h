/*
 * debug.h
 *
 *  Created on: Dec 4, 2014
 *      Author: Minh
 */

#ifndef USER_DEBUG_H_
#define USER_DEBUG_H_

// 如果不需要mqtt打印信息，则注释下面这条define
#define MQTT_DEBUG_ON

#if defined(MQTT_DEBUG_ON)
#define INFO( format, ... ) os_printf( format, ## __VA_ARGS__ )
#else
#define INFO( format, ... )
#endif

// #ifndef INFO
// #define INFO os_printf
// #endif

#endif /* USER_DEBUG_H_ */
