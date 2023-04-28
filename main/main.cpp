#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "sdkconfig.h"

#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"

#include "WiFi.h"
#include "commandline.h"
#include "FireBase.h"

const char *TAG = "main";

FireBase fb;
const char *root_cert_pem = (const char *)  "-----BEGIN CERTIFICATE-----\r\n"
											"MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\r\n"
											"A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\r\n"
											"b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\r\n"
											"MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\r\n"
											"YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\r\n"
											"aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\r\n"
											"jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\r\n"
											"xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\r\n"
											"1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\r\n"
											"snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\r\n"
											"U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\r\n"
											"9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\r\n"
											"BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\r\n"
											"AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\r\n"
											"yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\r\n"
											"38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\r\n"
											"AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\r\n"
											"DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\r\n"
											"HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\r\n"
											"-----END CERTIFICATE-----\r\n";

void cmd_response(char *str){
	if(str != NULL)
		ESP_LOGW(TAG, "%s", str);
}
char *cmd;

extern "C" void app_main(void){
	ESP_ERROR_CHECK(nvs_flash_init() );
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());



	commandline_init(cmd_response);
	ESP_LOGE(TAG, "Free heap = %lu", esp_get_free_heap_size());
	asprintf(&cmd, "CLI_CMD_WIFI_SCAN: {}");
	commandline_process((void *)cmd); free(cmd);
	asprintf(&cmd, "CLI_CMD_WIFI_CHECK_CONNECT: {}");
	commandline_process((void *)cmd); free(cmd);
	asprintf(&cmd, "CLI_CMD_WIFI_CONN: {\"ssid\": \"Nhat Nam\", \"pass\": \"0989339608\", \"auth\": \"WIFI_AUTH_WPA2_PSK\"}");
	commandline_process((void *)cmd); free(cmd);
	asprintf(&cmd, "CLI_CMD_WIFI_GETIP: {}");
	commandline_process((void *)cmd); free(cmd);



	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_NEW: {\"client_id\": 0}");
	commandline_process((void *)cmd); free(cmd);
	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_CONFIG: {\"url\": \"https://tt-iot-9b421-default-rtdb.asia-southeast1.firebasedatabase.app/.json\", "
														 "\"transport_ssl\": 1, "
														 "\"crt_bundle\": 1}");
	commandline_process((void *)cmd); free(cmd);
	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_CONNECT: {}");
	commandline_process((void *)cmd); free(cmd);
	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_SET_HEADER: {\"key\": \"Content-Type\", \"value\": \"application/json\"}");
	commandline_process((void *)cmd); free(cmd);
	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_SET_METHOD: {\"method\": \"HTTP_METHOD_PATCH\"}");
	commandline_process((void *)cmd); free(cmd);

	ESP_LOGE(TAG, "Free heap = %lu", esp_get_free_heap_size());

	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_SET_DATA: {\"data\": {\"Temperature\": \"abcdef\", \"Humidity\": \"1111111.5\"}}");
	commandline_process((void *)cmd); free(cmd);
	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_REQUEST: {}");
	commandline_process((void *)cmd); free(cmd);

	ESP_LOGE(TAG, "Free heap = %lu", esp_get_free_heap_size());

	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_SET_DATA: {\"data\": {\"Temperature\": \"115200.3\", \"Humidity\": \"159852.5\"}}");
	commandline_process((void *)cmd); free(cmd);
	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_REQUEST: {}");
	commandline_process((void *)cmd); free(cmd);

	ESP_LOGE(TAG, "Free heap = %lu", esp_get_free_heap_size());

	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_SET_DATA: {\"data\": {\"Temperature\": \"456426551.3\", \"Humidity\": \"459794784957.5\"}}");
	commandline_process((void *)cmd); free(cmd);
	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_REQUEST: {}");
	commandline_process((void *)cmd); free(cmd);

	ESP_LOGE(TAG, "Free heap = %lu", esp_get_free_heap_size());


	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_DISCONNECT: {}");
	commandline_process((void *)cmd); free(cmd);
//	asprintf(&cmd, "CLI_CMD_WIFI_DISCONN: {}");
//	commandline_process((void *)cmd); free(cmd);
//	ESP_LOGE(TAG, "Free heap = %lu", esp_get_free_heap_size());


/** ---------------------------------------------------------------------------- */


	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_NEW: {\"client_id\": 0}");
	commandline_process((void *)cmd); free(cmd);
	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_CONFIG: {\"url\": \"https://tt-iot-9b421-default-rtdb.asia-southeast1.firebasedatabase.app/.json\", "
														 "\"transport_ssl\": 1, "
														 "\"crt_bundle\": 1}");
	commandline_process((void *)cmd); free(cmd);
	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_CONNECT: {}");
	commandline_process((void *)cmd); free(cmd);
	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_SET_HEADER: {\"key\": \"Content-Type\", \"value\": \"application/json\"}");
	commandline_process((void *)cmd); free(cmd);
	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_SET_METHOD: {\"method\": \"HTTP_METHOD_PATCH\"}");
	commandline_process((void *)cmd); free(cmd);

	ESP_LOGE(TAG, "Free heap = %lu", esp_get_free_heap_size());

	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_SET_DATA: {\"data\": {\"Temperature\": \"abcdef\", \"Humidity\": \"1111111.5\"}}");
	commandline_process((void *)cmd); free(cmd);
	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_REQUEST: {}");
	commandline_process((void *)cmd); free(cmd);

	ESP_LOGE(TAG, "Free heap = %lu", esp_get_free_heap_size());

	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_SET_DATA: {\"data\": {\"Temperature\": \"115200.3\", \"Humidity\": \"159852.5\"}}");
	commandline_process((void *)cmd); free(cmd);
	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_REQUEST: {}");
	commandline_process((void *)cmd); free(cmd);

	ESP_LOGE(TAG, "Free heap = %lu", esp_get_free_heap_size());

	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_SET_DATA: {\"data\": {\"Temperature\": \"456426551.3\", \"Humidity\": \"459794784957.5\"}}");
	commandline_process((void *)cmd); free(cmd);
	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_REQUEST: {}");
	commandline_process((void *)cmd); free(cmd);

	ESP_LOGE(TAG, "Free heap = %lu", esp_get_free_heap_size());


	asprintf(&cmd, "CLI_CMD_HTTP_CLIENT_DISCONNECT: {}");
	commandline_process((void *)cmd); free(cmd);
	asprintf(&cmd, "CLI_CMD_WIFI_DISCONN: {}");
	commandline_process((void *)cmd); free(cmd);
	ESP_LOGE(TAG, "Free heap = %lu", esp_get_free_heap_size());

    while (true) {
        vTaskDelay(1000);
    	asprintf(&cmd, "CLI_CMD_RESTART: {}");
    	commandline_process((void *)cmd); free(cmd);
    }
}
