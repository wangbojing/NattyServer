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



#ifndef __NATTY_JSON_H__
#define __NATTY_JSON_H__

#include <stdio.h>
#include <stdlib.h>

#include "parson.h"
#include "NattyUserProtocol.h"


#define HTTP_QJK_BASE_URL		"http://app.quanjiakan.com"
#define HTTP_GAODE_BASE_URL		"http://apilocate.amap.com"



typedef enum {
	NTY_CATEGORY_START = 0,
	NTY_CATEGORY_EFENCE = NTY_CATEGORY_START,
	NTY_CATEGORY_RUNTIME,
	NTY_CATEGORY_TURN,
	NTY_CATEGORY_SCHEDULE,
	NTY_CATEGORY_TIMETABLES,
	NTY_CATEGORY_CONTACTS,
	NTY_CATEGORY_SOSREPORT,
	NTY_CATEGORY_LOCATIONREPORT,
	NTY_CATEGORY_STEPSREPORT,
	NTY_CATEGORY_EFENCEREPORT,
	NTY_CATEGORY_WEARSTATUS,
	NTY_CATEGORY_END = NTY_CATEGORY_WEARSTATUS,
	NTY_CATEGORY_VOICE,
	NTY_CATEGORY_BINDCONFIRM,
} EN_CATEGORY;

int ntyCommonJsonCategory(char *json, int length);

void ntyCopyString(char **dest, const char *src, size_t size);


void ntyJsonSetLocationType(const char *locationType, int *u8LocationType);
void ntyJsonGetLocationType(const int u8LocationType, char* locationType);


WeatherAck* ntyInitWeather();
void ntyReleaseWeather(WeatherAck *pWeatherAck);

ScheduleAck* ntyInitScheduleAck();
void ntyReleaseScheduleAck(ScheduleAck *pScheduleAck);

TimeTablesAck* ntyInitTimeTablesAck();
void ntyReleaseTimeTablesAck(TimeTablesAck *pTimeTablesAck);

JSON_Value* ntyMallocJsonValue(const char *jsonstring);

void ntyFreeJsonValue(JSON_Value *json);

void ntyJsonFree(void *obj);

const char * ntyJsonDeviceIMEI(JSON_Value *json);

const char * ntyJsonAppCategory(JSON_Value *json);

const char * ntyJsonWatchCategory(JSON_Value *json);

const char * ntyJsonAction(JSON_Value *json);

void ntyJsonAMap(JSON_Value *json, AMap *pAMap);
void ntyJsonAMapGetAddress(JSON_Value *json, AMap *pAMap);


void ntyJsonWIFI(JSON_Value *json, WIFIReq *pWIFIReq);

void ntyJsonWIFIItemRelease(WIFIItem *pWIFI);

void ntyJsonLAB(JSON_Value *json, LABReq *pLABReq);

void ntyJsonLABItemRelease(Nearbts *pNearbts);

void ntyJsonWeatherLocation(JSON_Value *json, WeatherLocationReq *pWeatherLocationReq);

void ntyJsonWeather(JSON_Value *json, WeatherReq *pWeatherReq);

void ntyJsonWeatherRelease(WeatherReq *pWeatherReq);

void ntyJsonICCID(JSON_Value *json, ICCIDReq *pICCIDReq);

void ntyJsonCommon(JSON_Value *json, CommonReq *pCommonReq);

void ntyJsonCommonResponse(JSON_Value *json, CommonResponse *pCommonResponse);

//add by luoby. ntyClientReqJsonParse

int ntyClientReqJsonParse( JSON_Value *json, ClientSelectReq *pclientSelectReq );
int ntyClientContactsAckJsonCompose( ClientContactsAck *pClientContactsAck, unsigned char *buffer);
int ntyClientTurnAckJsonCompose( ClientTurnAck *pClientTurnAck, unsigned char *buffer);

int ntyClientRunTimeAckJsonCompose( ClientRunTimeAck *pClientRunTimeAck, unsigned char *buffer );
int ntyClientTimeTablesAckJsonCompose( ClientTimeTablesAck *pClientTimeTablesAck, unsigned char *buffer );
int ntyClientLocationAckJsonCompose( ClientLocationAck *pClientLocationAck, unsigned char *buffer );

char *ntyClientEfenceAckJsonCompose( ClientEfenceAck *pClientEfenceAck );
int ntyClientURLAckJsonCompose( ClientURLAck *pClientURLAck , unsigned char *buffer);
int ntyLocatorBindReqJsonParse( JSON_Value *json, LocatorBindReq *ptrLocatorBindReq );


//end

void ntyJsonCommonExtend(JSON_Value *json, CommonReqExtend *pCommonReqExtend);

void ntyJsonAuthorizePush(JSON_Value *json, AuthorizePush *pAuthorizePush);

void ntyJsonAuthorizeReply(JSON_Value *json, AuthorizeReply *pAuthorizeReply);

void ntyJsonAddEfence(JSON_Value *json, AddEfenceReq *pAddEfenceReq);

void ntyJsonAddEfencePointRelease(EfencePoints *pPoints);

void ntyJsonDelEfence(JSON_Value *json, DelEfenceReq *pDelEfenceReq);

void ntyJsonRuntime(JSON_Value *json, RunTimeReq *pRunTimeReq);

void ntyJsonTurn(JSON_Value *json, TurnReq *pTurnReq);

void ntyJsonAddSchedule(JSON_Value *json, AddScheduleReq *pAddScheduleReq);

void ntyJsonUpdateSchedule(JSON_Value *json, UpdateScheduleReq *pUpdateScheduleReq);

void ntyJsonDelSchedule(JSON_Value *json, DelScheduleReq *pDelScheduleReq);

void ntyJsonTimeTables(JSON_Value *json, TimeTablesReq *pTimeTablesReq);

void ntyJsonTimeTablesItemRelease(TimeTablesItem *pTimeTables);

void ntyJsonAddContacts(JSON_Value *json, AddContactsReq *pAddContactsReq);

void ntyJsonUpdateContacts(JSON_Value *json, UpdateContactsReq *pUpdateContactsReq);

void ntyJsonDelContacts(JSON_Value *json, DelContactsReq *pDelContactsReq);

void ntyJsonLocationReport(JSON_Value *json,  LocationReport *pLocationReport);

void ntyJsonStepsReport(JSON_Value *json,  StepsReport *pStepsReport);

void ntyJsonHeartReport(JSON_Value *json,  HeartReport *pHeartReport);

void ntyJsonFalldown(JSON_Value *json,  Falldown *pFalldown);


char * ntyJsonBind(JSON_Value *json, BindReq *pBindReq);

char * ntyJsonWriteBindConfirmReq(BindConfirmReq *pBindConfirmReq);

char * ntyJsonWriteICCID(ICCIDAck *pICCIDAck);

char * ntyJsonWriteCommon(CommonAck *pCommonAck);

char * ntyJsonWriteCommonExtend(CommonExtendAck *pCommonExtendAck);

char * ntyJsonWriteCommonReqExtend(CommonReqExtend *pCommonReqExtend);

char * ntyJsonWriteConfig(ConfigAck *pConfigAck);

char * ntyJsonWritePower(PowerAck *pPowerAck);

char * ntyJsonWriteSignal(SignalAck *pSignalAck);

char * ntyJsonWriteWIFI(WIFIAck *pWIFIAck);

char * ntyJsonWriteLAB(LABAck *pLABAck);

char * ntyJsonWriteLocation(LocationAck *pLocationAck);

char * ntyJsonWriteWeather(WeatherAck *pWeatherAck);

char * ntyJsonWriteAddEfence(AddEfenceAck *pAddEfenceAck);

char * ntyJsonWriteDelEfence(DelEfenceAck *pDelEfenceAck);

char * ntyJsonWriteRunTime(RunTimeAck *pRunTimeAck);

char * ntyJsonWriteTurn(TurnAck *pTurnAck);

char * ntyJsonWriteAddSchedule(AddScheduleAck *pAddScheduleAck);

char * ntyJsonWriteDeviceAddSchedule(DeviceAddScheduleAck *pDeviceAddScheduleAck);

char * ntyJsonWriteDelSchedule(DelScheduleAck *pDelScheduleAck);

char * ntyJsonWriteUpdateSchedule(UpdateScheduleAck *pUpdateScheduleAck);

int ntyJsonWriteSchedule(ScheduleAck *pScheduleAck, unsigned char *buffer);

char * ntyJsonWriteTimeTables(TimeTablesAck *pTimeTablesAck);

char * ntyJsonWriteAddContacts(AddContactsAck *pAddContactsAck);

//char * ntyJsonWriteWatchAddContacts(AddContactsAck *pAddContactsAck);

char * ntyJsonWriteDeviceAddContacts(DeviceAddContactsAck *pDeviceAddContactsAck);

char * ntyJsonWriteUpdateContacts(UpdateContactsAck *pUpdateContactsAck);

char * ntyJsonWriteDelContacts(DelContactsAck *pDelContactsAck);

char * ntyJsonWriteDeviceDelContacts(DeviceDelContactsAck *pDeviceDelContactsAck);

char * ntyJsonWriteContacts(ContactsAck *pContactsAck);

char * ntyJsonWriteBindConfirmPush(BindConfirmPush *pBindConfirmPush);

char * ntyJsonWriteBindBroadCast(BindBroadCast *pBindBroadCast);

char * ntyJsonWriteBindConfirmAck(BindConfirmAck *pBindConfirmAck);

char * ntyJsonWriteBindOfflineMsgToAdmin(BindOfflineMsgToAdmin *pMsgToAdmin, char *phonenum);

char * ntyJsonWriteBindOfflineMsgToProposer(BindOfflineMsgToProposer *pMsgToProposer);

char * ntyJsonWriteBindAgree(BindAgreeAck *pBindAgreeAck);

#endif


