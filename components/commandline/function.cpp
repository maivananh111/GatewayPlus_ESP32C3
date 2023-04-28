/*
 * function.cpp
 *
 *  Created on: Apr 26, 2023
 *      Author: anh
 */
#include "sdkconfig.h"

#include "commandline.h"

#if CONFIG_CLI_SUPPORT_WIFI_CONTROL
#include "WiFi.h"
#endif /* CONFIG_CLI_SUPPORT_WIFI_CONTROL */
#if CONFIG_CLI_SUPPORT_HTTP_CLIENT
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_tls.h"
#endif /* CONFIG_CLI_SUPPORT_HTTP_CLIENT */

#include "stdio.h"
#include "string.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"


static const char *TAG = "CLI";

extern void (*resp_pfunction)(char *resp);

extern cli_err_t json_get_object(char *src, cli_json_t *dest, char *key);
extern cli_err_t json_release_object(cli_json_t *json);

#if CONFIG_CLI_SUPPORT_WIFI_CONTROL
void clif_check_connect(void);
void clif_get_ip(void);
void clif_wifi_scan(void);
void clif_wifi_connect(cli_packet_t *packet);
void clif_wifi_disconnect(void);


void clif_check_connect(void){
	char *buf;

	asprintf(&buf, "CLI_CMD_WIFI_CHECK_CONNECT: {\"isconnected\": %01d}", (int)WiFi_GetState());
	resp_pfunction((char *)buf);
	free(buf);
}

void clif_get_ip(void){
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

void clif_wifi_scan(void){
	uint8_t num_wifi = WiFi_STA_Scan();
	wifi_info_t info;
	int total_ssid_len = 0;

	/** Calculate all ssid length */
	for(uint8_t i=0; i<num_wifi; i++){
		WiFi_STA_Scan_Get_Info(i, &info);
		total_ssid_len += strlen(info.name);
		ESP_LOGW(TAG, "Find: %s, total len = %d", info.name, total_ssid_len);
	}
	/** Calculate total length */
	int total_len = strlen("CLI_CMD_WIFI_SCAN: {")+ total_ssid_len + (strlen("\"Noxx\": {\"ssid\": \"\", ") + strlen("\"rssi\": }, ") + 3) * num_wifi;
	ESP_LOGW(TAG, "Full Lenght = %d", total_len);
	char *ssid_str = (char *)malloc(total_len);
	memset(ssid_str, '\0', total_len);
	memcpy(ssid_str, "CLI_CMD_WIFI_SCAN: {", strlen("CLI_CMD_WIFI_SCAN: {"));

	for(uint8_t i=0; i<num_wifi; i++){
		WiFi_STA_Scan_Get_Info(i, &info);
		char *temp;
		asprintf(&temp, "\"No%02d\": {\"ssid\": \"%s\", \"rssi\": %03d}, ", i+1, info.name, info.rssi);
		strcat(ssid_str, temp);
		ESP_LOGE(TAG, "%s, Len = %d", temp, strlen(temp));
		free(temp);
	}
	ssid_str[total_len - 2] = '}';
	ssid_str[total_len - 1] = '\0';

	resp_pfunction((char *)ssid_str);
	free(ssid_str);
	WiFi_STA_Release_Info(&info);
}

void clif_wifi_connect(cli_packet_t *packet){
	char *ssid, *pass, *auth;
	cli_json_t dest;
	cli_err_t ret;

	ret = json_get_object(packet->data_str, &dest, "ssid");
	asprintf(&ssid, "%s", dest.value);
	json_release_object(&dest);

	ret = json_get_object(packet->data_str, &dest, "pass");
	asprintf(&pass, "%s", dest.value);
	json_release_object(&dest);

	ret = json_get_object(packet->data_str, &dest, "auth");
	asprintf(&auth, "%s", dest.value);
	json_release_object(&dest);

	ESP_LOGI(TAG, "WiFi connecting to %s - %s", ssid, pass);

	if(WiFi_GetState() == WIFI_CONNECTED){
		WiFi_STA_Disconnect();
	}

	while(WiFi_GetState() == WIFI_CONNECT_FAILED){
		WiFi_STA_Connect(ssid, pass, WiFi_StrToAuth(auth));
	};

	resp_pfunction("CLI_CMD_WIFI_CONN: OK");
}

void clif_wifi_disconnect(void){
	if(WiFi_GetState() == WIFI_CONNECTED){
		WiFi_STA_Disconnect();
	}

	ESP_LOGI(TAG, "WiFi disconnected");
	resp_pfunction("CLI_CMD_WIFI_DISCONN: OK");
}

#endif /* CONFIG_CLI_SUPPORT_WIFI_CONTROL */


#if CONFIG_CLI_SUPPORT_HTTP_CLIENT
#define NUM_HTTP_CLIENT 2
#define HTTP_WAIT_RESP_BIT BIT0

static esp_http_client_handle_t client;
static esp_http_client_config_t client_config;
static EventGroupHandle_t http_client_resp_bit = NULL;
static char *req_data = NULL;


static const char *http_client_method_str[] = {
	"HTTP_METHOD_GET",    /*!< HTTP GET Method */
	"HTTP_METHOD_POST",       /*!< HTTP POST Method */
	"HTTP_METHOD_PUT",        /*!< HTTP PUT Method */
	"HTTP_METHOD_PATCH",      /*!< HTTP PATCH Method */
	"HTTP_METHOD_DELETE",     /*!< HTTP DELETE Method */
	"HTTP_METHOD_HEAD",       /*!< HTTP HEAD Method */
	"HTTP_METHOD_NOTIFY",     /*!< HTTP NOTIFY Method */
	"HTTP_METHOD_SUBSCRIBE",  /*!< HTTP SUBSCRIBE Method */
	"HTTP_METHOD_UNSUBSCRIBE",/*!< HTTP UNSUBSCRIBE Method */
	"HTTP_METHOD_OPTIONS",    /*!< HTTP OPTIONS Method */
	"HTTP_METHOD_COPY",       /*!< HTTP COPY Method */
	"HTTP_METHOD_MOVE",       /*!< HTTP MOVE Method */
	"HTTP_METHOD_LOCK",       /*!< HTTP LOCK Method */
	"HTTP_METHOD_UNLOCK",     /*!< HTTP UNLOCK Method */
	"HTTP_METHOD_PROPFIND",   /*!< HTTP PROPFIND Method */
	"HTTP_METHOD_PROPPATCH",  /*!< HTTP PROPPATCH Method */
	"HTTP_METHOD_MKCOL",      /*!< HTTP MKCOL Method */
	"HTTP_METHOD_MAX",
};

void clif_http_client_new(cli_packet_t *packet);
void clif_http_client_config(cli_packet_t *packet);
void clif_http_client_connect(cli_packet_t *packet);
void clif_http_client_disconnect(cli_packet_t *packet);
void clif_http_client_set_header(cli_packet_t *packet);
void clif_http_client_set_method(cli_packet_t *packet);
void clif_http_client_set_data(cli_packet_t *packet);
void clif_http_client_request(cli_packet_t *packet);
esp_err_t clif_http_client_event_handler(esp_http_client_event_handle_t evt);

void clif_http_client_new(cli_packet_t *packet){
	cli_json_t json;
	cli_err_t err;

	if(http_client_resp_bit == NULL) http_client_resp_bit = xEventGroupCreate();
}

void clif_http_client_config(cli_packet_t *packet){
	cli_json_t json;
	cli_err_t err;

	/** URL */
	err = json_get_object(packet->data_str, &json, "url");
	if(err == CLI_ERR_OK) {
		asprintf((char **)&client_config.url, "%s", json.value);
		ESP_LOGW(TAG, "HTTP Set url %s", client_config.url);
		json_release_object(&json);
	}
	/** Path */
	err = json_get_object(packet->data_str, &json, "path");
	if(err == CLI_ERR_OK) {
		asprintf((char **)&client_config.path, "%s", json.value);
		ESP_LOGW(TAG, "HTTP Set path %s", client_config.path);
		json_release_object(&json);
	}

	/** Transport type */
	err = json_get_object(packet->data_str, &json, "transport_ssl");
	if(err == CLI_ERR_OK){
		if(json.value[0] == '1') {
			client_config.transport_type = HTTP_TRANSPORT_OVER_SSL;
			ESP_LOGW(TAG, "HTTP Set transport SSL");
		}
		else{
			client_config.transport_type = HTTP_TRANSPORT_OVER_TCP;
			ESP_LOGW(TAG, "HTTP Set transport TCP");
		}
		json_release_object(&json);
	}
	/** CRT bundle */
	err = json_get_object(packet->data_str, &json, "crt_bundle");
	if(err == CLI_ERR_OK){
		if(json.value[0] == '1') {
			client_config.crt_bundle_attach = esp_crt_bundle_attach;
			ESP_LOGW(TAG, "HTTP Set verify crt bundle");
		}
		json_release_object(&json);
	}
	/** Root cert pem */
	err = json_get_object(packet->data_str, &json, "cert_pem");
	if(err == CLI_ERR_OK) {
		asprintf((char **)&client_config.cert_pem, "%s", json.value);
		ESP_LOGW(TAG, "HTTP Set cert pem \r\n %s", client_config.cert_pem);
		json_release_object(&json);
	}

	/** User name */
	err = json_get_object(packet->data_str, &json, "username");
	if(err == CLI_ERR_OK) {
		asprintf((char **)&client_config.username, "%s", json.value);
		json_release_object(&json);
	}
	/** Password */
	err = json_get_object(packet->data_str, &json, "password");
	if(err == CLI_ERR_OK) {
		asprintf((char **)&client_config.password, "%s", json.value);
		json_release_object(&json);
	}
	/** User Agent */
	err = json_get_object(packet->data_str, &json, "user_agent");
	if(err == CLI_ERR_OK) {
		asprintf((char **)&client_config.user_agent, "%s", json.value);
		json_release_object(&json);
	}

	/** Event handler */
	client_config.event_handler = clif_http_client_event_handler;
	resp_pfunction("CLI_CMD_HTTP_CLIENT_CONFIG: OK");
}

void clif_http_client_connect(cli_packet_t *packet){
	cli_json_t json;
	cli_err_t err;

	client = esp_http_client_init(&client_config);
	resp_pfunction("CLI_CMD_HTTP_CLIENT_CONNECT: OK");
}

void clif_http_client_disconnect(cli_packet_t *packet){
	cli_json_t json;
	cli_err_t err;

	esp_http_client_cleanup(client);
	if(client_config.url != NULL) free((char *)client_config.url);
	if(client_config.path != NULL) free((char *)client_config.path);
	if(client_config.cert_pem != NULL) free((char *)client_config.cert_pem);
	if(client_config.username != NULL) free((char *)client_config.username);
	if(client_config.password != NULL) free((char *)client_config.password);
	if(client_config.user_agent != NULL) free((char *)client_config.user_agent
			);
	resp_pfunction("CLI_CMD_HTTP_CLIENT_DISCONNECT: OK");
}

void clif_http_client_set_header(cli_packet_t *packet){
	cli_json_t json;
	cli_err_t err;
	char *key, *value;

	/** Key */
	err = json_get_object(packet->data_str, &json, "key");
	if(err == CLI_ERR_OK) asprintf(&key, "%s", json.value);
	else resp_pfunction("CLI_CMD_HTTP_CLIENT_SET_HEADER: ERR");
	json_release_object(&json);
	/** Value */
	err = json_get_object(packet->data_str, &json, "value");
	if(err == CLI_ERR_OK) asprintf(&value, "%s", json.value);
	else resp_pfunction("CLI_CMD_HTTP_CLIENT_SET_HEADER: ERR");
	json_release_object(&json);

	esp_http_client_set_header(client, key, value);

	if(key) free(key);
	if(value) free(value);
	resp_pfunction("CLI_CMD_HTTP_CLIENT_SET_HEADER: OK");
}

void clif_http_client_set_method(cli_packet_t *packet){
	cli_json_t json;
	cli_err_t err;
	int i = 0;

	/** Get method */
	err = json_get_object(packet->data_str, &json, "method");
	if(err == CLI_ERR_OK){
		for(i=0; i<18; i++){
			if(strcmp(json.value, http_client_method_str[i]) == 0) break;
		}

		esp_http_client_set_method(client, (esp_http_client_method_t)i);
		resp_pfunction("CLI_CMD_HTTP_CLIENT_SET_METHOD: OK");
	}
	else{
		resp_pfunction("CLI_CMD_HTTP_CLIENT_SET_METHOD: ERR");
	}
	json_release_object(&json);
}

void clif_http_client_set_data(cli_packet_t *packet){
	cli_json_t json;
	cli_err_t err;

	/** Data */
	err = json_get_object(packet->data_str, &json, "data");
	if(err == CLI_ERR_OK) {
		asprintf(&req_data, "%s", json.value);
		esp_http_client_set_post_field(client, req_data, strlen(req_data));
		json_release_object(&json);
		resp_pfunction("CLI_CMD_HTTP_CLIENT_SET_DATA: OK");
	}
	else
		resp_pfunction("CLI_CMD_HTTP_CLIENT_SET_DATA: ERR");
}

void clif_http_client_request(cli_packet_t *packet){
	cli_json_t json;
	cli_err_t err;

	esp_http_client_perform(client);

	if(req_data)  free(req_data);

    EventBits_t bits = xEventGroupWaitBits(http_client_resp_bit, HTTP_WAIT_RESP_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if(bits & HTTP_WAIT_RESP_BIT){
    	xEventGroupClearBits(http_client_resp_bit, HTTP_WAIT_RESP_BIT);
    	resp_pfunction("CLI_CMD_HTTP_CLIENT_REQUEST: OK");
    }
    else
    	resp_pfunction("CLI_CMD_HTTP_CLIENT_REQUEST: ERR");
}

esp_err_t clif_http_client_event_handler(esp_http_client_event_handle_t evt){
    switch(evt->event_id){
		case HTTP_EVENT_ERROR:

		break;
		case HTTP_EVENT_ON_CONNECTED:
			ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
			resp_pfunction("CLI_CMD_HTTP_CLIENT_CONNECT: OK");
		break;
		case HTTP_EVENT_DISCONNECTED:
			ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
			resp_pfunction("CLI_CMD_HTTP_CLIENT_DISCONNECT: OK");
		break;
		case HTTP_EVENT_HEADER_SENT:

		break;
		case HTTP_EVENT_ON_HEADER:
			ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
		break;
		case HTTP_EVENT_ON_DATA:{
			int status_code = esp_http_client_get_status_code(client);
			char *resp_data = (char *)malloc(evt->data_len + 1);
			memcpy(resp_data, (char *)evt->data, evt->data_len);
			resp_data[evt->data_len] = '\0';

			int total_len = strlen("CLI_CMD_HTTP_CLIENT_RESPONSE: {\"status\": , \"data\": \"\"}") + 3 + strlen(resp_data);
			char *full_resp = (char *)malloc(total_len+1);
			sprintf(full_resp, "CLI_CMD_HTTP_CLIENT_RESPONSE: {\"status\": %03d, \"data\": \"%s\"}", status_code, resp_data);
			full_resp[total_len] = '\0';

			if(status_code != 200) ESP_LOGI("HTTP", "Responsed status: %d", status_code);

			xEventGroupSetBits(http_client_resp_bit, HTTP_WAIT_RESP_BIT);
			resp_pfunction(full_resp);

			if(resp_data) free(resp_data);
			if(full_resp) free(full_resp);
    	}
		break;
		default:
		break;
    }
    return ESP_OK;
}
#endif /* CONFIG_CLI_SUPPORT_HTTP_CLIENT */







