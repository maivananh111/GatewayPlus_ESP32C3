/*
 * WiFi.h
 *
 *  Created on: 1 thg 4, 2022
 *      Author: A315-56
 */

#ifndef COMPONENTS_WIFI_H_
#define COMPONENTS_WIFI_H_


#include "esp_netif.h"
#include "esp_wifi.h"

#ifdef __cplusplus
extern "C" {
#endif



typedef enum wifi_state{
	WIFI_CONNECT_FAILED = 0,
	WIFI_CONNECTED = 1,
}wifi_state_t;


void WiFi_STA_Connect(char *SSID, char *PASS, wifi_auth_mode_t auth);
void WiFi_STA_Disconnect(void);

wifi_state_t WiFi_GetState(void);
esp_netif_t *WiFi_STA_get_netif(void);

esp_err_t WiFi_STA_Set_IPV4(char *LocalIP, char *NetMask, char *DefaultGateWay);
esp_err_t WiFi_STA_Set_IPDHCP(void);
char *LocalIP(void);

uint8_t WiFi_STA_Scan(void);
char *WiFi_STA_Scan_Get_SSID(uint8_t Number);
int8_t WiFi_STA_Scan_Get_RSSI(uint8_t Number);



#ifdef __cplusplus
}
#endif

#endif /* COMPONENTS_WIFI_H_ */
