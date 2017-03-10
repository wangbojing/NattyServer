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

#include "NattyJson.h"

#include <stdlib.h>
#include <string.h>
//#include <stdio.h>

#include "NattyAbstractClass.h"



#if 0
typedef unsigned char U8;
typedef unsigned short U16;
#endif


WeatherAck* ntyInitWeather() {
	WeatherAck *pWeatherAck = malloc(sizeof(WeatherAck));
	return pWeatherAck;
}

void ntyReleaseWeather(WeatherAck *pWeatherAck) {
	if (pWeatherAck != NULL) {
		if (pWeatherAck->pResults != NULL) {
			size_t i;
			for (i=0; i<pWeatherAck->size; i++) {
				if (pWeatherAck->pResults[i].pDaily != NULL) {
					free(pWeatherAck->pResults[i].pDaily);
				}
			}
			free(pWeatherAck->pResults);
		}
		free(pWeatherAck);
	}
}

ScheduleAck* ntyInitScheduleAck() {
	ScheduleAck *pScheduleAck = malloc(sizeof(ScheduleAck));
	return pScheduleAck;
}

void ntyReleaseScheduleAck(ScheduleAck *pScheduleAck) {
	if (pScheduleAck != NULL) {
		if (pScheduleAck->results.pSchedule != NULL) {
			free(pScheduleAck->results.pSchedule);
		}
		free(pScheduleAck);
	}
}


TimeTablesAck* ntyInitTimeTablesAck() {
	TimeTablesAck *pTimeTablesAck = malloc(sizeof(TimeTablesAck));
	return pTimeTablesAck;
}

void ntyReleaseTimeTablesAck(TimeTablesAck *pTimeTablesAck) {
	if (pTimeTablesAck != NULL) {
		if (pTimeTablesAck->results.pTimeTables != NULL) {
			free(pTimeTablesAck->results.pTimeTables);
		}
		free(pTimeTablesAck);
	}
}

JSON_Value* ntyMallocJsonValue(const char *jsonstring) {
	if (jsonstring == NULL) {
		return NULL;
	}
	
	JSON_Value *schema = json_parse_string(jsonstring);
	return schema;
}

void ntyFreeJsonValue(JSON_Value *json) {
	if (json == NULL) {
		return;
	}
	
	json_value_free(json);
}

void ntyJsonFree(void *obj) {
	json_free_serialized_string(obj);
}


const char * ntyJsonDeviceIMEI(JSON_Value *json) {
	if (json == NULL) {
		return NULL;
	}
	JSON_Object *root_object = json_value_get_object(json);
	const char *IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	return IMEI;
}

const char * ntyJsonAppCategory(JSON_Value *json) {
	if (json == NULL) {
		return NULL;
	}
	JSON_Object *root_object = json_value_get_object(json);
	const char *category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	return category;
}

const char * ntyJsonWatchCategory(JSON_Value *json) {
	if (json == NULL) {
		return NULL;
	}
	JSON_Object *root_object = json_value_get_object(json);
	JSON_Object *results_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL_RESULTS);
	const char *category = json_object_get_string(results_object, NATTY_USER_PROTOCOL_CATEGORY);
	return category;
}

const char * ntyJsonAction(JSON_Value *json) {
	if (json == NULL) {
		return NULL;
	}
	JSON_Object *root_object = json_value_get_object(json);
	const char *action = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ACTION);
	return action;
}

void ntyJsonWIFI(JSON_Value *json, WIFIReq *pWIFIReq) {
	if (json == NULL || pWIFIReq == NULL) {
		ntydbg("param is null.\n");
		return;
	}
	
	JSON_Object *root_object = json_value_get_object(json);
	pWIFIReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pWIFIReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	ntydbg("IMEI:%s   Category:%s\n", pWIFIReq->IMEI, pWIFIReq->category);
	
	JSON_Object *wifi_object = NULL;
	JSON_Array *wifi_array = json_object_get_array(root_object, NATTY_USER_PROTOCOL_WIFI);
	size_t wifi_count = json_array_get_count(wifi_array);
	WIFIItem *pWIFI = malloc(sizeof(WIFIItem)*wifi_count);
	pWIFIReq->pWIFI = pWIFI;

	size_t i;
	for (i = 0; i < wifi_count; i++) {
		wifi_object = json_array_get_object(wifi_array, i);
		pWIFI[i].SSID = json_object_get_string(wifi_object, NATTY_USER_PROTOCOL_SSID);
		pWIFI[i].MAC = json_object_get_string(wifi_object, NATTY_USER_PROTOCOL_MAC);
		pWIFI[i].V = json_object_get_string(wifi_object, NATTY_USER_PROTOCOL_V);
		ntydbg("SSID:%s   MAC:%s   V:%s\n", pWIFI[i].SSID, pWIFI[i].MAC, pWIFI[i].V);
	}
}

void ntyJsonWIFIItemRelease(WIFIItem *pWIFI) {
	if (pWIFI != NULL) {
		free(pWIFI);
	}
}


void ntyJsonLAB(JSON_Value *json, LABReq *pLABReq) {
	if (json == NULL || pLABReq == NULL) {
		ntydbg("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pLABReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pLABReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	ntydbg("IMEI:%s   Category:%s\n", pLABReq->IMEI, pLABReq->category);

	JSON_Object *lab_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL_LAB);
	pLABReq->lab.bts = json_object_get_string(lab_object, NATTY_USER_PROTOCOL_BTS);
	ntydbg("bts:%s\n", pLABReq->lab.bts);

	JSON_Object *nearbts_object = NULL;
	JSON_Array *nearbts_array = json_object_get_array(lab_object, NATTY_USER_PROTOCOL_NEARBTS);
	size_t nearbts_count = json_array_get_count(nearbts_array);
	Nearbts *pNearbts = malloc(sizeof(Nearbts)*nearbts_count);
	pLABReq->lab.pNearbts = pNearbts;
	size_t i;
	for (i = 0; i < nearbts_count; i++) {
		nearbts_object = json_array_get_object(nearbts_array, i);
		pNearbts[i].cell = json_object_get_string(nearbts_object, NATTY_USER_PROTOCOL_CELL);
		pNearbts[i].signal = json_object_get_string(nearbts_object, NATTY_USER_PROTOCOL_SIGNAL);
		ntydbg("cell:%s   signal:%s\n", pNearbts[i].cell, pNearbts[i].signal);
	}
}

void ntyJsonLABItemRelease(Nearbts *pNearbts) {
	if (pNearbts != NULL) {
		free(pNearbts);
	}
}


/*
void ntyJsonLocation(JSON_Value *json, LocationAck *pLocation) {
	if (json == NULL || pLocation == NULL) {
		ntydbg("param is null.\n");
		return;
	}
	
	JSON_Object *root_object = json_value_get_object(json);
	JSON_Object *results_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL_RESULTS);
	pLocation->results.IMEI = json_object_get_string(results_object, NATTY_USER_PROTOCOL_IMEI);
	pLocation->results.category = json_object_get_string(results_object, NATTY_USER_PROTOCOL_CATEGORY);
	pLocation->results.type = json_object_get_string(results_object, NATTY_USER_PROTOCOL_TYPE);
	pLocation->results.radius = json_object_get_string(results_object, NATTY_USER_PROTOCOL_RADIUS);
	pLocation->results.location = json_object_get_string(results_object, NATTY_USER_PROTOCOL_LOCATION);
	
	ntydbg("type:%s   radius:%s   location:%s\n", pLocation->results.type, pLocation->results.radius, pLocation->results.location);
	json_value_free(json);
}
*/

void ntyJsonAMap(JSON_Value *json, AMap *pAMap) {
	if (json == NULL || pAMap == NULL) {
		ntydbg("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pAMap->status = json_object_get_string(root_object, NATTY_USER_PROTOCOL_STATUS);
	pAMap->info = json_object_get_string(root_object, NATTY_USER_PROTOCOL_INFO);
	pAMap->infocode = json_object_get_string(root_object, NATTY_USER_PROTOCOL_INFOCODE);
	
	JSON_Object *result_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL_RESULT);
	pAMap->result.type = json_object_get_string(result_object, NATTY_USER_PROTOCOL_TYPE);
	pAMap->result.radius = json_object_get_string(result_object, NATTY_USER_PROTOCOL_RADIUS);
	pAMap->result.location = json_object_get_string(result_object, NATTY_USER_PROTOCOL_LOCATION);
	pAMap->result.desc = json_object_get_string(result_object, NATTY_USER_PROTOCOL_DESC);
	pAMap->result.country = json_object_get_string(result_object, NATTY_USER_PROTOCOL_COUNTRY);
	pAMap->result.city = json_object_get_string(result_object, NATTY_USER_PROTOCOL_CITY);
	pAMap->result.citycode = json_object_get_string(result_object, NATTY_USER_PROTOCOL_CITYCODE);
	pAMap->result.adcode= json_object_get_string(result_object, NATTY_USER_PROTOCOL_ADCODE);
	pAMap->result.road = json_object_get_string(result_object, NATTY_USER_PROTOCOL_ROAD);
	pAMap->result.street = json_object_get_string(result_object, NATTY_USER_PROTOCOL_STREET);
	pAMap->result.poi = json_object_get_string(result_object, NATTY_USER_PROTOCOL_POI);
	
	ntydbg("status:%s,  info:%s,  infocode:%s,  location:%s\n", pAMap->status, pAMap->info, pAMap->infocode, pAMap->result.location);
}

void ntyJsonWeather(JSON_Value *json, WeatherReq *pWeatherReq) {
	if (json == NULL || pWeatherReq == NULL) {
		ntydbg("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pWeatherReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pWeatherReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	pWeatherReq->bts = json_object_get_string(root_object, NATTY_USER_PROTOCOL_BTS);
	
	ntydbg("IMEI:%s,  category:%s,  bts:%s\n", pWeatherReq->IMEI, pWeatherReq->category, pWeatherReq->bts);
}

void ntyJsonICCID(JSON_Value *json, ICCIDReq *pICCIDReq) {
	if (json == NULL || pICCIDReq == NULL) {
			ntydbg("param is null.\n");
			return;
	}
	
	JSON_Object *root_object = json_value_get_object(json);
	pICCIDReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	
	ntydbg("IMEI:%s\n", pICCIDReq->IMEI);
}

void ntyJsonCommon(JSON_Value *json, CommonReq *pCommonReq) {
	if (json == NULL || pCommonReq == NULL) {
		ntydbg("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pCommonReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pCommonReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	pCommonReq->action = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ACTION);
}


void ntyJsonEfence(JSON_Value *json, EfenceReq *pEfenceReq) {
	if (json == NULL || pEfenceReq == NULL) {
		ntydbg("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pEfenceReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pEfenceReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	
	JSON_Object *efence_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL_EFENCE);
	pEfenceReq->efence.num = json_object_get_string(efence_object, NATTY_USER_PROTOCOL_NUM);

	JSON_Object *point_object = NULL;
	JSON_Array *points_array = json_object_get_array(efence_object, NATTY_USER_PROTOCOL_POINTS);
	size_t points_count = json_array_get_count(points_array);
	EfencePoints *pPoints = malloc(sizeof(EfencePoints)*points_count);
	pEfenceReq->efence.pPoints = pPoints;
	pEfenceReq->efence.size = points_count;
	
	size_t i;
	for (i = 0; i < points_count; i++) {
		pPoints[i].point = json_array_get_string(points_array, i);
		ntydbg("point:%s\n", pPoints[i].point);
	}
}

void ntyJsonEfencePointRelease(EfencePoints *pPoints) {
	if (pPoints != NULL) {
		free(pPoints);
	}
}


void ntyJsonRuntime(JSON_Value *json, RunTimeReq *pRunTimeReq) {
	if (json == NULL || pRunTimeReq == NULL) {
		ntydbg("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pRunTimeReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pRunTimeReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	ntydbg("IMEI:%s   Category:%s\n", pRunTimeReq->IMEI, pRunTimeReq->category);
	
	JSON_Object *runtime_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL_RUNTIME);
	pRunTimeReq->runtime.auto_connection = json_object_get_string(runtime_object, NATTY_USER_PROTOCOL_AUTOCONNECTION);
	pRunTimeReq->runtime.loss_report = json_object_get_string(runtime_object, NATTY_USER_PROTOCOL_LOSSREPORT);
	pRunTimeReq->runtime.light_panel = json_object_get_string(runtime_object, NATTY_USER_PROTOCOL_LIGHTPANEL);
	pRunTimeReq->runtime.watch_bell = json_object_get_string(runtime_object, NATTY_USER_PROTOCOL_WATCHBELL);
	pRunTimeReq->runtime.taget_step = json_object_get_string(runtime_object, NATTY_USER_PROTOCOL_TAGETSTEP);
	
	ntydbg("AutoConnection:%s  LossReport:%s  LightPanel:%s  WatchBell:%s  TagetStep:%s\n",
		pRunTimeReq->runtime.auto_connection,
		pRunTimeReq->runtime.loss_report,
		pRunTimeReq->runtime.light_panel,
		pRunTimeReq->runtime.watch_bell,
		pRunTimeReq->runtime.taget_step);
}

void ntyJsonTurn(JSON_Value *json, TurnReq *pTurnReq) {
	if (json == NULL || pTurnReq == NULL) {
		ntydbg("param is null.\n");
		return;
	}

	ntydbg("====================begin ntyJsonTurn ==========================\n");
	
	JSON_Object *root_object = json_value_get_object(json);
	pTurnReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pTurnReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	ntydbg("IMEI:%s   Category:%s\n", pTurnReq->IMEI, pTurnReq->category);
	
	JSON_Object *turn_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL_TURN);
	JSON_Object *on_object = json_object_get_object(turn_object, NATTY_USER_PROTOCOL_ON);
	JSON_Object *off_object = json_object_get_object(turn_object, NATTY_USER_PROTOCOL_OFF);
	pTurnReq->turn.on.time = json_object_get_string(on_object, NATTY_USER_PROTOCOL_TIME);
	pTurnReq->turn.off.time = json_object_get_string(off_object, NATTY_USER_PROTOCOL_TIME);
	ntydbg("====================end   ntyJsonTurn ==========================\n");
}


void ntyJsonAddSchedule(JSON_Value *json, AddScheduleReq *pAddScheduleReq) {
	if (json == NULL || pAddScheduleReq == NULL) {
		ntydbg("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pAddScheduleReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pAddScheduleReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	pAddScheduleReq->action = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ACTION);
	ntydbg("IMEI:%s   Category:%s  Action:%s\n", pAddScheduleReq->IMEI, pAddScheduleReq->category, pAddScheduleReq->action);

	JSON_Object *schedule_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL_SCHEDULE);
	pAddScheduleReq->schedule.daily = json_object_get_string(schedule_object, NATTY_USER_PROTOCOL_DAILY);
	pAddScheduleReq->schedule.time = json_object_get_string(schedule_object, NATTY_USER_PROTOCOL_TIME);
	pAddScheduleReq->schedule.details = json_object_get_string(schedule_object, NATTY_USER_PROTOCOL_DETAILS);
	ntydbg("Daily:%s, Time:%s, Details:%s\n", pAddScheduleReq->schedule.daily, pAddScheduleReq->schedule.time, pAddScheduleReq->schedule.details);
}


void ntyJsonUpdateSchedule(JSON_Value *json, UpdateScheduleReq *pUpdateScheduleReq) {
	if (json == NULL || pUpdateScheduleReq == NULL) {
		ntydbg("param is null.\n");
		return;
	}
	
	JSON_Object *root_object = json_value_get_object(json);
	pUpdateScheduleReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pUpdateScheduleReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	pUpdateScheduleReq->action = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ACTION);
	pUpdateScheduleReq->id = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ID);
	ntydbg("IMEI:%s   Category:%s  Action:%s  id:%s\n", pUpdateScheduleReq->IMEI, pUpdateScheduleReq->category, pUpdateScheduleReq->action, pUpdateScheduleReq->id);

	JSON_Object *schedule_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL_SCHEDULE);
	pUpdateScheduleReq->schedule.daily = json_object_get_string(schedule_object, NATTY_USER_PROTOCOL_DAILY);
	pUpdateScheduleReq->schedule.time = json_object_get_string(schedule_object, NATTY_USER_PROTOCOL_TIME);
	pUpdateScheduleReq->schedule.details= json_object_get_string(schedule_object, NATTY_USER_PROTOCOL_DETAILS);
	ntydbg("Daily:%s, Time:%s, Details:%s\n", pUpdateScheduleReq->schedule.daily, pUpdateScheduleReq->schedule.time, pUpdateScheduleReq->schedule.details);
}


void ntyJsonDelSchedule(JSON_Value *json, DelScheduleReq *pDelScheduleReq) {
	if (json == NULL || pDelScheduleReq == NULL) {
		ntydbg("param is null.\n");
		return;
	}
	
	JSON_Object *root_object = json_value_get_object(json);
	pDelScheduleReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pDelScheduleReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	pDelScheduleReq->action = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ACTION);
	pDelScheduleReq->id = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ID);
	ntydbg("IMEI:%s   Category:%s  Action:%s  id:%s\n", pDelScheduleReq->IMEI, pDelScheduleReq->category, pDelScheduleReq->action, pDelScheduleReq->id);
}

void ntyJsonTimeTables(JSON_Value *json, TimeTablesReq *pTimeTablesReq) {
	if (json == NULL || pTimeTablesReq == NULL) {
		ntydbg("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pTimeTablesReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pTimeTablesReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	ntydbg("IMEI:%s   Category:%s\n", pTimeTablesReq->IMEI, pTimeTablesReq->category);

	JSON_Object *timetables_object = NULL;
	JSON_Object *morning_object = NULL;
	JSON_Object *afternoon_object = NULL;
	JSON_Array *timetables_array = json_object_get_array(root_object, NATTY_USER_PROTOCOL_TIMETABLES);
	size_t timetables_count = json_array_get_count(timetables_array);
	TimeTablesItem *pTimeTables = malloc(sizeof(TimeTablesItem)*timetables_count);
	pTimeTablesReq->pTimeTables = pTimeTables;

	size_t i,j;
	for (i = 0; i < timetables_count; i++) {
		timetables_object = json_array_get_object(timetables_array, i);
		pTimeTables[i].daily = json_object_get_string(root_object, NATTY_USER_PROTOCOL_DAILY);
		morning_object = json_object_get_object(timetables_object, NATTY_USER_PROTOCOL_MORNING);
		pTimeTables[i].morning.status = json_object_get_string(morning_object, NATTY_USER_PROTOCOL_STATUS);
		pTimeTables[i].morning.startTime = json_object_get_string(morning_object, NATTY_USER_PROTOCOL_STARTTIME);
		pTimeTables[i].morning.endTime = json_object_get_string(morning_object, NATTY_USER_PROTOCOL_ENDTIME);
		afternoon_object = json_object_get_object(timetables_object, NATTY_USER_PROTOCOL_AFTERNOON);
		pTimeTables[i].morning.status = json_object_get_string(afternoon_object, NATTY_USER_PROTOCOL_STATUS);
		pTimeTables[i].morning.startTime = json_object_get_string(afternoon_object, NATTY_USER_PROTOCOL_STARTTIME);
		pTimeTables[i].morning.endTime = json_object_get_string(afternoon_object, NATTY_USER_PROTOCOL_ENDTIME);
	}
}

void ntyJsonAddContacts(JSON_Value *json, AddContactsReq *pAddContactsReq) {
	if (json == NULL || pAddContactsReq == NULL) {
		ntydbg("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pAddContactsReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pAddContactsReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	pAddContactsReq->action = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ACTION);
	ntydbg("IMEI:%s   Category:%s  Action:%s\n", pAddContactsReq->IMEI, pAddContactsReq->category, pAddContactsReq->action);

	JSON_Object *contacts_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL_CONTACTS);
	pAddContactsReq->contacts.name = json_object_get_string(contacts_object, NATTY_USER_PROTOCOL_NAME);
	pAddContactsReq->contacts.image = json_object_get_string(contacts_object, NATTY_USER_PROTOCOL_IMAGE);
	pAddContactsReq->contacts.telphone = json_object_get_string(contacts_object, NATTY_USER_PROTOCOL_TELPHONE);
	pAddContactsReq->contacts.app = json_object_get_string(contacts_object, NATTY_USER_PROTOCOL_APP);
}

void ntyJsonUpdateContacts(JSON_Value *json, UpdateContactsReq *pUpdateContactsReq) {
	if (json == NULL || pUpdateContactsReq == NULL) {
		ntydbg("param is null.\n");
		return;
	}
	
	JSON_Object *root_object = json_value_get_object(json);
	pUpdateContactsReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pUpdateContactsReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	pUpdateContactsReq->action = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ACTION);
	pUpdateContactsReq->id = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ID);
	ntydbg("IMEI:%s   Category:%s  Action:%s  id:%s\n", pUpdateContactsReq->IMEI, pUpdateContactsReq->category, pUpdateContactsReq->action, pUpdateContactsReq->id);

	JSON_Object *contacts_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL_CONTACTS);
	pUpdateContactsReq->contacts.name = json_object_get_string(contacts_object, NATTY_USER_PROTOCOL_NAME);
	pUpdateContactsReq->contacts.image = json_object_get_string(contacts_object, NATTY_USER_PROTOCOL_IMAGE);
	pUpdateContactsReq->contacts.telphone = json_object_get_string(contacts_object, NATTY_USER_PROTOCOL_TELPHONE);
	pUpdateContactsReq->contacts.app = json_object_get_string(contacts_object, NATTY_USER_PROTOCOL_APP);
}

void ntyJsonDelContacts(JSON_Value *json, DelContactsReq *pDelContactsReq) {
	if (json == NULL || pDelContactsReq == NULL) {
		ntydbg("param is null.\n");
		return;
	}
	
	JSON_Object *root_object = json_value_get_object(json);
	pDelContactsReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pDelContactsReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	pDelContactsReq->action = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ACTION);
	pDelContactsReq->id = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ID);
	ntydbg("IMEI:%s   Category:%s  Action:%s  id:%s\n", pDelContactsReq->IMEI, pDelContactsReq->category, pDelContactsReq->action, pDelContactsReq->id);
}


void ntyJsonTimeTablesItemRelease(TimeTablesItem *pTimeTables) {
	if (pTimeTables != NULL) {
		free(pTimeTables);
	}
}


char * ntyJsonWriteCommon(CommonAck *pCommonAck) {
	if (pCommonAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULT, json_value_init_object());
	JSON_Object *result_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULT);
	json_object_set_string(result_obj, NATTY_USER_PROTOCOL_STATUS, pCommonAck->result.status);

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}

char * ntyJsonWriteConfig(ConfigAck *pConfigAck) {
	if (pConfigAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, pConfigAck->results.IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_CATEGORY, pConfigAck->results.category);
	json_object_set_value(results_obj, NATTY_USER_PROTOCOL_CONFIG, json_value_init_object());
	JSON_Object *config_obj = json_object_get_object(results_obj, NATTY_USER_PROTOCOL_CONFIG);
	
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_POWER, pConfigAck->results.config.power);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_SIGNAL, pConfigAck->results.config.signal);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_STEPS, pConfigAck->results.config.steps);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_PHONENUM, pConfigAck->results.config.phone_num);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_LOCATION, pConfigAck->results.config.location);

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}

char * ntyJsonWritePower(PowerAck *pPowerAck) {
	if (pPowerAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_POWER, pPowerAck->results.power);

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}

char * ntyJsonWriteSignal(SignalAck *pSignalAck) {
	if (pSignalAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_SIGNAL, pSignalAck->results.signal);

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}

char * ntyJsonWriteWIFI(WIFIAck *pWIFIAck) {
	if (pWIFIAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);

	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, pWIFIAck->result.IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_CATEGORY, pWIFIAck->result.category);
	

	json_object_set_value(results_obj, NATTY_USER_PROTOCOL_WIFI, json_value_init_array());
	JSON_Array *wifi_arr = json_object_get_array(results_obj, NATTY_USER_PROTOCOL_WIFI);
	
	size_t i;
	for (i = 0; i < pWIFIAck->result.size; i++) {
		json_array_append_value(wifi_arr, json_value_init_object());
		JSON_Object *wifiitem_obj = json_array_get_object(wifi_arr, i);
		json_object_set_string(wifiitem_obj, NATTY_USER_PROTOCOL_SSID, pWIFIAck->result.pWIFI[i].SSID);
		json_object_set_string(wifiitem_obj, NATTY_USER_PROTOCOL_MAC, pWIFIAck->result.pWIFI[i].MAC);
		json_object_set_string(wifiitem_obj, NATTY_USER_PROTOCOL_V, pWIFIAck->result.pWIFI[i].V);
	}

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;

}

char * ntyJsonWriteLAB(LABAck *pLABAck) {
	if (pLABAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);

	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, pLABAck->result.IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_CATEGORY, pLABAck->result.category);

	JSON_Object *lab_obj = json_object_get_object(results_obj, NATTY_USER_PROTOCOL_LAB);
	json_object_set_string(lab_obj, NATTY_USER_PROTOCOL_BTS, pLABAck->result.lab.bts);

	json_object_set_value(lab_obj, NATTY_USER_PROTOCOL_NEARBTS, json_value_init_array());
	JSON_Array *nearbts_arr = json_object_get_array(lab_obj, NATTY_USER_PROTOCOL_NEARBTS);
	
	size_t i;
	for (i = 0; i < pLABAck->result.size; i++) {
		json_array_append_value(nearbts_arr, json_value_init_object());
		JSON_Object *nearbts_obj = json_array_get_object(nearbts_arr, i);
		json_object_set_string(nearbts_obj, NATTY_USER_PROTOCOL_SIGNAL, pLABAck->result.lab.pNearbts[i].signal);
		json_object_set_string(nearbts_obj, NATTY_USER_PROTOCOL_CELL, pLABAck->result.lab.pNearbts[i].cell);
	}

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}


char *ntyJsonWriteLocation(LocationAck *pLocationAck) {
	if (pLocationAck == NULL) {
		return NULL;
	}
	
	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_TYPE, pLocationAck->results.type);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_RADIUS, pLocationAck->results.radius);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_LOCATION, pLocationAck->results.location);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_CATEGORY, pLocationAck->results.category);

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}


char *ntyJsonWriteWeather(WeatherAck *pWeatherAck) {
	if (pWeatherAck == NULL) {
		return NULL;
	}
	return NULL;
}

char * ntyJsonWriteEfence(EfenceAck *pEfenceAck) {
	if (pEfenceAck == NULL) {
		return NULL;
	}
	
	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, pEfenceAck->result.IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_CATEGORY, pEfenceAck->result.category);

	json_object_set_value(results_obj, NATTY_USER_PROTOCOL_EFENCE, json_value_init_object());
	JSON_Object *efence_obj = json_object_get_object(results_obj, NATTY_USER_PROTOCOL_EFENCE);
	json_object_set_string(efence_obj, NATTY_USER_PROTOCOL_NUM, pEfenceAck->result.efence.num);
	
	json_object_set_value(efence_obj, NATTY_USER_PROTOCOL_POINTS, json_value_init_array());
	JSON_Array *points_arr = json_object_get_array(efence_obj, NATTY_USER_PROTOCOL_POINTS);
	
	size_t i;
	for (i = 0; i < pEfenceAck->result.efence.size; i++) {
		json_array_append_string(points_arr, pEfenceAck->result.efence.pPoints[i].point);
	}

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}

char * ntyJsonWriteRunTime(RunTimeAck *pRunTimeAck) {
	if (pRunTimeAck == NULL) {
		return NULL;
	}
	
	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, pRunTimeAck->result.IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_CATEGORY, pRunTimeAck->result.category);

	json_object_set_value(results_obj, NATTY_USER_PROTOCOL_RUNTIME, json_value_init_object());
	JSON_Object *runtime_obj = json_object_get_object(results_obj, NATTY_USER_PROTOCOL_RUNTIME);
	json_object_set_string(runtime_obj, NATTY_USER_PROTOCOL_AUTOCONNECTION, pRunTimeAck->result.runtime.auto_connection);
	json_object_set_string(runtime_obj, NATTY_USER_PROTOCOL_LOSSREPORT, pRunTimeAck->result.runtime.loss_report);
	json_object_set_string(runtime_obj, NATTY_USER_PROTOCOL_LIGHTPANEL, pRunTimeAck->result.runtime.light_panel);
	json_object_set_string(runtime_obj, NATTY_USER_PROTOCOL_WATCHBELL, pRunTimeAck->result.runtime.watch_bell);
	json_object_set_string(runtime_obj, NATTY_USER_PROTOCOL_TAGETSTEP, pRunTimeAck->result.runtime.taget_step);

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}


char * ntyJsonWriteSchedule(ScheduleAck *pScheduleAck) {
	if (pScheduleAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, pScheduleAck->results.IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_CATEGORY, pScheduleAck->results.category);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_NUM, pScheduleAck->results.num);

	json_object_set_value(results_obj, NATTY_USER_PROTOCOL_SCHEDULE, json_value_init_array());
	JSON_Array *schedule_arr = json_object_get_array(results_obj, NATTY_USER_PROTOCOL_SCHEDULE);
	
	size_t i;
	for (i = 0; i < pScheduleAck->results.size; i++) {
		json_array_append_value(schedule_arr, json_value_init_object());
		JSON_Object *schedule_obj = json_array_get_object(schedule_arr, i);
		json_object_set_string(schedule_obj, NATTY_USER_PROTOCOL_ID, pScheduleAck->results.pSchedule[i].id);
		json_object_set_string(schedule_obj, NATTY_USER_PROTOCOL_DAILY, pScheduleAck->results.pSchedule[i].daily);
		json_object_set_string(schedule_obj, NATTY_USER_PROTOCOL_TIME, pScheduleAck->results.pSchedule[i].time);
		json_object_set_string(schedule_obj, NATTY_USER_PROTOCOL_DETAILS, pScheduleAck->results.pSchedule[i].details);
	}

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}

char * ntyJsonWriteTimeTables(TimeTablesAck *pTimeTablesAck) {
	if (pTimeTablesAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, pTimeTablesAck->results.IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_CATEGORY, pTimeTablesAck->results.category);

	json_object_set_value(results_obj, NATTY_USER_PROTOCOL_TIMETABLES, json_value_init_array());
	JSON_Array *timetables_arr = json_object_get_array(results_obj, NATTY_USER_PROTOCOL_TIMETABLES);
	
	size_t i;
	for (i = 0; i < pTimeTablesAck->results.size; i++) {
		json_array_append_value(timetables_arr, json_value_init_object());
		JSON_Object *timetables_obj = json_array_get_object(timetables_arr, i);
		json_object_set_string(timetables_obj, NATTY_USER_PROTOCOL_DAILY, pTimeTablesAck->results.pTimeTables[i].daily);

		json_object_set_value(timetables_obj, NATTY_USER_PROTOCOL_MORNING, json_value_init_object());
		JSON_Object *morning_obj = json_object_get_object(timetables_obj, NATTY_USER_PROTOCOL_MORNING);
		json_object_set_string(morning_obj, NATTY_USER_PROTOCOL_STATUS, pTimeTablesAck->results.pTimeTables[i].morning.status);
		json_object_set_string(morning_obj, NATTY_USER_PROTOCOL_STARTTIME, pTimeTablesAck->results.pTimeTables[i].morning.startTime);
		json_object_set_string(morning_obj, NATTY_USER_PROTOCOL_ENDTIME, pTimeTablesAck->results.pTimeTables[i].morning.endTime);

		json_object_set_value(timetables_obj, NATTY_USER_PROTOCOL_AFTERNOON, json_value_init_object());
		JSON_Object *afternoon_obj = json_object_get_object(timetables_obj, NATTY_USER_PROTOCOL_AFTERNOON);
		json_object_set_string(afternoon_obj, NATTY_USER_PROTOCOL_STATUS, pTimeTablesAck->results.pTimeTables[i].afternoon.status);
		json_object_set_string(afternoon_obj, NATTY_USER_PROTOCOL_STARTTIME, pTimeTablesAck->results.pTimeTables[i].afternoon.startTime);
		json_object_set_string(afternoon_obj, NATTY_USER_PROTOCOL_ENDTIME, pTimeTablesAck->results.pTimeTables[i].afternoon.endTime);
	}

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}


char * ntyJsonWriteContacts(ContactsAck *pContactsAck) {
	if (pContactsAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, pContactsAck->results.IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_CATEGORY, pContactsAck->results.category);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_NUM, pContactsAck->results.num);

	json_object_set_value(results_obj, NATTY_USER_PROTOCOL_CONTACTS, json_value_init_array());
	JSON_Array *contacts_arr = json_object_get_array(results_obj, NATTY_USER_PROTOCOL_CONTACTS);
	
	size_t i;
	for (i = 0; i < pContactsAck->results.size; i++) {
		json_array_append_value(contacts_arr, json_value_init_object());
		JSON_Object *contacts_obj = json_array_get_object(contacts_arr, i);
		json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_NAME, pContactsAck->results.pContacts[i].name);
		json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_IMAGE, pContactsAck->results.pContacts[i].image);
		json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_TELPHONE, pContactsAck->results.pContacts[i].telphone);
		json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_APP, pContactsAck->results.pContacts[i].app);
	}

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}




