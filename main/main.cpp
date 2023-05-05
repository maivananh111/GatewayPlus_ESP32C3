#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "sdkconfig.h"

#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "hal/uart_types.h"

#include "WiFi.h"
#include "FireBase.h"

#include "CMDControl.h"

const char *TAG = "main";

FireBase fb;
/*
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
*/
#define BUF_SIZE (4096)

char *full_struct = (char *)"{\"Kho1\":{"\
										"\"data\":{\"temp\":36.8,\"humi\":81.2,\"current\":15,\"time\":\"220050 050323\"}"\
									   "}"\
							"}";
char *data_struct = (char *)"\"data\":{\"temp\":36.8,\"humi\":81.2,\"current\":15,\"time\":\"220050 050323\"}";
char *ctrl_struct = (char *)"\"control\":{\"relay1\":1,\"relay2\":0,\"relay3\":0,\"relay4\":1}";
char *sett_struct = (char *)"\"settings\":{\"mode\":1,\"type\":1,\"smax_temp\":38.5,\"smin_temp\":37.5,\"stime_start\":\"10:25:15\",\"stime_stop\":\"11:30:00\"}";
char *prop_struct = (char *)"\"properties\":{\"address\":0xABCD1234,\"name\":\"Kho1\"}";

static QueueHandle_t uart_queue;
static QueueHandle_t cmd_queue;

static void uart_event_task(void *);
void cmd_response(char *str);
static void cmd_process(void *);
void shutdown_handler(void);

extern "C" void app_main(void){
	ESP_ERROR_CHECK(nvs_flash_init() );
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	esp_register_shutdown_handler(shutdown_handler);

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart_queue, 0);
    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    ESP_LOGW(TAG, "WiFi Module Starting............................");
    uart_write_bytes(UART_NUM_0, (const void *)"WIFI_RESTART: OK", (size_t)strlen("WIFI_RESTART: OK"));

//    UART_FIFO_LEN
//	UART_FULL_THRESH_DEFAULT
    cmdcontrol_init(cmd_response);

    xTaskCreate(uart_event_task, "uart_event_task", 10240, NULL, 12, NULL);
    xTaskCreate(cmd_process, "cmd_process", 20480, NULL, 10, NULL);

    while (true) {
        vTaskDelay(1000);
    }
}

void cmd_response(char *str){
	if(str != NULL){
		uart_write_bytes((uart_port_t)UART_NUM_0, (const void *)str, (size_t)strlen(str));
		ESP_LOGI(TAG, "Sent %s", str);
		vTaskDelay(50/portTICK_PERIOD_MS);
	}

}

static void uart_event_task(void *){
    uart_event_t event;

    while(1) {
        if(xQueueReceive(uart_queue, (void *)&event, (TickType_t)portMAX_DELAY)) {
            switch(event.type) {
                case UART_DATA:{
                	int length = 0;
                	uart_get_buffered_data_len(UART_NUM_0, (size_t *)&length);
                	char *buf = (char *)malloc(length + 1);
                    uart_read_bytes(UART_NUM_0, buf, length, portMAX_DELAY);
                    buf[length] = '\0';
                    ESP_LOGW(TAG, "%s", buf);
                    if(xQueueSend(cmd_queue, &buf, portMAX_DELAY) != pdTRUE){
                    	ESP_LOGE(TAG, "Can't send item to queue");
                    }
                }
				break;

                default:
                    ESP_LOGI(TAG, "uart0 event type: %d", event.type);
				break;
            }
        }
    }
    vTaskDelete(NULL);
}

static void cmd_process(void *){
	char *queue_data;

	cmd_queue = xQueueCreate(5, sizeof(uint32_t));

	while(1){
		 if(xQueueReceive(cmd_queue, &queue_data, (TickType_t)portMAX_DELAY)) {
			 cmdcontrol_process(queue_data);
			 free(queue_data);
		 }
	}
}

void shutdown_handler(void){
//	uart_write_bytes((uart_port_t)UART_NUM_0, (const void *)"ESP: RESTART", (size_t)strlen("ESP: RESTART"));
}













