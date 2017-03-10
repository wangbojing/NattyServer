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
#include "NattyUserProtocol.h"

#define CONNECTION_SIZE_THRESHOLD_RATIO			0.6			
#define CONNECTION_SIZE_REAP_RATIO				0.1			



#define MYSQL_DB_CONN_STRING				"mysql://112.93.116.188:3306/NATTYDB?user=watch_server&password=123456"

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

#define NTY_DB_PHNUM_VALUE_SELECT_FORMAT	"CALL PROC_PHNUM_VALUE_RESULT(%lld, '%s')"

#define NTY_DB_DEVICE_STATUS_RESET_FORMAT	"CALL PROC_DEVICE_STATUS_RESET()"

#define NTY_DB_DEVICE_STATUS_SELECT_FORMAT	"CALL PROC_DEVICE_STATUS_SELECT(%lld)"

#define NTY_DB_APP_STATUS_SELECT_FORMAT		"CALL PROC_APP_STATUS_SELECT(%lld)"


#define NTY_DB_NAMES_UTF8_SET_FORMAT		"SET NAMES UTF8;"


/* ** **** ******** **************** Natty V3.2 **************** ******** **** ** */
#define NTY_DB_INSERT_EFENCE				"CALL PROC_INSERT_EFENCE(%lld, %d, '%s', '%s');"

#define NTY_DB_DELETE_EFENCE				"CALL PROC_DELETE_EFENCE(%lld, %d)"

#define NTY_DB_UPDATE_EFENCE				"CALL PROC_UPDATE_EFENCE(%lld, %d, %d, '%s','%s')"

#define NTY_DB_SELECT_EFENCE				"CALL PROC_SELECT_EFENCE(%lld)"


//#define NTY_DB_INSERT_TIMETABLE				"CALL PROC_INSERT_TIMETABLE(%lld)"

//#define NTY_DB_DELETE_TIMETABLE				"CALL PROC_DELETE_TIMETABLE(%lld)"
//上课时间 
#define NTY_DB_SELECT_TIMETABLE				"CALL PROC_SELECT_TIMETABLE(%lld)"

#define NTY_DB_UPDATE_TIMETABLE				"CALL PROC_UPDATE_TIMETABLE(%lld, '%s', %d, '%s', %d, '%s')"

//setup
#define NTY_DB_SELECT_SETUP					"CALL PROC_SELECT_SETUP(%lld)"

#define NTY_DB_UPDATE_SETUP					"CALL PROC_UPDATE_SETUP(%lld)"


//runtime
#define NTY_DB_UPDATE_RUNTIME				"CALL PROC_UPDATE_RUNTIME(%lld, %d, %d, %d, '%s', %d)"

#define NTY_DB_UPDATE_RUNTIME_AUTOCONN		"CALL PROC_UPDATE_RUNTIME_AUTOCONN(%lld, %d)"

#define NTY_DB_UPDATE_RUNTIME_LOSSREPORT	"CALL PROC_UPDATE_RUNTIME_LOSSREPORT(%lld, %d)"

#define NTY_DB_UPDATE_RUNTIME_LIGHTPANEL	"CALL PROC_UPDATE_RUNTIME_LIGHTPANEL(%lld, %d)"

#define NTY_DB_UPDATE_RUNTIME_BELL			"CALL PROC_UPDATE_RUNTIME_BELL(%lld, '%s')"

#define NTY_DB_UPDATE_RUNTIME_TARGETSTEP	"CALL PROC_UPDATE_RUNTIME_TARGETSTEP(%lld, %d)"

#define NTY_DB_SELECT_RUNTIME				"CALL PROC_SELECT_RUNTIME(%lld)"


//turn
#define NTY_DB_UPDATE_TURN					"CALL PROC_UPDATE_TURN(%lld, %d, '%s', '%s')"





//作息时间 ，闹钟 
#define NTY_DB_INSERT_SCHEDULE				"CALL PROC_INSERT_SCHEDULE(%lld, '%s', '%s', '%s')"

#define NTY_DB_DELETE_SCHEDULE				"CALL PROC_DELETE_SCHEDULE(%lld, %d)"

#define NTY_DB_UPDATE_SCHEDULE				"CALL PROC_UPDATE_SCHEDULE(%lld, %d, '%s', '%s', '%s')"

#define NTY_DB_SELECT_SCHEDULE				"CALL PROC_SELECT_SCHEDULE(%lld)"

#define NTY_DB_INSERT_PHONEBOOK				"CALL PROC_INSERT_PHONEBOOK(%lld, '%s', '%s')"

#define NTY_DB_DELETE_PHONEBOOK				"CALL PROCE_DELETE_PHONEBOOK(%lld, %d)"

#define NTY_DB_UPDATE_PHONEBOOK_ADMIN		"CALL PROC_UPDATE_PHONEBOOK_ADMIN(%lld, %d, %lld)"

#define NTY_DB_UPDATE_PHONEBOOK				"CALL PROC_UPDATE_PHONEBOOK(%lld, %d, '%s', '%s')"

#define NTY_DB_SELECT_PHONEBOOK				"CALL PROC_SELECT_PHONEBOOK(%lld)"

//activate 带返回结果
#define NTY_DB_UPDATE_DEVICE_ACTIVATE		"CALL PROC_UPDATE_DEVICE_ACTIVATE(%lld)"
//CLEAR 带返回结果 
#define NTY_DB_UPDATE_DEVICE_CLEAR			"CALL PROC_UPDATE_DEVICE_CLEAR(%lld)"


#define NTY_DB_INSERT_LOCATION 				"CALL PROC_INSERT_LOCATION(%lld, %d, '%s', '%s', '%s')"

#define NTY_DB_SELECT_LOCATION				"CALL PROC_SELECT_LOCATION(%lld)"

//绑定关系  带返回结果
#define NTY_DB_INSERT_GROUP					"CALL PROC_INSERT_GROUP(%lld, %lld, '%s')"
//解绑关系 
#define NTY_DB_DELETE_GROUP					"CALL PROC_DELETE_GROUP(%lld, %lld)"

//
#define NTY_DB_INSERT_COMMON_MSG			"CALL PROC_INSERT_COMMON_MSG(%lld, %lld, '%s')"

#define NTY_DB_DELETE_COMMON_OFFLINE_MSG	"CALL PROC_DELETE_COMMON_OFFLINE_MSG(%lld, %lld)"

#define NTY_DB_INSERT_VOICE_MSG				"CALL PROC_INSERT_VOICE_MSG(%lld, %lld, '%s')"

#define NTY_DB_DELETE_VOICE_OFFLINE_MSG		"CALL PROC_DELETE_VOICE_OFFLINE_MSG(%d, %lld)"


#define NTY_DB_SELECT_WATCHLIST_FROMAPPID 	"CALL PROC_SELECT_WATCHLIST_FROMAPPID(%lld)"









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



int ntyExecuteEfenceInsertHandle(C_DEVID aid, C_DEVID did, int num, U8 *points, U8 *runtime);

int ntyExecuteRuntimeUpdateHandle(C_DEVID aid, C_DEVID did, int auto_conn, U8 loss_report, U8 light_panel, const char *bell, int target_step);

int ntyExecuteRuntimeAutoConnUpdateHandle(C_DEVID aid, C_DEVID did, int runtime_param);

int ntyExecuteRuntimeLossReportUpdateHandle(C_DEVID aid, C_DEVID did, U8 runtime_param);

int ntyExecuteRuntimeLightPanelUpdateHandle(C_DEVID aid, C_DEVID did, U8 runtime_param);

int ntyExecuteRuntimeBellUpdateHandle(C_DEVID aid, C_DEVID did, const char *runtime_param);

int ntyExecuteRuntimeTargetStepUpdateHandle(C_DEVID aid, C_DEVID did, int runtime_param);

int ntyExecuteTurnUpdateHandle(C_DEVID aid, C_DEVID did, U8 status, const char *ontime, const char *offtime);

int ntyExecuteScheduleInsertHandle(C_DEVID aid, C_DEVID did, const char *daily, const char *time, const char *details);

int ntyExecuteScheduleDeleteHandle(C_DEVID aid, C_DEVID did, int id);

int ntyExecuteScheduleUpdateHandle(C_DEVID aid, C_DEVID did, int id, const char *daily, const char *time, const char *details);

int ntyExecuteScheduleSelectHandle(C_DEVID aid, C_DEVID did, ScheduleAck *pScheduleAck, size_t size);

int ntyExecuteTimeTablesUpdateHandle(C_DEVID aid, C_DEVID did, const char *morning, U8 morning_turn, const char *afternoon,  U8 afternoon_turn, const char *daily);

int ntyExecuteICCIDSelectHandle(C_DEVID aid, C_DEVID did);



int ntyQueryPhNumSelect(void *self, C_DEVID did, U8 *imei, U8 *phnum);


int ntyQueryAppOnlineStatusHandle(C_DEVID did, int *online);
int ntyQueryDeviceOnlineStatusHandle(C_DEVID did, int *online);





int ntyConnectionPoolInit(void);
void ntyConnectionPoolDeInit(void);





#endif




