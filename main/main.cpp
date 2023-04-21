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

cli_packet_t pkt;

extern "C" void app_main(void){
	ESP_ERROR_CHECK(nvs_flash_init() );
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	char *json = (char *)"{\"abcd\": {"
			                      "\"cdef\": {"
			                     	 	 "\"anh\": \"Mai Van Anh\","
			                     	 	 "\"em\": 6789,"
			                      "},"
			                      "\"hgi\": 852"
			                      "},"
								  "\"mno\": 115200"
						 "}";
	char *packet = "CLI_CMD_HTTP_CLIENNT_SET_DATA: FREE, 0986382835";
	ESP_LOGE(TAG, "Free heap = %lu", esp_get_free_heap_size());
	cli_parse_packet(packet, &pkt);
	ESP_LOGI(TAG, "Command: %s", pkt.cmd_str);
	ESP_LOGI(TAG, "Command num: %d", (int)pkt.command);
	ESP_LOGI(TAG, "Data: %s", pkt.data_str);
	ESP_LOGE(TAG, "Free heap = %lu", esp_get_free_heap_size());

    while (true) {
        sleep(1);
    }
}
