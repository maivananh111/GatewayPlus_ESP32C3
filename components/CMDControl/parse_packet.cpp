/*
 * parse_packet.cpp
 *
 *  Created on: Apr 29, 2023
 *      Author: anh
 */

#include "parse_packet.h"
#include "esp_log.h"

#include "stdlib.h"
#include "string.h"




static const char *TAG = "Parse packet";
static void parse_error_handler(char *str, int line, char *func){
	ESP_LOGE(TAG, "%s, Line %d Function %s", str, line, func);
}

pkt_err_t json_merge(char *src, char *dest, char *sub){
	pkt_err_t ret = PKT_ERR_OK;
	char *src_cpy = src;
	int key_len = 0, val_len = 0;
	char *pkstart, *pvstart;
	int ivstart = 0, ivend = 0;

	/** check input */
	if(src == NULL || dest == NULL || sub == NULL){
		parse_error_handler((char *)"Error bad input argument!", (int)__LINE__, (char *)__FUNCTION__);
		ret = PKT_ERR_ARG;
		return ret;
	}
	/** Find key */
	char *tmp;
	asprintf(&tmp, "\"%s\":", sub);
	pkstart = strstr(src_cpy, tmp);
	free(tmp);
	if(pkstart == NULL){
		parse_error_handler((char *)"Error key not appear in the input request string", (int)__LINE__, (char *)__FUNCTION__);
		ret = PKT_ERR_NOKEY;
		return ret;
	}
	pkstart++;

	for(key_len=0; key_len<strlen(pkstart); key_len++){
		if(pkstart[key_len] == '"') break;
	}

	/** Find Value start index */
	ivstart = (int)((pkstart - src_cpy) + key_len + 2);
	pvstart = pkstart;


	return ret;
}

pkt_err_t json_get_object(char *src, pkt_json_t *dest, char *key){
	pkt_err_t ret = PKT_ERR_OK;
	char *src_cpy = src;
	int src_len = strlen(src);
	int key_len = 0, val_len = 0;
	char *pkstart, *pvstart;
	int ivstart = 0, ivend = 0;

	/** check input */
	if(src == NULL || dest == NULL || key == NULL){
		parse_error_handler((char *)"Error bad input argument!", (int)__LINE__, (char *)__FUNCTION__);
		ret = PKT_ERR_ARG;
		return ret;
	}
	if(src[0] != '{' || src[src_len - 1] != '}' || src[src_len] != '\0'){
		parse_error_handler((char *)"Error input request string format!", (int)__LINE__, (char *)__FUNCTION__);
		ret = PKT_ERR_FORMAT;
		return ret;
	}

	/** Find key */
	char *tmp;
	asprintf(&tmp, "\"%s\":", key);
	pkstart = strstr(src_cpy, tmp);
	free(tmp);
	if(pkstart == NULL){
		parse_error_handler((char *)"Error key not appear in the input request string", (int)__LINE__, (char *)__FUNCTION__);
		ret = PKT_ERR_NOKEY;
		return ret;
	}
	pkstart++;

	/**
	 * Get key
	 * */
	for(key_len=0; key_len<strlen(pkstart); key_len++){
		if(pkstart[key_len] == '"') break;
	}

	dest->key = (char *)malloc((key_len+1) * sizeof(char));
	if(dest->key == NULL){
		parse_error_handler((char *)"Error can't allocation memory!", (int)__LINE__, (char *)__FUNCTION__);
		ret = PKT_ERR_MEM;
		return ret;
	}
	memcpy(dest->key, pkstart, key_len); 	/** assign key to json struct */
	dest->key[key_len] = '\0';

	/**
	 * Get value
	 * */
	/** Find Value start index */
	ivstart = (int)((pkstart - src_cpy) + key_len + 2);
	pvstart = pkstart;
	if((char)(*(uint32_t *)(pvstart + key_len + 2)) != '{') {
		dest->leaf = true;
	}

	/** Get start point of value */
	pvstart = (char *)(pvstart + key_len + 2);
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
			parse_error_handler((char *)"Error key no value!", (int)__LINE__, (char *)__FUNCTION__);
			ret = PKT_ERR_NOVAL;
			return ret;
		}
		dest->value = (char *)malloc((val_len+1) * sizeof(char));
		if(dest->value == NULL){
			parse_error_handler((char *)"Error can't allocation memory!", (int)__LINE__, (char *)__FUNCTION__);
			ret = PKT_ERR_MEM;
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
			parse_error_handler((char *)"Error can't allocation memory!", (int)__LINE__, (char *)__FUNCTION__);
			ret = PKT_ERR_MEM;
			return ret;
		}
		memcpy(dest->value, pvstart, val_len); 	/** assign key to jsn struct */
		dest->value[val_len] = '\0';
	}

	return ret;
}

pkt_err_t json_release_object(pkt_json_t *json){
	if(json->key != NULL) free(json->key);
	if(json->value != NULL) free(json->value);
	json->leaf = false;

	return PKT_ERR_OK;
}

pkt_err_t parse_packet(char *src, pkt_t *dest){
	pkt_err_t ret = PKT_ERR_OK;
	char *src_cpy = src;
	int data_len = 0, cmd_len = 0;
	char *pvstart;

	/** Get ": " */
	pvstart = strstr(src, ": ");
	if(pvstart == NULL){
		parse_error_handler((char *)"Error packet format!", (int)__LINE__, (char *)__FUNCTION__);
		ret = PKT_ERR_FORMAT;
		return ret;
	}

	/** Get command length */
	cmd_len = (int)(pvstart - src_cpy);

	/** Assign command string */
	dest->cmd_str = (char *)malloc((cmd_len + 1) * sizeof(char));
	if(dest->cmd_str == NULL){
		parse_error_handler((char *)"Error can't allocation memory!", (int)__LINE__, (char *)__FUNCTION__);
		ret = PKT_ERR_MEM;
		return ret;
	}
	memcpy(dest->cmd_str, src_cpy, cmd_len);
	dest->cmd_str[cmd_len] = '\0';


	/** Get data */
	pvstart = (char *)(pvstart + 2);
	data_len = strlen(pvstart);
	dest->data_str = (char *)malloc((data_len + 1) * sizeof(char));
	if(dest->data_str == NULL){
		parse_error_handler((char *)"Error can't allocation memory!", (int)__LINE__, (char *)__FUNCTION__);
		ret = PKT_ERR_MEM;
		return ret;
	}
	memcpy(dest->data_str, pvstart, data_len);
	dest->data_str[data_len] = '\0';

	return ret;
}

pkt_err_t release_packet(pkt_t *packet){
	if(packet->cmd_str != NULL) free(packet->cmd_str);
	if(packet->data_str != NULL) free(packet->data_str);

	return PKT_ERR_OK;
}

int str_to_cmd(char *str, const char *cmd_list[], int max){
	int cmd = 0;

	for(int i=0; i<(int)max; i++){
		if(strcmp(str, cmd_list[i]) == 0){
			cmd = i;
			return cmd;
		}
	}

	return cmd;
}

char *cmd_to_str(int cmd, const char *cmd_list[]){
	return (char *)cmd_list[cmd];
}

