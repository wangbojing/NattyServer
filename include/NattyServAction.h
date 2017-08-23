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



#ifndef __NATTY_SERVACTION_H__
#define __NATTY_SERVACTION_H__

#include "NattyAbstractClass.h"
#include "NattyJson.h"

typedef struct _ActionParam {
	C_DEVID fromId; 
	C_DEVID toId;
	JSON_Value *json; 
	char *jsonstring; 
	U16 jsonlen;
	int index;
} ActionParam;
//add by luoyb 2017-04-27
typedef struct _ClientActionParam{
	C_DEVID fromId;
	C_DEVID toId;
	char *jsonString;
	JSON_Value *jsonObj;	
}ClientActionParam;



void ntyJsonRecvResult(C_DEVID devId, char *jsonresult);

int ntyJsonBroadCastRecvResult(C_DEVID fromId, C_DEVID toId, char *jsonresult, U32 index);

void ntyJsonCommonResult(C_DEVID devId, const char *code);

void ntyJsonCommonContextResult(C_DEVID devId, const char *context);

void ntyJsonCommonExtendResult(C_DEVID devId, const char * code, int id);

void ntyCommonReqAction(ActionParam *pActionParam);

void ntyUserDataReqAction(ActionParam *pActionParam);

void ntyJsonAuthorizePushAction(ActionParam *pActionParam);

void ntyJsonAuthorizeReplyAction(ActionParam *pActionParam);

void ntyJsonAddEfenceAction(ActionParam *pActionParam);

void ntyJsonDelEfenceAction(ActionParam *pActionParam);

void ntyJsonRunTimeAction(ActionParam *pActionParam);

void ntyJsonTurnAction(ActionParam *pActionParam);

void ntyJsonICCIDSetAction(ActionParam *pActionParam);

void ntyJsonAddScheduleAction(ActionParam *pActionParam);

void ntyJsonDelScheduleAction(ActionParam *pActionParam);

void ntyJsonUpdateScheduleAction(ActionParam *pActionParam);

#if 0
void ntyJsonSelectScheduleAction(ActionParam *pActionParam);
#endif

void ntyJsonTimeTablesAction(ActionParam *pActionParam);

int ntyJsonLocationWIFIAction(ActionParam *pActionParam);

int ntyJsonLocationLabAction(ActionParam *pActionParam);

int ntyJsonWeatherAction(ActionParam *pActionParam);

void ntyJsonAddContactsAction(ActionParam *pActionParam);

void ntyJsonUpdateContactsAction(ActionParam *pActionParam);

void ntyJsonDelContactsAction(ActionParam *pActionParam);

void ntyJsonICCIDAction(ActionParam *pActionParam);

void ntyJsonOfflineMsgReqAction(ActionParam *pActionParam);

void ntyJsonAlarmAction(char *eventType, char *content);

void ntyJsonSOSReportAction(ActionParam *pActionParam);

void ntyJsonHeartReportAction(ActionParam *pActionParam);

void ntyJsonBloodReportAction(ActionParam *pActionParam);

void ntyJsonEfenceReportAction(ActionParam *pActionParam);

int ntyJsonFallDownReportAction(ActionParam *pActionParam);

int ntyJsonSafetyReportReportAction(ActionParam *pActionParam);

void ntyJsonLocationReportAction(ActionParam *pActionParam);

void ntyJsonStepsReportAction(ActionParam *pActionParam);

void ntyJsonWearStatusAction(ActionParam *pActionParam);

void ntyBindAgreeAction(char *imei, C_DEVID fromId, C_DEVID proposerId, C_DEVID toId, char *phonenum, U32 msgId);

int ntyVoiceDataReqAction(C_DEVID senderId, C_DEVID gId, char *filename);

int ntyVoiceReqAction(C_DEVID fromId, U32 msgId);

int ntyVoiceAckAction(C_DEVID fromId, U32 msgId);

int ntyBindReqAction(ActionParam *pActionParam);

int ntyOfflineAction(C_DEVID fromId, U32 msgId);

int ntyReadOfflineCommonMsgAction(C_DEVID devId);

int ntyReadOfflineVoiceMsgAction(C_DEVID devId);

int ntyReadOfflineBindMsgToAdminAction(C_DEVID devId);


//add by luoyb
int ntyClientSelectScheduleReqAction( ClientActionParam *clientActionParamVal,ClientSelectReq *ptrclientSelectReq );
int ntyClientSelectContactsReqAction( ClientActionParam *pClientActionParam,ClientSelectReq *pClientSelectReq );
int ntyClientSelectTurnReqAction( ClientActionParam *pClientActionParam,ClientSelectReq *pClientSelectReq );
int ntyClientSelectRunTimeReqAction( ClientActionParam *pClientActionParam,ClientSelectReq *pClientSelectReq );
int ntyClientSelectTimeTablesReqAction( ClientActionParam *pClientActionParam,ClientSelectReq *pClientSelectReq );
int ntyClientSelectLocationReqAction( ClientActionParam *pClientActionParam,ClientSelectReq *pClientSelectReq );
int ntyClientSelectEfenceReqAction( ClientActionParam *pClientActionParam,ClientSelectReq *pClientSelectReq );
int ntyClientSelectURLReqAction( ClientActionParam *pClientActionParam,ClientSelectReq *pClientSelectReq );
int ntyJsonResetAction( ActionParam *pActionParam );
int ntyJsonRestoreAction( ActionParam *pActionParam );
int ntyAppIdToDeviceIdDeleteCb( void *self, void *arg );
int ntyLocatorBindReqAction( ClientActionParam *pClientActionParam, LocatorBindReq *pLocatorBindReq );
int ntyLocatorUnBindReqAction( C_DEVID appId, C_DEVID devId );

#endif // __NATTY_SERVACTION_H__


