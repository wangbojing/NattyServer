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



#ifndef __NATTY_MESSAGE_H__
#define __NATTY_MESSAGE_H__

#include <stdio.h>

//#include "NattyUserProtocol.h"

#define NATTY_RESULT_CODE_SUCCESS 					"200"		//Success
#define NATTY_RESULT_CODE_BUSY						"500"		//服务器繁忙
#define NATTY_RESULT_CODE_ERR_DB_EXCEPTION			"501"		//数据库操作异常


#define NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE 		"10001"		//设备不在线
#define NATTY_RESULT_CODE_ERR_JSON_FORMAT 			"10002"		//JSOn格式错误
#define NATTY_RESULT_CODE_ERR_JSON_CONVERT			"10003"		//JSON转换错误
#define NATTY_RESULT_CODE_ERR_JSON_DATA				"10004"		//JSON转数据错误
#define NATTY_RESULT_CODE_ERR_ICCID_NOTPHONENUM		"10005"		//JSON转换错误
#define NATTY_RESULT_CODE_ERR_DB_OPERATION			"10006"		
#define NATTY_RESULT_CODE_ERR_DB_NOEXIST			"10007"		
#define NATTY_RESULT_CODE_ERR_DB_SAVE_OFFLINE		"10008"		//保存离线数据错误

#define NATTY_RESULT_CODE_ERR_DB_SAVE_REPEATE_DATA	"10009"		//保存重复数据
#define NATTY_RESULT_CODE_ERR_DB_SEND_OFFLINE		"10010"		//获取离线数据错误

#define NATTY_RESULT_CODE_ERR_USER_NOTONLINE		"10011"		//UserId不存在
#define NATTY_RESULT_CODE_ERR_BIND_REPEATE_DATA		"10012"		//UserId与DeviceId已经绑定过
#define NATTY_RESULT_CODE_ERR_NOACTIVATE			"10013"		//设备没有激活
#define NATTY_RESULT_CODE_ERR_BROADCAST				"10014"		//广播数据
#define NATTY_RESULT_CODE_ERR_EFENCE_POINTS			"10015"		//电子围栏数据点数未在合理范围

#define NATTY_RESULT_CODE_ERR_REPEAT_CLIENTID		"10016"		//重复客户端ID
#define NATTY_RESULT_CODE_ERR_NOT_FIND_ADMIN 		"10018"		//没有找到管理员


#if 0
const char* ntyCommonResultMessage(const char *code);
#else
void ntyCommonResultMessage(const char *code, char *message);
#endif


#endif


