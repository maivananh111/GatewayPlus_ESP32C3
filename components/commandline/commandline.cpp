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


static const char *TAG = "CLI";
const char *cli_command_string[] = {
	"CLI_CMD_ERR",
	/**
	 * Network control command.
	 */
#if defined(CONFIG_CLI_SUPPORT_WIFI_CONTROL)
	"CLI_CMD_NET_WIFI_SCAN",
	"CLI_CMD_NET_WIFI_CONN",
	"CLI_CMD_NET_WIFI_DISCONN",
	"CLI_CMD_NET_GETIP",
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

static cli_cmd_t cli_str2cmd(char *str);

void commandline_init(void (*presponse_function)(char *resp)){
	resp_pfunction = presponse_function;
}

void commandline_register_handler(void (*pfunction)(cli_cmd_t command)){
	pevent_handler = pfunction;
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
		ESP_LOGE(TAG, "Input argument error!");
		ret = CLI_ERR_ARG;
		return ret;
	}
	if(src[0] != '{' || src[src_len - 1] != '}'){
		ESP_LOGE(TAG, "Input request string format error!");
		ret = CLI_ERR_FORMAT;
		return ret;
	}

	/** Find key */
	pkstart = strstr(src_cpy, key);
	if(pkstart == NULL){
		ESP_LOGE(TAG, "Key \"%s\" not appear in the input request string!", key);
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
	memcpy(dest->key, pkstart, key_len); 	/** assign key to jsn struct */
	dest->key[key_len] = '\0';

	/**
	 * Get value
	 * */
	/** Find Value start index */
	ivstart = (int)((pkstart - src_cpy) + key_len + 3);
	pvstart = pkstart;
	if((char)(*(uint32_t *)(pvstart + key_len + 3)) != '{'){
		dest->leaf = true;
		ESP_LOGE(TAG, "\"%s\" is leaf item.", dest->key);
	}

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
			ESP_LOGE(TAG, "Key %s no value.", dest->key);
			ret = CLI_ERR_NOVAL;
			return ret;
		}
		dest->value = (char *)malloc((val_len+1) * sizeof(char));
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
		memcpy(dest->value, pvstart, val_len); 	/** assign key to jsn struct */
		dest->value[val_len] = '\0';
	}

	return ret;
}

static cli_err_t json_release_object(cli_json_t *json){
	cli_err_t ret;
	if(json->key != NULL) free(json->key);
	else{
		ESP_LOGE(TAG, "Error release json object.");
		ret = CLI_ERR_RELEASE;
		return ret;
	}
	if(json->value != NULL) free(json->value);
	else{
		ESP_LOGE(TAG, "Error release json object.");
		ret = CLI_ERR_RELEASE;
		return ret;
	}
	json->leaf = false;

	return ret;
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

cli_err_t cli_parse_packet(char *src,cli_packet_t *dest){
	cli_err_t ret;
	char *src_cpy = src;
	int src_len = strlen(src_cpy), data_len = 0;
	int ivstart = 0, cmd_len = 0;
	char *pvstart;

	/** Get ": " */
	pvstart = strstr(src, ": ");
	if(pvstart == NULL){
		ESP_LOGE(TAG, "Packet format error!");
		ret = CLI_ERR_FORMAT;
		return ret;
	}

	/** Get command length */
	cmd_len = (int)(pvstart - src_cpy);

	/** Assign command string */
	dest->cmd_str = (char *)malloc((cmd_len + 1) * sizeof(char));
	memcpy(dest->cmd_str, src_cpy, cmd_len);
	dest->cmd_str[cmd_len] = '\0';

	/** Get command type */
	dest->command = cli_str2cmd(dest->cmd_str);

	/** Get data */
	pvstart = (char *)(pvstart + 2);
	data_len = strlen(pvstart);
	dest->data_str = (char *)malloc((data_len + 1) * sizeof(char));
	memcpy(dest->data_str, pvstart, data_len);
	dest->data_str[data_len] = '\0';

	return ret;
}

void commandline_process(void *param){
	QueueHandle_t *req_queue = (QueueHandle_t *)param;
	char *req_full;
	while(1){
		if(xQueueReceive(*req_queue, (void *)&req_full, (TickType_t)portMAX_DELAY)){



			free(req_full);
		}
		vTaskDelay(10/portTICK_PERIOD_MS);
	}

}




