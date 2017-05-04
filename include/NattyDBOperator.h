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


#include "NattyConfig.h"
#include "NattyAbstractClass.h"
#include "NattyUserProtocol.h"


#if ENABLE_NTY_CONNECTION_POOL
#include "NattyConnectionPool.h"
#else
#include <zdb.h>
#endif

#define CONNECTION_SIZE_THRESHOLD_RATIO			0.6			
#define CONNECTION_SIZE_REAP_RATIO				0.1			


//#define MYSQL_DB_CONN_STRING				"mysql://192.168.1.178:3306/NATTYDB?user=juguikeji&password=qjk.123456"
#define MYSQL_DB_CONN_STRING				"mysql://112.93.116.188:3306/NATTYDB?user=watch_server&password=123456"

#define NTY_DB_WATCH_INSERT_FORMAT			"CALL PROC_WATCH_INSERT('%s')"			//CALL PROC_WATCH_INSERT('352315052834187');

#define NTY_DB_APPIDLIST_SELECT_FORMAT 		"CALL PROC_APPIDLIST_DID_SELECT(%lld)" 		//CALL PROC_APPIDLIST_DID_SELECT(239307951255536007);

#define NTY_DB_WATCHIDLIST_SELECT_FORMAT	"CALL PROC_WATCHIDLIST_APPID_SELECT(%lld)"

//have return value
#define NTY_DB_DEV_APP_INSERT_FORMAT		"CALL PROC_DEV_APP_RELATION_REAULT(%lld, '%s')"		//CALL PROC_DEV_APP_RELATION_REAULT(10794, '352315052834187');

#define NTY_DB_DEV_APP_DELETE_FORMAT		"CALL PROC_DEV_APP_RELATION_DELETE(%lld, %lld)"

#define NTY_DB_LOCATION_INSERT_FORMAT		"CALL PROC_LOCATION_DID_INSERT(%lld, '%s', '%s', '%d', '%s')"				//CALL PROC_LOCATION_DID_INSERT(239307951255536007, '112.4563212', '23.3456321', '')

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

#define NTY_DB_INSERT_EFENCE				"CALL PROC_INSERT_EFENCE(%lld, %d, %d, '%s', '%s');"

#define NTY_DB_DELETE_EFENCE				"CALL PROC_DELETE_EFENCE(%lld, %d)"

#define NTY_DB_UPDATE_EFENCE				"CALL PROC_UPDATE_EFENCE(%lld, %d, %d, '%s','%s')"

#define NTY_DB_SELECT_EFENCE				"CALL PROC_SELECT_EFENCE(%lld)"


#define NTY_DB_SELECT_ICCID					"CALL PROC_SELECT_ICCID('%s')"

#define NTY_DB_SET_ICCID					"CALL PROC_SET_ICCID(%lld, '%s', '%s')"


#define NTY_DB_PHNUM_VALUE_SELECT			"CALL PROC_PHNUM_VALUE_SELECT(%lld, '%s')"

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
#define NTY_DB_INSERT_SCHEDULE				"CALL PROC_INSERT_SCHEDULE(%lld, '%s', '%s', %d, '%s')"

#define NTY_DB_DELETE_SCHEDULE				"CALL PROC_DELETE_SCHEDULE(%lld, %d)"

#define NTY_DB_UPDATE_SCHEDULE				"CALL PROC_UPDATE_SCHEDULE(%lld, %d, '%s', '%s', %d, '%s')"

#define NTY_DB_SELECT_SCHEDULE				"CALL PROC_SELECT_SCHEDULE(%lld)"

#define NTY_DB_INSERT_PHONEBOOK				"CALL PROC_INSERT_PHONEBOOK(%lld, %lld, '%s', '%s', '%s')"

#define NTY_DB_DELETE_PHONEBOOK				"CALL PROC_DELETE_PHONEBOOK(%lld, %d)"

#define NTY_DB_UPDATE_PHONEBOOK_ADMIN		"CALL PROC_UPDATE_PHONEBOOK_ADMIN(%lld, %d, %lld)"

#define NTY_DB_UPDATE_PHONEBOOK				"CALL PROC_UPDATE_PHONEBOOK(%lld, %lld, '%s', '%s', '%s', %d)"

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


#define NTY_DB_INSERT_LOCATIONREPORT		"CALL PROC_INSERT_LOCATIONREPORT(%lld, %d, '%s', '%s', '%s')"

#define NTY_DB_INSERT_STEPSREPORT			"CALL PROC_INSERT_STEPSREPORT(%lld, %d)"

#define NTY_DB_INSERT_HEARTREPORT			"CALL PROC_INSERT_HEARTREPORT(%lld, %d)"


#define NTY_DB_INSERT_COMMON_MSG_REJECT		"CALL PROC_INSERT_COMMON_MSG_REJECT(%lld, %lld, '%s')"

#define NTY_DB_UPDATE_DEVICE_STATUS			"CALL PROC_UPDATE_DEVICE_STATUS(%lld, %d)"

#define NTY_DB_DELETE_COMMON_OFFLINE_MSG	"CALL PROC_DELETE_COMMON_OFFLINE_MSG(%d, %lld)"

#define NTY_DB_SELECT_COMMON_OFFLINE_MSG	"CALL PROC_SELECT_COMMON_OFFLINE_MSG(%lld)"

//#define NTY_DB_SELECT_COMMON_MSG			"CALL PROC_SELECT_COMMON_MSG(%lld)"

#define NTY_DB_INSERT_VOICE_MSG				"CALL PROC_INSERT_VOICE_MSG(%lld, %lld, '%s')"

#define NTY_DB_DELETE_VOICE_OFFLINE_MSG		"CALL PROC_DELETE_VOICE_OFFLINE_MSG(%d, %lld)"


#define NTY_DB_SELECT_WATCHLIST_FROMAPPID 	"CALL PROC_SELECT_WATCHLIST_FROMAPPID(%lld)"



#define NTY_DB_CHECK_GROUP					"CALL PROC_CHECK_GROUP(%lld, %lld)"

#define NTY_DB_SELECT_ADMIN					"CALL PROC_SELECT_ADMIN(%lld)"

#define NTY_DB_INSERT_VOICE_MSG				"CALL PROC_INSERT_VOICE_MSG(%lld, %lld, '%s')"

#define NTY_DB_DELETE_VOICE_OFFLINE_MSG		"CALL PROC_DELETE_VOICE_OFFLINE_MSG(%d, %lld)"

#define NTY_DB_SELECT_VOICE_MSG				"CALL PROC_SELECT_VOICE_MSG(%d)"

#define NTY_DB_INSERT_BIND_CONFIRM			"CALL PROC_INSERT_BIND_CONFIRM(%lld, %lld, '%s', '%s', %lld, '%s', '%s')"

#define NTY_DB_INSERT_BIND_GROUP			"CALL PROC_INSERT_BIND_GROUP(%d)"

#define NTY_DB_INSERT_BIND_AGREE			"CALL PROC_INSERT_BIND_AGREE(%d)"

#define NTY_DB_DELETE_BIND_GROUP			"CALL PROC_DELETE_BIND_GROUP(%d)"

#define NTY_DB_SELECT_PHONE_NUMBER			"CALL PROC_SELECT_PHONE_NUMBER(%lld, %lld)"

#define NTY_DB_INSERT_ADMIN_GROUP			"CALL PROC_INSERT_ADMIN_GROUP(%lld, '%s', %lld, '%s', '%s', '%s')"

#define NTY_DB_SELECT_VOICE_OFFLINE_MSG		"CALL PROC_SELECT_VOICE_OFFLINE_MSG(%lld)"

#define NTY_DB_SELECT_BIND_OFFLINE_MSG_TO_ADMIN		"CALL PROC_SELECT_BIND_OFFLINE_MSG_TO_ADMIN(%lld)"

#define NTY_DB_SELECT_BIND_OFFLINE_MSG_TO_PROPOSER	"CALL PROC_SELECT_BIND_OFFLINE_MSG_TO_PROPOSER(%lld)"

#define NTY_DB_INSERT_COMMON_ITEM_MSG		"CALL PROC_INSERT_COMMON_ITEM_MSG(%lld, %lld, '%s')"

#define NTY_DB_DELETE_COMMON_ITEM_MSG		"CALL PROC_DELETE_COMMON_ITEM_MSG(%d)"

#define NTY_DB_SELECT_PHONEBOOK_BINDAGREE	"CALL PROC_SELECT_PHONEBOOK_BINDAGREE(%lld, '%s', %lld)"



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
int ntyQueryDevAppGroupInsertHandle(C_DEVID aid, C_DEVID did);
int ntyExecuteDevAppGroupDeleteHandle(C_DEVID aid, C_DEVID did);
int ntyExecuteLocationInsertHandle(C_DEVID did, U8 *lng, U8 *lat, U8 type, U8 *info);
int ntyExecuteLocationNewInsertHandle( C_DEVID did, U8 type, const char *lnglat, const char *info, const char *desc);
int ntyExecuteStepInsertHandle(C_DEVID did, int value);
int ntyExecuteHeartRateInsertHandle(C_DEVID did, int value);


int ntyExecuteDeviceLoginUpdateHandle(C_DEVID did);
int ntyExecuteDeviceLogoutUpdateHandle(C_DEVID did);

int ntyExecuteAppLoginUpdateHandle(C_DEVID aid);
int ntyExecuteAppLogoutUpdateHandle(C_DEVID aid);



int ntyExecuteLocationReportInsertHandle(C_DEVID did, U8 type, const char *info, const char *lnglat, const char *detatils, int *msg);

int ntyExecuteStepsReportInsertHandle(C_DEVID did, int step, int *msg);

int ntyExecuteHeartReportInsertHandle(C_DEVID did, int heart, int *msg);

int ntyExecuteEfenceInsertHandle(C_DEVID aid, C_DEVID did, int index, int num, U8 *points, U8 *runtime, int *id);

int ntyExecuteEfenceDeleteHandle(C_DEVID aid, C_DEVID did, int index);

int ntyExecuteICCIDSelectHandle(C_DEVID did, const char *iccid, char *phonenum);

int ntyExecuteICCIDSetInsertHandle(C_DEVID did, const char *iccid, char *phonenum);

int ntyExecuteRuntimeUpdateHandle(C_DEVID aid, C_DEVID did, int auto_conn, U8 loss_report, U8 light_panel, const char *bell, int target_step);

int ntyExecuteRuntimeAutoConnUpdateHandle(C_DEVID aid, C_DEVID did, int runtime_param);

int ntyExecuteRuntimeLossReportUpdateHandle(C_DEVID aid, C_DEVID did, U8 runtime_param);

int ntyExecuteRuntimeLightPanelUpdateHandle(C_DEVID aid, C_DEVID did, U8 runtime_param);

int ntyExecuteRuntimeBellUpdateHandle(C_DEVID aid, C_DEVID did, const char *runtime_param);

int ntyExecuteRuntimeTargetStepUpdateHandle(C_DEVID aid, C_DEVID did, int runtime_param);

int ntyExecuteTurnUpdateHandle(C_DEVID aid, C_DEVID did, U8 status, const char *ontime, const char *offtime);

int ntyExecuteContactsInsertHandle(C_DEVID aid, C_DEVID did, Contacts *contacts, int *contactsId);

int ntyExecuteContactsUpdateHandle(C_DEVID aid, C_DEVID did, Contacts *contacts, int contactsId);

int ntyExecuteContactsDeleteHandle(C_DEVID aid, C_DEVID did, int contactsId);

int ntyExecuteScheduleInsertHandle(C_DEVID aid, C_DEVID did, const char *daily, const char *time, int status, const char *details, int *scheduleId);

int ntyExecuteScheduleDeleteHandle(C_DEVID aid, C_DEVID did, int scheduleId);

int ntyExecuteScheduleUpdateHandle(C_DEVID aid, C_DEVID did, int scheduleId, const char *daily, const char *time, int status, const char *details);

int ntyExecuteScheduleSelectHandle(C_DEVID aid, C_DEVID did, ScheduleAck *pScheduleAck, size_t size);

int ntyExecuteTimeTablesUpdateHandle(C_DEVID aid, C_DEVID did, const char *morning, U8 morning_turn, const char *afternoon,  U8 afternoon_turn, const char *daily, int *result);

int ntyExecuteCommonMsgInsertHandle(C_DEVID sid, C_DEVID gid, char *details, int *msg);

int ntyExecuteCommonItemMsgInsertHandle(C_DEVID sid, C_DEVID gid, char *details, int *msg);

int ntyExecuteCommonItemMsgDeleteHandle(int msgId);

int ntyQueryCommonMsgSelectHandle(C_DEVID msgId, C_DEVID *senderId, C_DEVID *groupId, char *json);

int ntyQueryPhNumSelect(void *self, C_DEVID did, U8 *imei, U8 *phnum);

int ntyQueryCommonOfflineMsgSelectHandle(C_DEVID deviceId, void *container);

int ntyQueryAppOnlineStatusHandle(C_DEVID did, int *online);
int ntyQueryDeviceOnlineStatusHandle(C_DEVID did, int *online);


int ntyQueryVoiceMsgInsertHandle(C_DEVID senderId, C_DEVID gId, char *filename, U32 *msgId);
int ntyExecuteVoiceOfflineMsgDeleteHandle(U32 index, C_DEVID userId);
int ntyQueryVoiceMsgSelectHandle(U32 index, C_DEVID *senderId, C_DEVID *gId, U8 *filename, long *stamp);

int ntyQueryAdminSelectHandle(C_DEVID did, C_DEVID *appid);

int ntyQueryDevAppGroupCheckSelectHandle(C_DEVID aid, C_DEVID did);

int ntyQueryBindConfirmInsertHandle(C_DEVID admin, C_DEVID imei, U8 *name, U8 *wimage, C_DEVID proposer, U8 *call, U8 *uimage, int *msgId);

int ntyQueryPhoneBookSelectHandle(C_DEVID imei, C_DEVID userId, char *phonenum);

int ntyExecuteDevAppGroupBindInsertHandle(int msgId, C_DEVID *proposerId, U8 *phonenum);

int ntyExecuteDevAppGroupBindAndAgreeInsertHandle(int msgId, C_DEVID *proposerId, U8 *phonenum, int *pid, U8 *pname, U8 *pimage);

int ntyExecuteCommonOfflineMsgDeleteHandle(int msgId, C_DEVID clientId);

int ntyQueryAdminGroupInsertHandle(C_DEVID devId, U8 *bname, C_DEVID fromId, U8 *userCall, U8 *wimage, U8 *uimage);

int ntyQueryVoiceOfflineMsgSelectHandle(C_DEVID fromId, void *container);

int ntyQueryPhonebookBindAgreeSelectHandle(C_DEVID did, C_DEVID proposerId, char *phonenum, int *pid, U8 *pname, U8 *pimage);

int ntyExecuteBindConfirmDeleteHandle(int msgId, C_DEVID *id);

int ntyQueryBindOfflineMsgToAdminSelectHandle(C_DEVID fromId, void *container);

int ntyQueryBindOfflineMsgToProposerSelectHandle(C_DEVID fromId, void *container);

int ntyExecuteCommonMsgToProposerInsertHandle(C_DEVID sid, C_DEVID gid, const char *detatils, int *msg);

int ntyExecuteChangeDeviceOnlineStatusHandle(C_DEVID did, int status);


int ntyConnectionPoolInit(void);
void ntyConnectionPoolDeInit(void);





#endif




