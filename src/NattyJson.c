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


JSON_Value* ntyMallocJsonByString(const char *jsonstring) {
	if (jsonstring == NULL) {
		return NULL;
	}
	
	JSON_Value *schema = json_parse_string(jsonstring);
	return schema;
}

void ntyFreeJson(JSON_Value *json) {
	if (json == NULL) {
		return;
	}
	
	json_value_free(json);
}

const char * ntyJsonAppCategory(JSON_Value *json) {
	if (json == NULL) {
		return NULL;
	}
	JSON_Object *root_object = json_value_get_object(json);
	const char *category = json_object_get_string(root_object, NATTY_USER_PROTOCOL__CATEGORY);
	return category;
}

const char * ntyJsonWatchCategory(JSON_Value *json) {
	if (json == NULL) {
		return NULL;
	}
	JSON_Object *root_object = json_value_get_object(json);
	JSON_Object *results_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL__RESULTS);
	const char *category = json_object_get_string(results_object, NATTY_USER_PROTOCOL__CATEGORY);
	return category;
}

const char * ntyJsonAction(JSON_Value *json) {
	if (json == NULL) {
		return NULL;
	}
	JSON_Object *root_object = json_value_get_object(json);
	const char *action = json_object_get_string(root_object, NATTY_USER_PROTOCOL__ACTION);
	return action;
}

void ntyJsonWIFI(JSON_Value *json, WIFIReq *pWIFIReq) {
	if (json == NULL || pWIFIReq == NULL) {
		ntydbg("param is null.\n");
		return;
	}
	
	JSON_Object *root_object = json_value_get_object(json);
	pWIFIReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL__IMEI);
	pWIFIReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL__CATEGORY);
	ntydbg("IMEI:%s   Category:%s\n", pWIFIReq->IMEI, pWIFIReq->category);
	
	JSON_Object *wifi_object = NULL;
	JSON_Array *wifi_array = json_object_get_array(root_object, NATTY_USER_PROTOCOL__WIFI);
	size_t wifi_count = json_array_get_count(wifi_array);
	WIFIItem *pWIFI = malloc(sizeof(WIFIItem)*wifi_count);
	pWIFIReq->pWIFI = pWIFI;

	size_t i;
	for (i = 0; i < wifi_count; i++) {
		wifi_object = json_array_get_object(wifi_array, i);
		pWIFI[i].SSID = json_object_get_string(wifi_object, NATTY_USER_PROTOCOL__SSID);
		pWIFI[i].MAC = json_object_get_string(wifi_object, NATTY_USER_PROTOCOL__MAC);
		pWIFI[i].V = json_object_get_string(wifi_object, NATTY_USER_PROTOCOL__V);
		ntydbg("SSID:%s   MAC:%s   V:%s\n", pWIFI[i].SSID, pWIFI[i].MAC, pWIFI[i].V);
	}
	
}


void ntyJsonLAB(JSON_Value *json, LABReq *pLABReq) {
	if (json == NULL || pLABReq == NULL) {
		ntydbg("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pLABReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL__IMEI);
	pLABReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL__CATEGORY);
	ntydbg("IMEI:%s   Category:%s\n", pLABReq->IMEI, pLABReq->category);

	JSON_Object *lab_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL__LAB);
	pLABReq->lab.bts = json_object_get_string(lab_object, NATTY_USER_PROTOCOL__BTS);
	ntydbg("bts:%s\n", pLABReq->lab.bts);

	JSON_Object *nearbts_object = NULL;
	JSON_Array *nearbts_array = json_object_get_array(lab_object, NATTY_USER_PROTOCOL__NEARBTS);
	size_t nearbts_count = json_array_get_count(nearbts_array);
	Nearbts *pNearbts = malloc(sizeof(Nearbts)*nearbts_count);
	pLABReq->lab.pNearbts = pNearbts;
	size_t i;
	for (i = 0; i < nearbts_count; i++) {
		nearbts_object = json_array_get_object(nearbts_array, i);
		pNearbts[i].cell = json_object_get_string(nearbts_object, NATTY_USER_PROTOCOL__CELL);
		pNearbts[i].signal = json_object_get_string(nearbts_object, NATTY_USER_PROTOCOL__SIGNAL);
		ntydbg("cell:%s   signal:%s\n", pNearbts[i].cell, pNearbts[i].signal);
	}
	
}

/*
void ntyJsonLocation(JSON_Value *json, LocationAck *pLocation) {
	if (json == NULL || pLocation == NULL) {
		ntydbg("param is null.\n");
		return;
	}
	
	JSON_Object *root_object = json_value_get_object(json);
	JSON_Object *results_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL__RESULTS);
	pLocation->results.IMEI = json_object_get_string(results_object, NATTY_USER_PROTOCOL__IMEI);
	pLocation->results.category = json_object_get_string(results_object, NATTY_USER_PROTOCOL__CATEGORY);
	pLocation->results.type = json_object_get_string(results_object, NATTY_USER_PROTOCOL__TYPE);
	pLocation->results.radius = json_object_get_string(results_object, NATTY_USER_PROTOCOL__RADIUS);
	pLocation->results.location = json_object_get_string(results_object, NATTY_USER_PROTOCOL__LOCATION);
	
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
	pAMap->status = json_object_get_string(root_object, NATTY_USER_PROTOCOL__STATUS);
	pAMap->info = json_object_get_string(root_object, NATTY_USER_PROTOCOL__INFO);
	pAMap->infocode = json_object_get_string(root_object, NATTY_USER_PROTOCOL__INFOCODE);
	
	JSON_Object *result_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL__RESULT);
	pAMap->result.type = json_object_get_string(result_object, NATTY_USER_PROTOCOL__TYPE);
	pAMap->result.radius = json_object_get_string(result_object, NATTY_USER_PROTOCOL__RADIUS);
	pAMap->result.location = json_object_get_string(result_object, NATTY_USER_PROTOCOL__LOCATION);
	pAMap->result.desc = json_object_get_string(result_object, NATTY_USER_PROTOCOL__DESC);
	pAMap->result.country = json_object_get_string(result_object, NATTY_USER_PROTOCOL__COUNTRY);
	pAMap->result.city = json_object_get_string(result_object, NATTY_USER_PROTOCOL__CITY);
	pAMap->result.citycode = json_object_get_string(result_object, NATTY_USER_PROTOCOL__CITYCODE);
	pAMap->result.adcode= json_object_get_string(result_object, NATTY_USER_PROTOCOL__ADCODE);
	pAMap->result.road = json_object_get_string(result_object, NATTY_USER_PROTOCOL__ROAD);
	pAMap->result.street = json_object_get_string(result_object, NATTY_USER_PROTOCOL__STREET);
	pAMap->result.poi = json_object_get_string(result_object, NATTY_USER_PROTOCOL__POI);
	
	ntydbg("status:%s,  info:%s,  infocode:%s,  location:%s\n", pAMap->status, pAMap->info, pAMap->infocode, pAMap->result.location);
}

/*
void ntyJsonWeather(JSON_Value *json, WeatherAck *pWeather) {
	return;
}
*/

void ntyJsonCommon(JSON_Value *json, CommonReq *pCommonReq) {
	if (json == NULL || pCommonReq == NULL) {
		ntydbg("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pCommonReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL__IMEI);
	pCommonReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL__CATEGORY);
	pCommonReq->action = json_object_get_string(root_object, NATTY_USER_PROTOCOL__ACTION);
}


void ntyJsonEfence(JSON_Value *json, EfenceReq *pEfenceReq) {
	if (json == NULL || pEfenceReq == NULL) {
		ntydbg("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pEfenceReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL__IMEI);
	pEfenceReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL__CATEGORY);
	
	JSON_Object *efence_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL__EFENEC);
	pEfenceReq->efence.num = json_object_get_string(efence_object, NATTY_USER_PROTOCOL__NUM);

	JSON_Object *point_object = NULL;
	JSON_Array *points_array = json_object_get_array(efence_object, NATTY_USER_PROTOCOL__POINTS);
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


void ntyJsonRuntime(JSON_Value *json, RunTimeReq *pRunTimeReq) {
	if (json == NULL || pRunTimeReq == NULL) {
		ntydbg("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pRunTimeReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL__IMEI);
	pRunTimeReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL__CATEGORY);
	ntydbg("IMEI:%s   Category:%s\n", pRunTimeReq->IMEI, pRunTimeReq->category);
	
	JSON_Object *runtime_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL__RUNTIME);
	pRunTimeReq->runtime.auto_connection = json_object_get_string(runtime_object, NATTY_USER_PROTOCOL__AUTOCONNECTION);
	pRunTimeReq->runtime.loss_report = json_object_get_string(runtime_object, NATTY_USER_PROTOCOL__LOSSREPORT);
	pRunTimeReq->runtime.light_panel = json_object_get_string(runtime_object, NATTY_USER_PROTOCOL__LIGHTPANEL);
	pRunTimeReq->runtime.watch_bell = json_object_get_string(runtime_object, NATTY_USER_PROTOCOL__WATCHBELL);
	pRunTimeReq->runtime.taget_step = json_object_get_string(runtime_object, NATTY_USER_PROTOCOL__TAGETSTEP);
	
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

	JSON_Object *root_object = json_value_get_object(json);
	pTurnReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL__IMEI);
	pTurnReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL__CATEGORY);
	ntydbg("IMEI:%s   Category:%s\n", pTurnReq->IMEI, pTurnReq->category);
	
	JSON_Object *turn_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL__TURN);
	JSON_Object *on_object = json_object_get_object(turn_object, NATTY_USER_PROTOCOL__ON);
	JSON_Object *off_object = json_object_get_object(turn_object, NATTY_USER_PROTOCOL__OFF);
	pTurnReq->turn.on.time = json_object_get_string(on_object, NATTY_USER_PROTOCOL__TIME);
	pTurnReq->turn.off.time = json_object_get_string(off_object, NATTY_USER_PROTOCOL__TIME);
}


void ntyJsonAddSchedule(JSON_Value *json, AddScheduleReq *pAddScheduleReq) {
	if (json == NULL || pAddScheduleReq == NULL) {
		ntydbg("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pAddScheduleReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL__IMEI);
	pAddScheduleReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL__CATEGORY);
	pAddScheduleReq->action = json_object_get_string(root_object, NATTY_USER_PROTOCOL__ACTION);
	ntydbg("IMEI:%s   Category:%s  Action:%s\n", pAddScheduleReq->IMEI, pAddScheduleReq->category, pAddScheduleReq->action);

	JSON_Object *schedule_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL__SCHEDULE);
	pAddScheduleReq->schedule.daily = json_object_get_string(schedule_object, NATTY_USER_PROTOCOL__DAILY);
	pAddScheduleReq->schedule.time = json_object_get_string(schedule_object, NATTY_USER_PROTOCOL__TIME);
	pAddScheduleReq->schedule.details = json_object_get_string(schedule_object, NATTY_USER_PROTOCOL__DETAILS);
	ntydbg("Daily:%s, Time:%s, Details:%s\n", pAddScheduleReq->schedule.daily, pAddScheduleReq->schedule.time, pAddScheduleReq->schedule.details);
}


void ntyJsonUpdateSchedule(JSON_Value *json, UpdateScheduleReq *pUpdateScheduleReq) {
	if (json == NULL || pUpdateScheduleReq == NULL) {
		ntydbg("param is null.\n");
		return;
	}
	
	JSON_Object *root_object = json_value_get_object(json);
	pUpdateScheduleReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL__IMEI);
	pUpdateScheduleReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL__CATEGORY);
	pUpdateScheduleReq->action = json_object_get_string(root_object, NATTY_USER_PROTOCOL__ACTION);
	pUpdateScheduleReq->id = json_object_get_string(root_object, NATTY_USER_PROTOCOL__ID);
	ntydbg("IMEI:%s   Category:%s  Action:%s  id:%s\n", pUpdateScheduleReq->IMEI, pUpdateScheduleReq->category, pUpdateScheduleReq->action, pUpdateScheduleReq->id);

	JSON_Object *schedule_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL__SCHEDULE);
	pUpdateScheduleReq->schedule.daily = json_object_get_string(schedule_object, NATTY_USER_PROTOCOL__DAILY);
	pUpdateScheduleReq->schedule.time = json_object_get_string(schedule_object, NATTY_USER_PROTOCOL__TIME);
	pUpdateScheduleReq->schedule.details= json_object_get_string(schedule_object, NATTY_USER_PROTOCOL__DETAILS);
	ntydbg("Daily:%s, Time:%s, Details:%s\n", pUpdateScheduleReq->schedule.daily, pUpdateScheduleReq->schedule.time, pUpdateScheduleReq->schedule.details);
}


void ntyJsonDelSchedule(JSON_Value *json, DelScheduleReq *pDelScheduleReq) {
	if (json == NULL || pDelScheduleReq == NULL) {
		ntydbg("param is null.\n");
		return;
	}
	
	JSON_Object *root_object = json_value_get_object(json);
	pDelScheduleReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL__IMEI);
	pDelScheduleReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL__CATEGORY);
	pDelScheduleReq->action = json_object_get_string(root_object, NATTY_USER_PROTOCOL__ACTION);
	pDelScheduleReq->id = json_object_get_string(root_object, NATTY_USER_PROTOCOL__ID);
	ntydbg("IMEI:%s   Category:%s  Action:%s  id:%s\n", pDelScheduleReq->IMEI, pDelScheduleReq->category, pDelScheduleReq->action, pDelScheduleReq->id);
}

void ntyJsonTimeTables(JSON_Value *json, TimeTablesReq *pTimeTablesReq) {
	if (json == NULL || pTimeTablesReq == NULL) {
		ntydbg("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pTimeTablesReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL__IMEI);
	pTimeTablesReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL__CATEGORY);
	ntydbg("IMEI:%s   Category:%s\n", pTimeTablesReq->IMEI, pTimeTablesReq->category);

	JSON_Object *timetables_object = NULL;
	JSON_Object *morning_object = NULL;
	JSON_Object *afternoon_object = NULL;
	JSON_Array *timetables_array = json_object_get_array(root_object, NATTY_USER_PROTOCOL__TIMETABLES);
	size_t timetables_count = json_array_get_count(timetables_array);
	TimeTablesItem *pTimeTables = malloc(sizeof(TimeTablesItem)*timetables_count);
	pTimeTablesReq->pTimeTables = pTimeTables;

	size_t i,j;
	for (i = 0; i < timetables_count; i++) {
		timetables_object = json_array_get_object(timetables_array, i);
		pTimeTables[i].daily = json_object_get_string(root_object, NATTY_USER_PROTOCOL__DAILY);
		morning_object = json_object_get_object(timetables_object, NATTY_USER_PROTOCOL__MORNING);
		pTimeTables[i].morning.status = json_object_get_string(morning_object, NATTY_USER_PROTOCOL__STATUS);
		pTimeTables[i].morning.startTime = json_object_get_string(morning_object, NATTY_USER_PROTOCOL__STARTTIME);
		pTimeTables[i].morning.endTime = json_object_get_string(morning_object, NATTY_USER_PROTOCOL__ENDTIME);
		afternoon_object = json_object_get_object(timetables_object, NATTY_USER_PROTOCOL__AFTERNOON);
		pTimeTables[i].morning.status = json_object_get_string(afternoon_object, NATTY_USER_PROTOCOL__STATUS);
		pTimeTables[i].morning.startTime = json_object_get_string(afternoon_object, NATTY_USER_PROTOCOL__STARTTIME);
		pTimeTables[i].morning.endTime = json_object_get_string(afternoon_object, NATTY_USER_PROTOCOL__ENDTIME);
	}
}


char * ntyJsonWriteConfig(ConfigAck *pConfigAck) {
	if (pConfigAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL__RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL__RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL__IMEI, pConfigAck->results.IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL__CATEGORY, pConfigAck->results.category);
	json_object_set_value(results_obj, NATTY_USER_PROTOCOL__CONFIG, json_value_init_object());
	JSON_Object *config_obj = json_object_get_object(results_obj, NATTY_USER_PROTOCOL__CONFIG);
	
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL__POWER, pConfigAck->results.config.power);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL__SIGNAL, pConfigAck->results.config.signal);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL__STEPS, pConfigAck->results.config.steps);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL__PHONENUM, pConfigAck->results.config.phone_num);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL__LOCATION, pConfigAck->results.config.location);

	
	return json_serialize_to_string(schema);
}

char * ntyJsonWritePower(PowerAck *pPowerAck) {
	if (pPowerAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL__RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL__RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL__POWER, pPowerAck->results.power);
	
	return json_serialize_to_string(schema);
}

char * ntyJsonWriteSignal(SignalAck *pSignalAck) {
	if (pSignalAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL__RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL__RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL__SIGNAL, pSignalAck->results.signal);
	
	return json_serialize_to_string(schema);
}


char *ntyJsonWriteLocation(LocationAck *pLocationAck) {
	if (pLocationAck == NULL) {
		return NULL;
	}
	
	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL__RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL__RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL__TYPE, pLocationAck->results.type);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL__RADIUS, pLocationAck->results.radius);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL__LOCATION, pLocationAck->results.location);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL__CATEGORY, pLocationAck->results.category);
	
	return json_serialize_to_string(schema);
}


char *ntyJsonWriteWeather(WeatherAck *pWeatherAck) {
	if (pWeatherAck == NULL) {
		return NULL;
	}
	return NULL;
}


char * ntyJsonWriteRunTime(RunTimeAck *pRunTimeAck) {
	if (pRunTimeAck == NULL) {
		return NULL;
	}
	
	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL__RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL__RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL__IMEI, pRunTimeAck->result.IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL__CATEGORY, pRunTimeAck->result.category);

	json_object_set_value(results_obj, NATTY_USER_PROTOCOL__RUNTIME, json_value_init_object());
	JSON_Object *runtime_obj = json_object_get_object(results_obj, NATTY_USER_PROTOCOL__RUNTIME);
	json_object_set_string(runtime_obj, NATTY_USER_PROTOCOL__AUTOCONNECTION, pRunTimeAck->result.runtime.auto_connection);
	json_object_set_string(runtime_obj, NATTY_USER_PROTOCOL__LOSSREPORT, pRunTimeAck->result.runtime.loss_report);
	json_object_set_string(runtime_obj, NATTY_USER_PROTOCOL__LIGHTPANEL, pRunTimeAck->result.runtime.light_panel);
	json_object_set_string(runtime_obj, NATTY_USER_PROTOCOL__WATCHBELL, pRunTimeAck->result.runtime.watch_bell);
	json_object_set_string(runtime_obj, NATTY_USER_PROTOCOL__TAGETSTEP, pRunTimeAck->result.runtime.taget_step);
	
	return json_serialize_to_string(schema);
}


char * ntyJsonWriteSchedule(ScheduleAck *pScheduleAck) {
	if (pScheduleAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL__RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL__RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL__IMEI, pScheduleAck->results.IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL__CATEGORY, pScheduleAck->results.category);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL__NUM, pScheduleAck->results.num);

	json_object_set_value(results_obj, NATTY_USER_PROTOCOL__SCHEDULE, json_value_init_array());
	JSON_Array *schedule_arr = json_object_get_array(results_obj, NATTY_USER_PROTOCOL__SCHEDULE);
	
	size_t i;
	for (i = 0; i < pScheduleAck->results.size; i++) {
		json_array_append_value(schedule_arr, json_value_init_object());
		JSON_Object *schedule_obj = json_array_get_object(schedule_arr, i);
		json_object_set_string(schedule_obj, NATTY_USER_PROTOCOL__ID, pScheduleAck->results.pSchedule[i].id);
		json_object_set_string(schedule_obj, NATTY_USER_PROTOCOL__DAILY, pScheduleAck->results.pSchedule[i].daily);
		json_object_set_string(schedule_obj, NATTY_USER_PROTOCOL__TIME, pScheduleAck->results.pSchedule[i].time);
		json_object_set_string(schedule_obj, NATTY_USER_PROTOCOL__DETAILS, pScheduleAck->results.pSchedule[i].details);
	}
	return json_serialize_to_string(schema);
}

char * ntyJsonWriteTimeTables(TimeTablesAck *pTimeTablesAck) {
	if (pTimeTablesAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL__RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL__RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL__IMEI, pTimeTablesAck->results.IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL__CATEGORY, pTimeTablesAck->results.category);

	json_object_set_value(results_obj, NATTY_USER_PROTOCOL__TIMETABLES, json_value_init_array());
	JSON_Array *timetables_arr = json_object_get_array(results_obj, NATTY_USER_PROTOCOL__TIMETABLES);
	
	size_t i;
	for (i = 0; i < pTimeTablesAck->results.size; i++) {
		json_array_append_value(timetables_arr, json_value_init_object());
		JSON_Object *timetables_obj = json_array_get_object(timetables_arr, i);
		json_object_set_string(timetables_obj, NATTY_USER_PROTOCOL__DAILY, pTimeTablesAck->results.pTimeTables[i].daily);

		json_object_set_value(timetables_obj, NATTY_USER_PROTOCOL__MORNING, json_value_init_object());
		JSON_Object *morning_obj = json_object_get_object(timetables_obj, NATTY_USER_PROTOCOL__MORNING);
		json_object_set_string(morning_obj, NATTY_USER_PROTOCOL__STATUS, pTimeTablesAck->results.pTimeTables[i].morning.status);
		json_object_set_string(morning_obj, NATTY_USER_PROTOCOL__STARTTIME, pTimeTablesAck->results.pTimeTables[i].morning.startTime);
		json_object_set_string(morning_obj, NATTY_USER_PROTOCOL__ENDTIME, pTimeTablesAck->results.pTimeTables[i].morning.endTime);

		json_object_set_value(timetables_obj, NATTY_USER_PROTOCOL__AFTERNOON, json_value_init_object());
		JSON_Object *afternoon_obj = json_object_get_object(timetables_obj, NATTY_USER_PROTOCOL__AFTERNOON);
		json_object_set_string(afternoon_obj, NATTY_USER_PROTOCOL__STATUS, pTimeTablesAck->results.pTimeTables[i].afternoon.status);
		json_object_set_string(afternoon_obj, NATTY_USER_PROTOCOL__STARTTIME, pTimeTablesAck->results.pTimeTables[i].afternoon.startTime);
		json_object_set_string(afternoon_obj, NATTY_USER_PROTOCOL__ENDTIME, pTimeTablesAck->results.pTimeTables[i].afternoon.endTime);
	}
	return json_serialize_to_string(schema);
}










