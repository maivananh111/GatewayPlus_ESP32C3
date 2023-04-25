/*
 * commandline.h
 *
 *  Created on: Apr 19, 2023
 *      Author: anh
 */

#ifndef COMPONENTS_COMMANDLINE_H_
#define COMPONENTS_COMMANDLINE_H_

#include "stdio.h"

#include "sdkconfig.h"



#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	CLI_CMD_ERR = 0x00,
	/**
	 * Network control command.
	 */
#if defined(CONFIG_CLI_SUPPORT_WIFI_CONTROL)
	CLI_CMD_WIFI_SCAN,
	CLI_CMD_WIFI_CHECK_CONNECT,
	CLI_CMD_WIFI_CONN,
	CLI_CMD_WIFI_DISCONN,
	CLI_CMD_WIFI_GETIP,
#endif /* defined(CONFIG_CLI_SUPPORT_WIFI_CONTROL) */

	/**
	 * HTTP client command.
	 */
#if defined(CONFIG_CLI_SUPPORT_HTTP_CLIENT)
	CLI_CMD_HTTP_CLIENT_NEW,
	CLI_CMD_HTTP_CLIENT_CONFIG,
	CLI_CMD_HTTP_CLIENT_CONNECT,
	CLI_CMD_HTTP_CLIENT_DISCONNECT,
	CLI_CMD_HTTP_CLIENT_SET_HEADER,
	CLI_CMD_HTTP_CLIENNT_SET_DATA,
	CLI_CMD_HTTP_CLIENT_SET_METHOD,
	CLI_CMD_HTTP_CLIENT_REQUEST,
#endif /* defined(CONFIG_CLI_SUPPORT_HTTP_CLIENT) */

	CLI_CMD_NUM,
} cli_cmd_t;

typedef enum{
	CLI_ERR_OK,
	CLI_ERR_ARG    = 0x00000001,
	CLI_ERR_FORMAT = 0x00000002,
	CLI_ERR_NOKEY  = 0x00000004,
	CLI_ERR_NOVAL  = 0x00000008,
	CLI_ERR_MEM    = 0x00000010,
} cli_err_t;


typedef struct{
	char *cmd_str = NULL;
	char *data_str = NULL;
	cli_cmd_t command = CLI_CMD_ERR;
}cli_packet_t;



extern const char *cli_command_string[];

void commandline_init(void (*presponse_function)(char *resp));
void commandline_process(void *param);





#ifdef __cplusplus
}
#endif

#endif /* COMPONENTS_COMMANDLINE_H_ */
