/*
 * parse_packet.h
 *
 *  Created on: Apr 29, 2023
 *      Author: anh
 */

#ifndef COMPONENTS_PARSE_PACKET_H_
#define COMPONENTS_PARSE_PACKET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "sdkconfig.h"

typedef enum{
	PKT_ERR_OK,
	PKT_ERR_ARG    = 0x00000001,
	PKT_ERR_FORMAT = 0x00000002,
	PKT_ERR_NOKEY  = 0x00000004,
	PKT_ERR_NOVAL  = 0x00000008,
	PKT_ERR_MEM    = 0x00000010,
} pkt_err_t;

typedef struct {
	char *key = NULL;
	bool leaf = false;
	char *value = NULL;
} pkt_json_t;

typedef struct{
	char *cmd_str = NULL;
	char *data_str = NULL;
} pkt_t;


pkt_err_t json_merge(char *src, char *dest, char *sub);
pkt_err_t json_get_object(char *src, pkt_json_t *dest, char *key);
pkt_err_t json_release_object(pkt_json_t *json);
pkt_err_t parse_packet(char *src, pkt_t *dest);
pkt_err_t release_packet(pkt_t *packet);
int str_to_cmd(char *str, const char *cmd_list[], int max);
char *cmd_to_str(int cmd, const char *cmd_list[]);


#ifdef __cplusplus
}
#endif

#endif /* COMPONENTS_PARSE_PACKET_H_ */
