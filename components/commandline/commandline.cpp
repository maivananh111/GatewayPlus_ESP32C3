/*
 * commandline.cpp
 *
 *  Created on: Apr 19, 2023
 *      Author: anh
 */

#include "commandline.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

#include "string.h"

#if CONFIG_CLI_SUPPORT_WIFI_CONTROL
#include "WiFi.h"
#endif /* CONFIG_CLI_SUPPORT_WIFI_CONTROL */
#if CONFIG_CLI_SUPPORT_HTTP_CLIENT
#include "esp_http_client.h"
#include "esp_event.h"
#include "esp_crt_bundle.h"
#endif /* CONFIG_CLI_SUPPORT_HTTP_CLIENT */

static const char *TAG = "CLI";
const char *cli_command_string[] = {
	"CLI_CMD_ERR",
	/**
	 * Network control command.
	 */
#if defined(CONFIG_CLI_SUPPORT_WIFI_CONTROL)
	"CLI_CMD_WIFI_SCAN",
	"CLI_CMD_WIFI_CHECK_CONNECT",
	"CLI_CMD_WIFI_CONN",
	"CLI_CMD_WIFI_DISCONN",
	"CLI_CMD_WIFI_GETIP",
#endif /* defined(CONFIG_CLI_SUPPORT_WIFI_CONTROL) */

	/**
	 * HTTP client command.
	 */
#if defined(CONFIG_CLI_SUPPORT_HTTP_CLIENT)
	"CLI_CMD_HTTP_CLIENT_NEW",
	"CLI_CMD_HTTP_CLIENT_CONFIG",
	"CLI_CMD_HTTP_CLIENT_CONNECT",
	"CLI_CMD_HTTP_CLIENT_DISCONNECT",
	"CLI_CMD_HTTP_CLIENT_SET_HEADER",
	"CLI_CMD_HTTP_CLIENNT_SET_DATA",
	"CLI_CMD_HTTP_CLIENT_SET_METHOD",
	"CLI_CMD_HTTP_CLIENT_REQUEST",
#endif /* defined(CONFIG_CLI_SUPPORT_HTTP_CLIENT) */


	"CLI_CMD_NUM",
};


void (*resp_pfunction)(char *resp);
void (*pevent_handler)(cli_cmd_t command);

/**
 * json function.
 */
typedef struct {
	char *key = NULL;
	bool leaf = false;
	char *value = NULL;
} cli_json_t;

static cli_err_t json_get_object(char *src, cli_json_t *dest, char *key);
static cli_err_t json_release_object(cli_json_t *json);
static cli_err_t cli_parse_packet(char *src,cli_packet_t *dest);
static cli_err_t cli_release_packet(cli_packet_t *packet);
static cli_cmd_t cli_str2cmd(char *str);
static void cli_command_handler(cli_cmd_t cmd);
static void cli_error_handler(char *str, int line, char *func);

static void cli_error_handler(char *str, int line, char *func){
	ESP_LOGE(TAG, "%s, Line %d Function %s", str, line, func);
}

static cli_err_t json_get_object(char *src, cli_json_t *dest, char *key){
	cli_err_t ret = CLI_ERR_OK;
	char *src_cpy = src;
	int src_len = strlen(src);
	int key_len = 0, val_len = 0;
	char *pkstart, *pend, *pvstart;
	int ivstart = 0, ivend = 0;

	/** check input */
	if(src == NULL || dest == NULL || key == NULL){
		cli_error_handler((char *)"Error bad input argument!", (int)__LINE__, (char *)__FUNCTION__);
		ret = CLI_ERR_ARG;
		return ret;
	}
	if(src[0] != '{' || src[src_len - 1] != '}' || src[src_len] != '\0'){
		cli_error_handler((char *)"Error input request string format!", (int)__LINE__, (char *)__FUNCTION__);
		ret = CLI_ERR_FORMAT;
		return ret;
	}

	/** Find key */
	pkstart = strstr(src_cpy, key);
	if(pkstart == NULL){
		cli_error_handler((char *)"Error key not appear in the input request string!", (int)__LINE__, (char *)__FUNCTION__);
		ret = CLI_ERR_NOKEY;
		return ret;
	}
	/**
	 * Get key
	 * */
	for(key_len=0; key_len<strlen(pkstart); key_len++){
		if(pkstart[key_len] == '"') break;
	}

	dest->key = (char *)malloc((key_len+1) * sizeof(char));
	if(dest->key == NULL){
		cli_error_handler((char *)"Error can't allocation memory!", (int)__LINE__, (char *)__FUNCTION__);
		ret = CLI_ERR_MEM;
		return ret;
	}
	memcpy(dest->key, pkstart, key_len); 	/** assign key to jsn struct */
	dest->key[key_len] = '\0';

	/**
	 * Get value
	 * */
	/** Find Value start index */
	ivstart = (int)((pkstart - src_cpy) + key_len + 3);
	pvstart = pkstart;
	if((char)(*(uint32_t *)(pvstart + key_len + 3)) != '{') dest->leaf = true;

	/** Get start point off value */
	pvstart = (char *)(pvstart + key_len + 3);
	/** Check leaf item */
	if(dest->leaf == true){
		if((char)(*pvstart) == '"') { /** Value is string */
			pvstart++;

			for(val_len=0; val_len<strlen(pvstart); val_len++){
				if(pvstart[val_len] == '"') break;
			}
		}
		else{ /** Value is number or everythings */
			for(val_len=0; val_len<strlen(pvstart); val_len++){
				if(pvstart[val_len] == '}' || pvstart[val_len] == ',') break;
			}
		}
		if(val_len == 0){
			cli_error_handler((char *)"Error key no value!", (int)__LINE__, (char *)__FUNCTION__);
			ret = CLI_ERR_NOVAL;
			return ret;
		}
		dest->value = (char *)malloc((val_len+1) * sizeof(char));
		if(dest->value == NULL){
			cli_error_handler((char *)"Error can't allocation memory!", (int)__LINE__, (char *)__FUNCTION__);
			ret = CLI_ERR_MEM;
			return ret;
		}
		memcpy(dest->value, pvstart, val_len); 	/** assign key to jsn struct */
		dest->value[val_len] = '\0';
	}
	else{
		/** Search right brace } */
		int l_brace = 0, r_brace = 0;
		for(ivend=ivstart; ivend<src_len; ivend++){
			if(src_cpy[ivend] == '{') l_brace++;
			if(src_cpy[ivend] == '}') r_brace++;
			if(l_brace == r_brace) break;
		}
		val_len = ivend - ivstart + 1;
		dest->value = (char *)malloc(val_len + 1);
		if(dest->value == NULL){
			cli_error_handler((char *)"Error can't allocation memory!", (int)__LINE__, (char *)__FUNCTION__);
			ret = CLI_ERR_MEM;
			return ret;
		}
		memcpy(dest->value, pvstart, val_len); 	/** assign key to jsn struct */
		dest->value[val_len] = '\0';
	}

	return ret;
}

static cli_err_t json_release_object(cli_json_t *json){
	if(json->key != NULL) free(json->key);
	if(json->value != NULL) free(json->value);
	json->leaf = false;

	return CLI_ERR_OK;
}

static cli_cmd_t cli_str2cmd(char *str){
	cli_cmd_t cmd = CLI_CMD_ERR;

	for(int i=0; i<(int)CLI_CMD_NUM; i++){
		if(strcmp(str, cli_command_string[i]) == 0){
			cmd = (cli_cmd_t)i;
			return cmd;
		}
	}

	return cmd;
}

static cli_err_t cli_parse_packet(char *src,cli_packet_t *dest){
	cli_err_t ret = CLI_ERR_OK;
	char *src_cpy = src;
	int src_len = strlen(src_cpy), data_len = 0;
	int ivstart = 0, cmd_len = 0;
	char *pvstart;

	/** Get ": " */
	pvstart = strstr(src, ": ");
	if(pvstart == NULL){
		cli_error_handler((char *)"Error packet format!", (int)__LINE__, (char *)__FUNCTION__);
		ret = CLI_ERR_FORMAT;
		dest->command = CLI_CMD_ERR;
		return ret;
	}

	/** Get command length */
	cmd_len = (int)(pvstart - src_cpy);

	/** Assign command string */
	dest->cmd_str = (char *)malloc((cmd_len + 1) * sizeof(char));
	if(dest->cmd_str == NULL){
		cli_error_handler((char *)"Error can't allocation memory!", (int)__LINE__, (char *)__FUNCTION__);
		ret = CLI_ERR_MEM;
		return ret;
	}
	memcpy(dest->cmd_str, src_cpy, cmd_len);
	dest->cmd_str[cmd_len] = '\0';

	/** Get command type */
	dest->command = cli_str2cmd(dest->cmd_str);

	/** Get data */
	pvstart = (char *)(pvstart + 2);
	data_len = strlen(pvstart);
	dest->data_str = (char *)malloc((data_len + 1) * sizeof(char));
	if(dest->data_str == NULL){
		cli_error_handler((char *)"Error can't allocation memory!", (int)__LINE__, (char *)__FUNCTION__);
		ret = CLI_ERR_MEM;
		return ret;
	}
	memcpy(dest->data_str, pvstart, data_len);
	dest->data_str[data_len] = '\0';

	return ret;
}

static cli_err_t cli_release_packet(cli_packet_t *packet){
	if(packet->cmd_str != NULL) free(packet->cmd_str);
	if(packet->data_str != NULL) free(packet->data_str);
	packet->command = CLI_CMD_ERR;

	return CLI_ERR_OK;
}

void commandline_init(void (*presponse_function)(char *resp)){
	resp_pfunction = presponse_function;
}

static void cli_command_handler(cli_cmd_t cmd){
	switch(cmd){
		/**
		 * CLI_CMD_ERR
		 */
		case CLI_CMD_ERR:{
			cli_error_handler((char *)"Unknown command!", (int)__LINE__, (char *)__FUNCTION__);
		}
		break;

		/**
		 * CLI_CMD_WIFI_GETIP.
		 */
		case CLI_CMD_WIFI_GETIP:{
			char *buf;
			wifi_ipinfo_t ipinfo;
			if(WiFi_STA_Get_IPInfo(&ipinfo) == ESP_OK){
				/** handle response IP */
				asprintf(&buf, "CLI_CMD_WIFI_GETIP: {\"ip\": \"%s\", \"netmask\": \"%s\", \"gateway\": \"%s\"}",
						ipinfo.ip, ipinfo.netmask, ipinfo.gateway);

				WiFi_STA_Release_IPInfo(&ipinfo);
			}
			else{
				asprintf(&buf, "CLI_CMD_WIFI_GETIP: {\"ip\": \"0.0.0.0\", \"netmask\": \"0.0.0.0\", \"gateway\": \"0.0.0.0\"}");
			}

			resp_pfunction(buf);
			free(buf);
		}
		break;

		/**
		 * CLI_CMD_WIFI_CHECK_CONNECT.
		 */
		case CLI_CMD_WIFI_CHECK_CONNECT:{
			char *buf;

			asprintf(&buf, "CLI_CMD_WIFI_CHECK_CONNECT: {\"isconnected\": %01d}", (int)WiFi_GetState());

			resp_pfunction((char *)buf);

			free(buf);
		}
		break;

		/**
		 * CLI_CMD_WIFI_SCAN.
		 */
		case CLI_CMD_WIFI_SCAN:{
			char *buf = "CLI_CMD_WIFI_SCAN: {\"ssid\": ";
			uint8_t num_wifi = WiFi_STA_Scan();
			wifi_info_t info;
			int total_len = strlen(buf);
			for(uint8_t i=0; i<num_wifi; i++){
				WiFi_STA_Scan_Get_Info(i, &info);
				 total_len += strlen(info.name);
				ESP_LOGW(TAG, "Find: %s, total len = %d", info.name, total_len);
//				buf = (char *)realloc(buf, total_len);
//				strcat(buf, info.name);
//				buf[total_len-1] = '|';
			}

//			resp_pfunction((char *)buf);
//			free(buf);
//			WiFi_STA_Release_Info(&info);
		}
		break;

		case CLI_CMD_WIFI_CONN:{

		}
		break;

		case CLI_CMD_WIFI_DISCONN:{

		}
		break;

		default:

		break;
	}

}

void commandline_process(void *param){
//	QueueHandle_t *req_queue = (QueueHandle_t *)param;
	cli_err_t ret = CLI_ERR_OK;
	char *req_full = (char *)param;
	cli_json_t json;
	cli_packet_t pkt;
	cli_cmd_t cmd;

//	while(1){
//		if(xQueueReceive(*req_queue, (void *)&req_full, (TickType_t)portMAX_DELAY)){
			/** Parse received packet */
			ret = cli_parse_packet(req_full, &pkt);
			if(ret != CLI_ERR_OK){
				cli_error_handler((char *)"Error parse packet!", (int)__LINE__, (char *)__FUNCTION__);
				cli_release_packet(&pkt);
//				continue;
				return;
			}

			cmd = pkt.command;
			ESP_LOGI(TAG, "Packet command: %s", pkt.cmd_str);
			ESP_LOGI(TAG, "Packet data: %s", pkt.data_str);
			ESP_LOGI(TAG, "Packet command number %d", (int)pkt.command);
			/** Command handle */
			cli_command_handler(cmd);

//			free(req_full);
//			vTaskDelay(10/portTICK_PERIOD_MS);
//		}
//	}
}




