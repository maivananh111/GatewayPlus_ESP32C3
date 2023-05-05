/*
 * commandline.h
 *
 *  Created on: Apr 19, 2023
 *      Author: anh
 */

#ifndef COMPONENTS_COMMANDLINE_H_
#define COMPONENTS_COMMANDLINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"

#include "sdkconfig.h"

typedef enum {
	WIFI_ERR = 0x00,

	WIFI_RESTART,
	/**
	 * Network control command.
	 */
#if defined(CONFIG_CMD_SUPPORT_WIFI_CONTROL)
	WIFI_SCAN,
	WIFI_ISCONNECTED,
	WIFI_CONN,
	WIFI_DISCONN,
	WIFI_GETIP,
#endif /* defined(CONFIG_CMD_SUPPORT_WIFI_CONTROL) */

	/**
	 * HTTP client command.
	 */
#if defined(CONFIG_CMD_SUPPORT_HTTP_CLIENT)
	WIFI_HTTP_CLIENT_NEW,
	WIFI_HTTP_CLIENT_CONFIG,
	WIFI_HTTP_CLIENT_INIT,
	WIFI_HTTP_CLIENT_CLEAN,
	WIFI_HTTP_CLIENT_SET_HEADER,
	WIFI_HTTP_CLIENT_SET_URL,
	WIFI_HTTP_CLIENT_SET_METHOD,
	WIFI_HTTP_CLIENT_SET_DATA,
	WIFI_HTTP_CLIENT_REQUEST,
	WIFI_HTTP_CLIENT_RESPONSE,
#endif /* defined(CONFIG_CMD_SUPPORT_HTTP_CLIENT) */

	WIFI_CMD_NUM,
} packet_cmd_t;



void cmdcontrol_init(void (*presponse_function)(char *resp));
void cmdcontrol_process(void *param);


#ifdef __cplusplus
}
#endif

#endif /* COMPONENTS_COMMANDLINE_H_ */
