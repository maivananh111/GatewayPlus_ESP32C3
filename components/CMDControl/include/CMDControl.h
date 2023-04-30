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
	WIFI_CMD_ERR = 0x00,

	WIFI_CMD_RESTART,
	/**
	 * Network control command.
	 */
#if defined(CONFIG_CMD_SUPPORT_WIFI_CONTROL)
	WIFI_CMD_WIFI_SCAN,
	WIFI_CMD_WIFI_ISCONNECTED,
	WIFI_CMD_WIFI_CONN,
	WIFI_CMD_WIFI_DISCONN,
	WIFI_CMD_WIFI_GETIP,
#endif /* defined(CONFIG_CLI_SUPPORT_WIFI_CONTROL) */

	/**
	 * HTTP client command.
	 */
#if defined(CONFIG_CMD_SUPPORT_HTTP_CLIENT)
	WIFI_CMD_HTTP_CLIENT_NEW,
	WIFI_CMD_HTTP_CLIENT_CONFIG,
	WIFI_CMD_HTTP_CLIENT_INIT,
	WIFI_CMD_HTTP_CLIENT_CLEAN,
	WIFI_CMD_HTTP_CLIENT_SET_HEADER,
	WIFI_CMD_HTTP_CLIENT_SET_METHOD,
	WIFI_CMD_HTTP_CLIENT_SET_DATA,
	WIFI_CMD_HTTP_CLIENT_REQUEST,
	WIFI_CMD_HTTP_CLIENT_RESPONSE,
#endif /* defined(CONFIG_CLI_SUPPORT_HTTP_CLIENT) */

	WIFI_CMD_NUM,
} packet_cmd_t;



void cmdcontrol_init(void (*presponse_function)(char *resp));
void cmdcontrol_process(void *param);


#ifdef __cplusplus
}
#endif

#endif /* COMPONENTS_COMMANDLINE_H_ */
