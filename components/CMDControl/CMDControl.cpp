/*
 * commandline.cpp
 *
 *  Created on: Apr 19, 2023
 *      Author: anh
 */

#include "CMDControl.h"
#include "CMDFunction.h"
#include "parse_packet.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

#include "string.h"

#if CONFIG_CMD_SUPPORT_WIFI_CONTROL
#include "WiFi.h"
#endif /* CONFIG_CMD_SUPPORT_WIFI_CONTROL */
#if CONFIG_CMD_SUPPORT_HTTP_CLIENT
#include "esp_http_client.h"
#include "esp_event.h"
#include "esp_crt_bundle.h"
#endif /* CONFIG_CMD_SUPPORT_HTTP_CLIENT */

static const char *TAG = "CMD Control";
const char *cmdcontrol_command_string[] = {
	"WIFI_ERR",

	"WIFI_RESTART",
	/**
	 * Network control command.
	 */
#if defined(CONFIG_CMD_SUPPORT_WIFI_CONTROL)
	"WIFI_SCAN",
	"WIFI_ISCONNECTED",
	"WIFI_CONN",
	"WIFI_DISCONN",
	"WIFI_GETIP",
#endif /* defined(CONFIG_CLI_SUPPORT_WIFI_CONTROL) */

	/**
	 * HTTP client command.
	 */
#if defined(CONFIG_CMD_SUPPORT_HTTP_CLIENT)
	"WIFI_HTTP_CLIENT_NEW",
	"WIFI_HTTP_CLIENT_CONFIG",
	"WIFI_HTTP_CLIENT_INIT",
	"WIFI_HTTP_CLIENT_CLEAN",
	"WIFI_HTTP_CLIENT_SET_HEADER",
	"WIFI_HTTP_CLIENT_SET_URL",
	"WIFI_HTTP_CLIENT_SET_METHOD",
	"WIFI_HTTP_CLIENT_SET_DATA",
	"WIFI_HTTP_CLIENT_REQUEST",
	"WIFI_HTTP_CLIENT_RESPONSE",
#endif /* defined(CONFIG_CLI_SUPPORT_HTTP_CLIENT) */


	"WIFI_CMD_NUM",
};

void (*resp_pfunction)(char *resp);
void (*pevent_handler)(packet_cmd_t command);

static void cmdcontrol_handler(pkt_t *packet);

void cmdcontrol_init(void (*presponse_function)(char *resp)){
	resp_pfunction = presponse_function;
}

static void cmdcontrol_handler(pkt_t *packet){
	packet_cmd_t command = (packet_cmd_t)str_to_cmd(packet->cmd_str, cmdcontrol_command_string, (int)WIFI_CMD_NUM);
	ESP_LOGI(TAG, "Receive command %s, heap %lu", packet->cmd_str, esp_get_free_heap_size());
	switch(command){
		case WIFI_RESTART:
			esp_restart();
		break;
#if defined(CONFIG_CMD_SUPPORT_WIFI_CONTROL)
		case WIFI_GETIP:{
			cmdf_get_ip();
		}
		break;
		case WIFI_ISCONNECTED:{
			cmdf_isconnected();
		}
		break;
		case WIFI_SCAN:{
			cmdf_wifi_scan();
		}
		break;

		case WIFI_CONN:{
			cmdf_wifi_connect(packet);
		}
		break;

		case WIFI_DISCONN:{
			cmdf_wifi_disconnect();
		}
		break;
#endif /* defined(CONFIG_CMD_SUPPORT_WIFI_CONTROL) */



#if defined(CONFIG_CMD_SUPPORT_HTTP_CLIENT)
		case WIFI_HTTP_CLIENT_NEW:{
			cmdf_http_client_new(packet);
		}
		break;
		case WIFI_HTTP_CLIENT_CONFIG:{
			cmdf_http_client_config(packet);
		}
		break;
		case WIFI_HTTP_CLIENT_INIT:{
			cmdf_http_client_init(packet);
		}
		break;
		case WIFI_HTTP_CLIENT_CLEAN:{
			cmdf_http_client_clean(packet);
		}
		break;
		case WIFI_HTTP_CLIENT_SET_HEADER:{
			cmdf_http_client_set_header(packet);
		}
		break;
		case WIFI_HTTP_CLIENT_SET_URL:{
			cmdf_http_client_set_url(packet);
		}
		break;
		case WIFI_HTTP_CLIENT_SET_METHOD:{
			cmdf_http_client_set_method(packet);
		}
		break;
		case WIFI_HTTP_CLIENT_SET_DATA:{
			cmdf_http_client_set_data(packet);
		}
		break;
		case WIFI_HTTP_CLIENT_REQUEST:{
			cmdf_http_client_request(packet);
		}
		break;


#endif /* defined(CONFIG_CMD_SUPPORT_HTTP_CLIENT) */

		default:
			ESP_LOGE(TAG, "Unknown command!");
		break;
	}

}

void cmdcontrol_process(void *param){
	pkt_err_t ret = PKT_ERR_OK;
	char *req_full = (char *)param;
	pkt_json_t json;
	pkt_t pkt;

	/** Parse received packet */
	ret = parse_packet(req_full, &pkt);
	if(ret != PKT_ERR_OK){
		ESP_LOGE(TAG, "Packet parse error!");
		release_packet(&pkt);
		return;
	}

	/** Command handle */
	cmdcontrol_handler(&pkt);
	release_packet(&pkt);
}




