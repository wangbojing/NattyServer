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
#include "../include/parson.h"
#include "NattyUserProtocol.h"


WeatherAck* ntyInitWeather();
void ntyReleaseWeather(WeatherAck *pWeatherAck);

ScheduleAck* ntyInitScheduleAck();
void ntyReleaseScheduleAck(ScheduleAck *pScheduleAck);

TimeTablesAck* ntyInitTimeTablesAck();
void ntyReleaseTimeTablesAck(TimeTablesAck *pTimeTablesAck);



JSON_Value* ntyMallocJsonByString(const char *jsonstring);

void ntyFreeJson(JSON_Value *json);

const char * ntyJsonAppCategory(JSON_Value *json);

const char * ntyJsonWatchCategory(JSON_Value *json);

const char * ntyJsonAction(JSON_Value *json);

void ntyJsonWIFI(JSON_Value *json, WIFIReq *pWIFIReq);

void ntyJsonLAB(JSON_Value *json, LABReq *pLABReq);

//void ntyJsonLocation(JSON_Value *json, LocationAck *pLocation);

void ntyJsonAMap(JSON_Value *json, AMap *pAMap);

//void ntyJsonWeather(JSON_Value *json, WeatherAck *pWeather);

void ntyJsonCommon(JSON_Value *json, CommonReq *pCommonReq);

void ntyJsonEfence(JSON_Value *json, EfenceReq *pEfenceReq);

void ntyJsonRuntime(JSON_Value *json, RunTimeReq *pRunTimeReq);

void ntyJsonTurn(JSON_Value *json, TurnReq *pTurnReq);

void ntyJsonAddSchedule(JSON_Value *json, AddScheduleReq *pAddScheduleReq);

void ntyJsonUpdateSchedule(JSON_Value *json, UpdateScheduleReq *pUpdateScheduleReq);

void ntyJsonDelSchedule(JSON_Value *json, DelScheduleReq *pDelScheduleReq);

void ntyJsonTimeTables(JSON_Value *json, TimeTablesReq *pTimeTablesReq);


char * ntyJsonWriteConfig(ConfigAck *pConfigAck);

char * ntyJsonWritePower(PowerAck *pPowerAck);

char * ntyJsonWriteSignal(SignalAck *pSignalAck);

char * ntyJsonWriteLocation(LocationAck *pLocationAck);

char * ntyJsonWriteWeather(WeatherAck *pWeatherAck);

char * ntyJsonWriteRunTime(RunTimeAck *pRunTimeAck);

char * ntyJsonWriteSchedule(ScheduleAck *pScheduleAck);

char * ntyJsonWriteTimeTables(TimeTablesAck *pTimeTablesAck);



#endif


