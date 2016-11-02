/*
 *  Author : WangBoJing , email : 1989wangbojing@gmail.com
 * 
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of Author. (C) 2016
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


#ifndef __NATTY_DB_OPERATOR_H__
#define __NATTY_DB_OPERATOR_H__

#include <zdb.h>
#include "NattyAbstractClass.h"

#define CONNECTION_SIZE_THRESHOLD_RATIO			0.6			
#define CONNECTION_SIZE_REAP_RATIO				0.1			



#define MYSQL_DB_CONN_STRING				"mysql://112.93.116.188:3306/qjk?user=watch_server&password=123456"

#define NTY_DB_WATCH_INSERT_FORMAT			"CALL PROC_WATCH_INSERT('%s')"			//CALL PROC_WATCH_INSERT('352315052834187');

#define NTY_DB_APPIDLIST_SELECT_FORMAT 		"CALL PROC_APPIDLIST_DID_SELECT(%lld)" 		//CALL PROC_APPIDLIST_DID_SELECT(239307951255536007);

#define NTY_DB_WATCHIDLIST_SELECT_FORMAT	"CALL PROC_WATCHIDLIST_APPID_SELECT(%lld)"

//have return value
#define NTY_DB_DEV_APP_INSERT_FORMAT		"CALL PROC_DEV_APP_RELATION_REAULT(%lld, '%s')"		//CALL PROC_DEV_APP_RELATION_REAULT(10794, '352315052834187');

#define NTY_DB_DEV_APP_DELETE_FORMAT		"CALL PROC_DEV_APP_RELATION_DELETE(%lld, %lld)"

#define NTY_DB_LOCATION_INSERT_FORMAT		"CALL PROC_LOCATION_DID_INSERT(%lld, '%s', '%s', '%d','%s')"				//CALL PROC_LOCATION_DID_INSERT(239307951255536007, '112.4563212', '23.3456321', '')

#define NTY_DB_STEP_INSERT_FORMAT			"CALL PROC_STEP_DID_INSERT(%lld, %d)"

#define NTY_DB_HEARTRATE_INSERT_FORMAT		"CALL PROC_HEARTRATE_DID_INSERT(%lld, %d)"

#define NTY_DB_DEVICELOGIN_UPDATE_FORMAT	"CALL PROC_DEVICE_STATUS_UPDATE(%lld, 1)"

#define NTY_DB_DEVICELOGOUT_UPDATE_FORMAT	"CALL PROC_DEVICE_STATUS_UPDATE(%lld, 0)"

#define NTY_DB_APPLOGIN_UPDATE_FORMAT		"CALL PROC_APP_STATUS_UPDATE(%lld, 1)"

#define NTY_DB_APPLOGOUT_UPDATE_FORMAT		"CALL PROC_APP_STATUS_UPDATE(%lld, 0);"

#define NTY_DB_PHNUM_VALUE_SELECT_FORMAT	"CALL PROC_PHNUM_VALUE_RESULT(%lld, %s)"

#define NTY_DB_DEVICE_STATUS_RESET_FORMAT	"CALL PROC_DEVICE_STATUS_RESET()"

#define NTY_DB_DEVICE_STATUS_SELECT_FORMAT	"CALL PROC_DEVICE_STATUS_SELECT(%lld)"

#define NTY_DB_APP_STATUS_SELECT_FORMAT		"CALL PROC_APP_STATUS_SELECT(%lld)"


#define NTY_DB_NAMES_UTF8_SET_FORMAT		"SET NAMES UTF8;"

typedef struct _CONN_POOL {
	const void *_;
	ConnectionPool_T nPool;
	URL_T url;
} ConnectionPool;

typedef struct _CONNPOOLHANDLE {
	size_t size;
	void* (*ctor)(void *_self);
	void* (*dtor)(void *_self);
	int (*execute)(void *_self, U8 *sql);
	int (*query)(void *_self, U8 *sql, void **result, int *length);
} ConnectionPoolHandle;



int ntyExecuteWatchInsertHandle(U8 *imei);
int ntyQueryAppIDListSelectHandle(C_DEVID did, void *tree);
int ntyQueryWatchIDListSelectHandle(C_DEVID aid, void *tree);
int ntyQueryDevAppRelationInsertHandle(C_DEVID aid, U8 *imei);
int ntyExecuteDevAppRelationDeleteHandle(C_DEVID aid, C_DEVID did);
int ntyExecuteLocationInsertHandle(C_DEVID did, U8 *lng, U8 *lat, U8 type, U8 *info);
int ntyExecuteStepInsertHandle(C_DEVID did, int value);
int ntyExecuteHeartRateInsertHandle(C_DEVID did, int value);


int ntyExecuteDeviceLoginUpdateHandle(C_DEVID did);
int ntyExecuteDeviceLogoutUpdateHandle(C_DEVID did);

int ntyExecuteAppLoginUpdateHandle(C_DEVID aid);
int ntyExecuteAppLogoutUpdateHandle(C_DEVID aid);



int ntyQueryPhNumSelect(void *self, C_DEVID did, U8 *imei, U8 *phnum);


int ntyQueryAppOnlineStatusHandle(C_DEVID did, int *online);
int ntyQueryDeviceOnlineStatusHandle(C_DEVID did, int *online);





int ntyConnectionPoolInit(void);
void ntyConnectionPoolDeInit(void);





#endif




