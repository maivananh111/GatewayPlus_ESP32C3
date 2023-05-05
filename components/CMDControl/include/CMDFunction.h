/*
 * CMDFunction.h
 *
 *  Created on: Apr 29, 2023
 *      Author: anh
 */

#ifndef COMPONENTS_CMDFUNCTION_H_
#define COMPONENTS_CMDFUNCTION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"

#include "sdkconfig.h"
#include "parse_packet.h"

#if CONFIG_CMD_SUPPORT_WIFI_CONTROL
void cmdf_isconnected(void);
void cmdf_get_ip(void);
void cmdf_wifi_scan(void);
void cmdf_wifi_connect(pkt_t *packet);
void cmdf_wifi_disconnect(void);
#endif /* CONFIG_CMD_SUPPORT_WIFI_CONTROL */

#if CONFIG_CMD_SUPPORT_HTTP_CLIENT
void cmdf_http_client_new(pkt_t *packet);
void cmdf_http_client_config(pkt_t *packet);
void cmdf_http_client_init(pkt_t *packet);
void cmdf_http_client_clean(pkt_t *packet);
void cmdf_http_client_set_header(pkt_t *packet);
void cmdf_http_client_set_url(pkt_t *packet);
void cmdf_http_client_set_method(pkt_t *packet);
void cmdf_http_client_set_data(pkt_t *packet);
void cmdf_http_client_request(pkt_t *packet);
#endif /* CONFIG_CMD_SUPPORT_HTTP_CLIENT */

#ifdef __cplusplus
}
#endif

#endif /* COMPONENTS_CMDFUNCTION_H_ */
