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
#include "../include/parson.h"
#include "NattyUserProtocol.h"


#define HTTP_QJK_BASE_URL		"http://app.quanjiakan.com"
#define HTTP_GAODE_BASE_URL		"http://apilocate.amap.com"


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


void ntyJsonWIFI(JSON_Value *json, WIFIReq *pWIFIReq);

void ntyJsonWIFIItemRelease(WIFIItem *pWIFI);

void ntyJsonLAB(JSON_Value *json, LABReq *pLABReq);

void ntyJsonLABItemRelease(Nearbts *pNearbts);

//void ntyJsonLocation(JSON_Value *json, LocationAck *pLocation);

void ntyJsonAMap(JSON_Value *json, AMap *pAMap);

void ntyJsonWeather(JSON_Value *json, WeatherReq *pWeatherReq);

void ntyJsonICCID(JSON_Value *json, ICCIDReq *pICCIDReq);

void ntyJsonCommon(JSON_Value *json, CommonReq *pCommonReq);

void ntyJsonEfence(JSON_Value *json, EfenceReq *pEfenceReq);

void ntyJsonEfencePointRelease(EfencePoints *pPoints);

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

char * ntyJsonWriteCommon(CommonAck *pCommonAck);

char * ntyJsonWriteConfig(ConfigAck *pConfigAck);

char * ntyJsonWritePower(PowerAck *pPowerAck);

char * ntyJsonWriteSignal(SignalAck *pSignalAck);

char * ntyJsonWriteWIFI(WIFIAck *pWIFIAck);

char * ntyJsonWriteLAB(LABAck *pLABAck);

char * ntyJsonWriteLocation(LocationAck *pLocationAck);

char * ntyJsonWriteWeather(WeatherAck *pWeatherAck);

char * ntyJsonWriteEfence(EfenceAck *pEfenceAck);

char * ntyJsonWriteRunTime(RunTimeAck *pRunTimeAck);

char * ntyJsonWriteSchedule(ScheduleAck *pScheduleAck);

char * ntyJsonWriteTimeTables(TimeTablesAck *pTimeTablesAck);

char * ntyJsonWriteContacts(ContactsAck *pContactsAck);


#endif


