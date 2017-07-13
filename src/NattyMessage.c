/*
 *  Author : luopeng , email : hibroad@hotmail.com
 * 
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of Author. (C) 2017
 * 
 *
 
****       *****
  ***        *
  ***        *                         *               *
  * **       *                         *               *
  * **       *                         *               *
  *  **      *                        **              **
  *  **      *                       ***             ***
  *   **     *       ******       ***********     ***********    *****    *****
  *   **     *     **     **          **              **           **      **
  *    **    *    **       **         **              **           **      *
  *    **    *    **       **         **              **            *      *
  *     **   *    **       **         **              **            **     *
  *     **   *            ***         **              **             *    *
  *      **  *       ***** **         **              **             **   *
  *      **  *     ***     **         **              **             **   *
  *       ** *    **       **         **              **              *  *
  *       ** *   **        **         **              **              ** *
  *        ***   **        **         **              **               * *
  *        ***   **        **         **     *        **     *         **
  *         **   **        **  *      **     *        **     *         **
  *         **    **     ****  *       **   *          **   *          *
*****        *     ******   ***         ****            ****           *
                                                                       *
                                                                      *
                                                                  *****
                                                                  ****


 *
 */


#include "NattyMessage.h"
#include <string.h>

#if 0
const char* ntyCommonResultMessage(const char *code) {
	char *message = NULL;
	if (code != NULL) {
		if (strcmp(code, NATTY_RESULT_CODE_SUCCESS) == 0) {
			message = "success";			//成功
		} else if (strcmp(code, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE) == 0) {
			message = "Device not online";	//设备不在线
		} else if (strcmp(code, NATTY_RESULT_CODE_ERR_JSON_FORMAT) == 0) {
			message = "JSON format error";	//JSON格式错误
		} else if (strcmp(code, NATTY_RESULT_CODE_ERR_JSON_CONVERT) == 0) {
			message = "JSON convert error";	//JSON转换错误
		} else if (strcmp(code, NATTY_RESULT_CODE_ERR_JSON_DATA) == 0) {
			message = "JSON data error";	//JSON数据错误
		} else if (strcmp(code, NATTY_RESULT_CODE_ERR_ICCID_NOTPHONENUM) == 0) {
			message = "ICCID not band phone number error";	//JSON转换错误
		} else if (strcmp(code, NATTY_RESULT_CODE_ERR_DB_SAVE_OFFLINE) == 0) {
			message = "Cannot save offline data";
		} else if (strcmp(code, NATTY_RESULT_CODE_ERR_DB_SAVE_REPEATE_DATA) == 0) {
			message = "Save Repeate data";
		} else if (strcmp(code, NATTY_RESULT_CODE_BUSY) == 0) {
			message = "Server be Busy";
		}
	}

	return message;
}
#else 
void ntyCommonResultMessage(const char *code, char *message) {
	if (code == NULL || message == NULL) {
		return;
	}
	if (strcmp(code, NATTY_RESULT_CODE_SUCCESS) == 0) {
		strcat(message, "Success");			//成功
	} else if (strcmp(code, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE) == 0) {
		strcat(message, "Device not online");	//设备不在线
	} else if (strcmp(code, NATTY_RESULT_CODE_ERR_JSON_FORMAT) == 0) {
		strcat(message, "JSON format error");	//JSON格式错误
	} else if (strcmp(code, NATTY_RESULT_CODE_ERR_JSON_CONVERT) == 0) {
		strcat(message, "JSON convert error");	//JSON转换错误
	} else if (strcmp(code, NATTY_RESULT_CODE_ERR_JSON_DATA) == 0) {
		strcat(message, "JSON data error");	//JSON数据错误
	} else if (strcmp(code, NATTY_RESULT_CODE_ERR_ICCID_NOTPHONENUM) == 0) {
		strcat(message, "ICCID not band phone number error");	//JSON转换错误
	} else if (strcmp(code, NATTY_RESULT_CODE_ERR_DB_SAVE_OFFLINE) == 0) {
		strcat(message, "Cannot save offline data");
	} else if (strcmp(code, NATTY_RESULT_CODE_ERR_DB_SAVE_REPEATE_DATA) == 0) {
		strcat(message, "Save Repeate data");
	} else if (strcmp(code, NATTY_RESULT_CODE_ERR_REPEAT_CLIENTID) == 0) {
		strcat(message, "Repeate Client Id");
	} else if (strcmp(code, NATTY_RESULT_CODE_ERR_NOT_FIND_ADMIN) == 0) {
		strcat(message, "Cannot find admin");
	} else if (strcmp(code, NATTY_RESULT_CODE_BUSY) == 0) {
		strcat(message, "Server be Busy");
	} else {
		strcat(message, "");
	}
}

#endif






