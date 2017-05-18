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
#include "NattyResult.h"


#if 0
typedef unsigned char U8;
typedef unsigned short U16;
#endif


void ntyCopyString(char **dest, const char *src, size_t size) {
	*dest = malloc(size+1);
	if (*dest != NULL) {
		memset(*dest, 0, size+1);
		memcpy(*dest, src, size);
	} else {
		ntylog(" %s --> malloc failed dest\n", __func__);
		*dest = NULL;
	}
}

void ntyJsonSetLocationType(const char *locationType, int *u8LocationType) {
	if (strcmp(locationType, NATTY_USER_PROTOCOL_WIFI) == 0) {
		*u8LocationType = 1;
	} else if (strcmp(locationType, NATTY_USER_PROTOCOL_LAB) == 0) {
		*u8LocationType = 2;
	}
}

void ntyJsonGetLocationType(const int u8LocationType, char* locationType) {
	if (u8LocationType == 1) {
		locationType = NATTY_USER_PROTOCOL_WIFI;
	} else if (u8LocationType == 2) {
		locationType = NATTY_USER_PROTOCOL_LAB;
	}
}

WeatherAck* ntyInitWeather() {
	size_t len_WeatherAck = sizeof(WeatherAck);
	WeatherAck *pWeatherAck = malloc(len_WeatherAck);
	if (pWeatherAck == NULL) {
		ntylog(" ---ntyInitWeather  error ---\n");
		return NULL;
	}
	memset(pWeatherAck, 0, len_WeatherAck);
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
	size_t len_ScheduleAck = sizeof(ScheduleAck);
	ScheduleAck *pScheduleAck = malloc(len_ScheduleAck);
	if (pScheduleAck == NULL) {
		ntylog(" ---ntyInitScheduleAck  error ---\n");
		return NULL;
	}
	memset(pScheduleAck, 0, len_ScheduleAck);
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
	size_t len_TimeTablesAck = sizeof(TimeTablesAck);
	TimeTablesAck *pTimeTablesAck = malloc(len_TimeTablesAck);
	if (pTimeTablesAck == NULL) {
		ntylog(" ---ntyInitTimeTablesAck  error ---\n");
		return NULL;
	}
	memset(pTimeTablesAck, 0, len_TimeTablesAck);
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
		ntylog("param is null.\n");
		return;
	}
	
	JSON_Object *root_object = json_value_get_object(json);
	pWIFIReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pWIFIReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	ntylog("IMEI:%s   Category:%s\n", pWIFIReq->IMEI, pWIFIReq->category);
	
	JSON_Object *wifi_object = NULL;
	JSON_Array *wifi_array = json_object_get_array(root_object, NATTY_USER_PROTOCOL_WIFI);
	pWIFIReq->size = json_array_get_count(wifi_array);
	WIFIItem *pWIFI = malloc(sizeof(WIFIItem)*pWIFIReq->size);
	if (pWIFI == NULL) {
		ntylog("ntyJsonWIFI --> malloc failed\n");

		pWIFIReq->size = 0;
		return ;
	}
	memset(pWIFI, 0, pWIFIReq->size*sizeof(WIFIItem));
	
	pWIFIReq->pWIFI = pWIFI;

	size_t i;
	for (i = 0; i < pWIFIReq->size; i++) {
		wifi_object = json_array_get_object(wifi_array, i);
		pWIFI[i].SSID = json_object_get_string(wifi_object, NATTY_USER_PROTOCOL_SSID);
		pWIFI[i].MAC = json_object_get_string(wifi_object, NATTY_USER_PROTOCOL_MAC);
		pWIFI[i].V = json_object_get_string(wifi_object, NATTY_USER_PROTOCOL_V);
		ntylog("SSID:%s   MAC:%s   V:%s\n", pWIFI[i].SSID, pWIFI[i].MAC, pWIFI[i].V);
	}
}

void ntyJsonWIFIItemRelease(WIFIItem *pWIFI) {
	if (pWIFI != NULL) {
		free(pWIFI);
	}
}

void ntyJsonLAB(JSON_Value *json, LABReq *pLABReq) {
	if (json == NULL || pLABReq == NULL) {
		ntylog("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pLABReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pLABReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);

	JSON_Object *lab_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL_LAB);
	pLABReq->lab.bts = json_object_get_string(lab_object, NATTY_USER_PROTOCOL_BTS);

	JSON_Object *nearbts_object = NULL;
	JSON_Array *nearbts_array = json_object_get_array(lab_object, NATTY_USER_PROTOCOL_NEARBTS);
	pLABReq->size = json_array_get_count(nearbts_array);
	Nearbts *pNearbts = malloc(sizeof(Nearbts)*pLABReq->size);
	if (pNearbts == NULL) {
		ntylog("ntyJsonLAB --> malloc failed\n");

		pLABReq->size = 0;
		return ;
	}
	memset(pNearbts, 0, pLABReq->size*sizeof(Nearbts));
	
	pLABReq->lab.pNearbts = pNearbts;
	size_t i;
	for (i = 0; i < pLABReq->size; i++) {
		nearbts_object = json_array_get_object(nearbts_array, i);
		pNearbts[i].cell = json_object_get_string(nearbts_object, NATTY_USER_PROTOCOL_CELL);
		pNearbts[i].signal = json_object_get_string(nearbts_object, NATTY_USER_PROTOCOL_SIGNAL);
		ntylog("cell:%s   signal:%s\n", pNearbts[i].cell, pNearbts[i].signal);
	}
}

void ntyJsonLABItemRelease(Nearbts *pNearbts) {
	if (pNearbts != NULL) {
		free(pNearbts);
	}
}

void ntyJsonAMap(JSON_Value *json, AMap *pAMap) {
	if (json == NULL || pAMap == NULL) {
		ntylog("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pAMap->status = json_object_get_string(root_object, NATTY_AMAP_PROTOCOL_STATUS);
	pAMap->info = json_object_get_string(root_object, NATTY_AMAP_PROTOCOL_INFO);
	pAMap->infocode = json_object_get_string(root_object, NATTY_AMAP_PROTOCOL_INFOCODE);
	
	JSON_Object *result_object = json_object_get_object(root_object, NATTY_AMAP_PROTOCOL_RESULT);
	pAMap->result.type = json_object_get_string(result_object, NATTY_AMAP_PROTOCOL_TYPE);
	pAMap->result.radius = json_object_get_string(result_object, NATTY_AMAP_PROTOCOL_RADIUS);
	pAMap->result.location = json_object_get_string(result_object, NATTY_AMAP_PROTOCOL_LOCATION);
	pAMap->result.desc = json_object_get_string(result_object, NATTY_AMAP_PROTOCOL_DESC);
	pAMap->result.country = json_object_get_string(result_object, NATTY_AMAP_PROTOCOL_COUNTRY);
	pAMap->result.city = json_object_get_string(result_object, NATTY_AMAP_PROTOCOL_CITY);
	pAMap->result.citycode = json_object_get_string(result_object, NATTY_AMAP_PROTOCOL_CITYCODE);
	pAMap->result.adcode = json_object_get_string(result_object, NATTY_AMAP_PROTOCOL_ADCODE);
	pAMap->result.road = json_object_get_string(result_object, NATTY_AMAP_PROTOCOL_ROAD);
	pAMap->result.street = json_object_get_string(result_object, NATTY_AMAP_PROTOCOL_STREET);
	pAMap->result.poi = json_object_get_string(result_object, NATTY_AMAP_PROTOCOL_POI);
	
	//ntydbg("status:%s,  info:%s,  infocode:%s,  location:%s\n", pAMap->status, pAMap->info, pAMap->infocode, pAMap->result.location);

	if (pAMap->result.location != NULL) {
		if (strcmp(pAMap->result.location, "(null)") == 0) {
			pAMap->result.location = NULL;
		}
	}
}

void ntyJsonWeatherLocation(JSON_Value *json, WeatherLocationReq *pWeatherLocationReq) {
	if (json == NULL || pWeatherLocationReq == NULL) {
		ntylog("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pWeatherLocationReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pWeatherLocationReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	pWeatherLocationReq->bts = json_object_get_string(root_object, NATTY_USER_PROTOCOL_BTS);
	
	//ntydbg("IMEI:%s,  category:%s,  bts:%s\n", pWeatherLocationReq->IMEI, pWeatherLocationReq->category, pWeatherLocationReq->bts);
}

void ntyJsonWeather(JSON_Value *json, WeatherReq *pWeatherReq) {
	if (json == NULL || pWeatherReq == NULL) {
		ntylog("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);

	JSON_Object *results_object = NULL;
	JSON_Array *results_array = json_object_get_array(root_object, NATTY_WEATHER_PROTOCOL_RESULTS);
	pWeatherReq->size = json_array_get_count(results_array);
	WeatherResults *pWeatherResults = malloc(sizeof(WeatherResults)*pWeatherReq->size);
	pWeatherReq->pResults = pWeatherResults;
	size_t i;
	for (i = 0; i < pWeatherReq->size; i++) {
		results_object = json_array_get_object(results_array, i);
		pWeatherResults[i].last_update = json_object_get_string(results_object, NATTY_WEATHER_PROTOCOL_LAST_UPDATE);
		
		JSON_Object *location_object = json_object_get_object(results_object, NATTY_WEATHER_PROTOCOL_LOCATION);
		pWeatherResults[i].location.id = json_object_get_string(location_object, NATTY_WEATHER_PROTOCOL_ID);
		pWeatherResults[i].location.name = json_object_get_string(location_object, NATTY_WEATHER_PROTOCOL_NAME);
		pWeatherResults[i].location.country = json_object_get_string(location_object, NATTY_WEATHER_PROTOCOL_COUNTRY);
		pWeatherResults[i].location.path = json_object_get_string(location_object, NATTY_WEATHER_PROTOCOL_PATH);
		pWeatherResults[i].location.timezone = json_object_get_string(location_object, NATTY_WEATHER_PROTOCOL_TIMEZONE);
		pWeatherResults[i].location.timezone_offset = json_object_get_string(location_object, NATTY_WEATHER_PROTOCOL_TIMEZONE_OFFSET);

		JSON_Object *daily_object = NULL;
		JSON_Array *daily_array = json_object_get_array(results_object, NATTY_WEATHER_PROTOCOL_DAILY);
		pWeatherResults[i].size = json_array_get_count(daily_array);
		WeatherDaily *pWeatherDaily = malloc(sizeof(WeatherDaily)*pWeatherResults[i].size);
		pWeatherResults[i].pDaily = pWeatherDaily;
	
		size_t j;
		for (j = 0; j < pWeatherResults[i].size; j++) {
			daily_object = json_array_get_object(daily_array, j);
			pWeatherDaily[j].date = json_object_get_string(daily_object, NATTY_WEATHER_PROTOCOL_DATE);
			pWeatherDaily[j].text_day = json_object_get_string(daily_object, NATTY_WEATHER_PROTOCOL_TEXT_DAY);
			pWeatherDaily[j].code_day = json_object_get_string(daily_object, NATTY_WEATHER_PROTOCOL_CODE_DAY);
			pWeatherDaily[j].text_night = json_object_get_string(daily_object, NATTY_WEATHER_PROTOCOL_TEXT_NIGHT);
			pWeatherDaily[j].code_night = json_object_get_string(daily_object, NATTY_WEATHER_PROTOCOL_CODE_NIGHT);
			pWeatherDaily[j].high = json_object_get_string(daily_object, NATTY_WEATHER_PROTOCOL_HIGH);
			pWeatherDaily[j].low = json_object_get_string(daily_object, NATTY_WEATHER_PROTOCOL_LOW);
			pWeatherDaily[j].precip = json_object_get_string(daily_object, NATTY_WEATHER_PROTOCOL_PRECIP);
			pWeatherDaily[j].wind_direction = json_object_get_string(daily_object, NATTY_WEATHER_PROTOCOL_WIND_DIRECTION);
			pWeatherDaily[j].wind_direction_degree = json_object_get_string(daily_object, NATTY_WEATHER_PROTOCOL_WIND_DIRECTION_DEGREE);
			pWeatherDaily[j].wind_speed = json_object_get_string(daily_object, NATTY_WEATHER_PROTOCOL_WIND_SPEED);
			pWeatherDaily[j].wind_scale = json_object_get_string(daily_object, NATTY_WEATHER_PROTOCOL_WIND_SCALE);
		}
	}
}


void ntyJsonWeatherRelease(WeatherReq *pWeatherReq) {
	if (pWeatherReq != NULL) {
		size_t i;
		for (i = 0; i<pWeatherReq->size; i++) {
			if (pWeatherReq->pResults[i].pDaily != NULL) {
				free(pWeatherReq->pResults[i].pDaily);
			}
		}
		free(pWeatherReq->pResults);
	}
}

void ntyJsonICCID(JSON_Value *json, ICCIDReq *pICCIDReq) {
	if (json == NULL || pICCIDReq == NULL) {
		ntylog("param is null.\n");
		return;
	}
	
	JSON_Object *root_object = json_value_get_object(json);
	pICCIDReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pICCIDReq->ICCID = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ICCID);
	
	//ntydbg("IMEI:%s\n", pICCIDReq->IMEI);
}

void ntyJsonICCIDSet(JSON_Value *json, ICCIDSet *pICCIDSet) {
	if (json == NULL || pICCIDSet == NULL) {
		ntylog("param is null.\n");
		return;
	}
	
	JSON_Object *root_object = json_value_get_object(json);
	pICCIDSet->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pICCIDSet->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	pICCIDSet->ICCID = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ICCID);
	pICCIDSet->phone_num = json_object_get_string(root_object, NATTY_USER_PROTOCOL_PHONENUM);
}

void ntyJsonCommon(JSON_Value *json, CommonReq *pCommonReq) {
	if (json == NULL || pCommonReq == NULL) {
		ntylog("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pCommonReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pCommonReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	pCommonReq->action = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ACTION);
}

void ntyJsonCommonExtend(JSON_Value *json, CommonReqExtend *pCommonReqExtend) {
	if (json == NULL || pCommonReqExtend == NULL) {
		ntylog("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pCommonReqExtend->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pCommonReqExtend->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	pCommonReqExtend->action = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ACTION);
	pCommonReqExtend->id = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ID);
}

void ntyJsonAddEfence(JSON_Value *json, AddEfenceReq *pAddEfenceReq) {
	if (json == NULL || pAddEfenceReq == NULL) {
		ntylog("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pAddEfenceReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pAddEfenceReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	pAddEfenceReq->action = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ACTION);
	pAddEfenceReq->index = json_object_get_string(root_object, NATTY_USER_PROTOCOL_INDEX);
	
	JSON_Object *efence_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL_EFENCE);
	pAddEfenceReq->efence.num = json_object_get_string(efence_object, NATTY_USER_PROTOCOL_NUM);

	JSON_Object *point_object = NULL;
	JSON_Array *points_array = json_object_get_array(efence_object, NATTY_USER_PROTOCOL_POINTS);
	size_t points_count = json_array_get_count(points_array);
	EfencePoints *pPoints = malloc(sizeof(EfencePoints)*points_count);
	pAddEfenceReq->efence.pPoints = pPoints;
	pAddEfenceReq->efence.size = points_count;
	
	size_t i;
	for (i = 0; i < points_count; i++) {
		pPoints[i].point = json_array_get_string(points_array, i);
		//ntydbg("point:%s\n", pPoints[i].point);
	}
}

void ntyJsonAddEfencePointRelease(EfencePoints *pPoints) {
	if (pPoints != NULL) {
		free(pPoints);
	}
}

void ntyJsonDelEfence(JSON_Value *json, DelEfenceReq *pDelEfenceReq) {
	if (json == NULL || pDelEfenceReq == NULL) {
		ntylog("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pDelEfenceReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pDelEfenceReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	pDelEfenceReq->action = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ACTION);
	pDelEfenceReq->index = json_object_get_string(root_object, NATTY_USER_PROTOCOL_INDEX);
}

void ntyJsonRuntime(JSON_Value *json, RunTimeReq *pRunTimeReq) {
	if (json == NULL || pRunTimeReq == NULL) {
		ntylog("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pRunTimeReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pRunTimeReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	//ntydbg("IMEI:%s   Category:%s\n", pRunTimeReq->IMEI, pRunTimeReq->category);
	
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
		ntylog("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pTurnReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pTurnReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	//ntydbg("IMEI:%s   Category:%s\n", pTurnReq->IMEI, pTurnReq->category);
	
	JSON_Object *turn_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL_TURN);
	JSON_Object *on_object = json_object_get_object(turn_object, NATTY_USER_PROTOCOL_ON);
	JSON_Object *off_object = json_object_get_object(turn_object, NATTY_USER_PROTOCOL_OFF);
	pTurnReq->turn.status = json_object_get_string(turn_object, NATTY_USER_PROTOCOL_STATUS);
	pTurnReq->turn.on.time = json_object_get_string(on_object, NATTY_USER_PROTOCOL_TIME);
	pTurnReq->turn.off.time = json_object_get_string(off_object, NATTY_USER_PROTOCOL_TIME);
}

void ntyJsonAddSchedule(JSON_Value *json, AddScheduleReq *pAddScheduleReq) {
	if (json == NULL || pAddScheduleReq == NULL) {
		ntylog("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pAddScheduleReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pAddScheduleReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	pAddScheduleReq->action = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ACTION);
	//ntydbg("IMEI:%s   Category:%s  Action:%s\n", pAddScheduleReq->IMEI, pAddScheduleReq->category, pAddScheduleReq->action);

	JSON_Object *schedule_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL_SCHEDULE);
	pAddScheduleReq->schedule.daily = json_object_get_string(schedule_object, NATTY_USER_PROTOCOL_DAILY);
	pAddScheduleReq->schedule.time = json_object_get_string(schedule_object, NATTY_USER_PROTOCOL_TIME);
	pAddScheduleReq->schedule.details = json_object_get_string(schedule_object, NATTY_USER_PROTOCOL_DETAILS);
	pAddScheduleReq->schedule.status = json_object_get_string(schedule_object, NATTY_USER_PROTOCOL_STATUS);
	//ntydbg("Daily:%s, Time:%s, Details:%s\n", pAddScheduleReq->schedule.daily, pAddScheduleReq->schedule.time, pAddScheduleReq->schedule.details);
}

void ntyJsonUpdateSchedule(JSON_Value *json, UpdateScheduleReq *pUpdateScheduleReq) {
	if (json == NULL || pUpdateScheduleReq == NULL) {
		ntylog("param is null.\n");
		return;
	}
	
	JSON_Object *root_object = json_value_get_object(json);
	pUpdateScheduleReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pUpdateScheduleReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	pUpdateScheduleReq->action = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ACTION);
	pUpdateScheduleReq->id = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ID);
	//ntydbg("IMEI:%s   Category:%s  Action:%s  id:%s\n", pUpdateScheduleReq->IMEI, pUpdateScheduleReq->category, pUpdateScheduleReq->action, pUpdateScheduleReq->id);

	JSON_Object *schedule_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL_SCHEDULE);
	pUpdateScheduleReq->schedule.daily = json_object_get_string(schedule_object, NATTY_USER_PROTOCOL_DAILY);
	pUpdateScheduleReq->schedule.time = json_object_get_string(schedule_object, NATTY_USER_PROTOCOL_TIME);
	pUpdateScheduleReq->schedule.details = json_object_get_string(schedule_object, NATTY_USER_PROTOCOL_DETAILS);
	pUpdateScheduleReq->schedule.status = json_object_get_string(schedule_object, NATTY_USER_PROTOCOL_STATUS);
	//ntydbg("Daily:%s, Time:%s, Details:%s\n", pUpdateScheduleReq->schedule.daily, pUpdateScheduleReq->schedule.time, pUpdateScheduleReq->schedule.details);
}

void ntyJsonDelSchedule(JSON_Value *json, DelScheduleReq *pDelScheduleReq) {
	if (json == NULL || pDelScheduleReq == NULL) {
		ntylog("param is null.\n");
		return;
	}
	
	JSON_Object *root_object = json_value_get_object(json);
	pDelScheduleReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pDelScheduleReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	pDelScheduleReq->action = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ACTION);
	pDelScheduleReq->id = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ID);
	//ntydbg("IMEI:%s   Category:%s  Action:%s  id:%s\n", pDelScheduleReq->IMEI, pDelScheduleReq->category, pDelScheduleReq->action, pDelScheduleReq->id);
}

void ntyJsonTimeTables(JSON_Value *json, TimeTablesReq *pTimeTablesReq) {
	if (json == NULL || pTimeTablesReq == NULL) {
		ntylog("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pTimeTablesReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pTimeTablesReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	//ntydbg("IMEI:%s   Category:%s\n", pTimeTablesReq->IMEI, pTimeTablesReq->category);

	JSON_Object *timetables_object = NULL;
	JSON_Object *morning_object = NULL;
	JSON_Object *afternoon_object = NULL;
	JSON_Array *timetables_array = json_object_get_array(root_object, NATTY_USER_PROTOCOL_TIMETABLES);
	pTimeTablesReq->size = json_array_get_count(timetables_array);
	TimeTablesItem *pTimeTables = malloc(sizeof(TimeTablesItem)*pTimeTablesReq->size);
	pTimeTablesReq->pTimeTables = pTimeTables;

	size_t i,j;
	for (i = 0; i < pTimeTablesReq->size; i++) {
		timetables_object = json_array_get_object(timetables_array, i);
		pTimeTables[i].daily = json_object_get_string(timetables_object, NATTY_USER_PROTOCOL_DAILY);
		morning_object = json_object_get_object(timetables_object, NATTY_USER_PROTOCOL_MORNING);
		pTimeTables[i].morning.status = json_object_get_string(morning_object, NATTY_USER_PROTOCOL_STATUS);
		pTimeTables[i].morning.startTime = json_object_get_string(morning_object, NATTY_USER_PROTOCOL_STARTTIME);
		pTimeTables[i].morning.endTime = json_object_get_string(morning_object, NATTY_USER_PROTOCOL_ENDTIME);
		afternoon_object = json_object_get_object(timetables_object, NATTY_USER_PROTOCOL_AFTERNOON);
		pTimeTables[i].afternoon.status = json_object_get_string(afternoon_object, NATTY_USER_PROTOCOL_STATUS);
		pTimeTables[i].afternoon.startTime = json_object_get_string(afternoon_object, NATTY_USER_PROTOCOL_STARTTIME);
		pTimeTables[i].afternoon.endTime = json_object_get_string(afternoon_object, NATTY_USER_PROTOCOL_ENDTIME);
	}
}

void ntyJsonAddContacts(JSON_Value *json, AddContactsReq *pAddContactsReq) {
	if (json == NULL || pAddContactsReq == NULL) {
		ntylog("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pAddContactsReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pAddContactsReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	pAddContactsReq->action = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ACTION);

	JSON_Object *contacts_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL_CONTACTS);
	pAddContactsReq->contacts.name = json_object_get_string(contacts_object, NATTY_USER_PROTOCOL_NAME);
	pAddContactsReq->contacts.image = json_object_get_string(contacts_object, NATTY_USER_PROTOCOL_IMAGE);
	pAddContactsReq->contacts.telphone = json_object_get_string(contacts_object, NATTY_USER_PROTOCOL_TELPHONE);
	//pAddContactsReq->contacts.app = json_object_get_string(contacts_object, NATTY_USER_PROTOCOL_APP);
	//pAddContactsReq->contacts.admin = json_object_get_string(contacts_object, NATTY_USER_PROTOCOL_ADMIN);
}

void ntyJsonUpdateContacts(JSON_Value *json, UpdateContactsReq *pUpdateContactsReq) {
	if (json == NULL || pUpdateContactsReq == NULL) {
		ntylog("param is null.\n");
		return;
	}
	
	JSON_Object *root_object = json_value_get_object(json);
	pUpdateContactsReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pUpdateContactsReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	pUpdateContactsReq->action = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ACTION);

	JSON_Object *contacts_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL_CONTACTS);
	pUpdateContactsReq->contacts.id = json_object_get_string(contacts_object, NATTY_USER_PROTOCOL_ID);
	pUpdateContactsReq->contacts.name = json_object_get_string(contacts_object, NATTY_USER_PROTOCOL_NAME);
	pUpdateContactsReq->contacts.image = json_object_get_string(contacts_object, NATTY_USER_PROTOCOL_IMAGE);
	pUpdateContactsReq->contacts.telphone = json_object_get_string(contacts_object, NATTY_USER_PROTOCOL_TELPHONE);
	//pUpdateContactsReq->contacts.app = json_object_get_string(contacts_object, NATTY_USER_PROTOCOL_APP);
	//pUpdateContactsReq->contacts.admin = json_object_get_string(contacts_object, NATTY_USER_PROTOCOL_ADMIN);
}

void ntyJsonDelContacts(JSON_Value *json, DelContactsReq *pDelContactsReq) {
	if (json == NULL || pDelContactsReq == NULL) {
		ntylog("param is null.\n");
		return;
	}
	
	JSON_Object *root_object = json_value_get_object(json);
	pDelContactsReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pDelContactsReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	pDelContactsReq->action = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ACTION);
	pDelContactsReq->id = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ID);
}


void ntyJsonLocationReport(JSON_Value *json,  LocationReport *pLocationReport) {
	if (json == NULL || pLocationReport == NULL) {
		ntylog("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	JSON_Object *results_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL_RESULTS);
	pLocationReport->results.IMEI = json_object_get_string(results_object, NATTY_USER_PROTOCOL_IMEI);
	pLocationReport->results.category = json_object_get_string(results_object, NATTY_USER_PROTOCOL_CATEGORY);
	JSON_Object *locationreport_object = json_object_get_object(results_object, NATTY_USER_PROTOCOL_LOCATIONREPORT);
	pLocationReport->results.locationReport.location = json_object_get_string(locationreport_object, NATTY_USER_PROTOCOL_LOCATION);
	pLocationReport->results.locationReport.radius = json_object_get_string(locationreport_object, NATTY_USER_PROTOCOL_RADIUS);
	pLocationReport->results.locationReport.type = json_object_get_string(locationreport_object, NATTY_USER_PROTOCOL_TYPE);
}

void ntyJsonStepsReport(JSON_Value *json,  StepsReport *pStepsReport) {
	if (json == NULL || pStepsReport == NULL) {
		ntylog("param is null.\n");
		return;
	}
	
	JSON_Object *root_object = json_value_get_object(json);
	JSON_Object *results_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL_RESULTS);
	pStepsReport->results.IMEI  = json_object_get_string(results_object, NATTY_USER_PROTOCOL_IMEI);
	pStepsReport->results.category = json_object_get_string(results_object, NATTY_USER_PROTOCOL_CATEGORY);
	pStepsReport->results.stepsReport = json_object_get_string(results_object, NATTY_USER_PROTOCOL_STEPSREPORT);
	pStepsReport->results.time = json_object_get_string(results_object, NATTY_USER_PROTOCOL_TIME);
}

void ntyJsonHeartReport(JSON_Value *json,  HeartReport *pHeartReport) {
	if (json == NULL || pHeartReport == NULL) {
		ntylog("param is null.\n");
		return;
	}
	
	JSON_Object *root_object = json_value_get_object(json);
	JSON_Object *results_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL_RESULTS);
	pHeartReport->results.IMEI  = json_object_get_string(results_object, NATTY_USER_PROTOCOL_IMEI);
	pHeartReport->results.category = json_object_get_string(results_object, NATTY_USER_PROTOCOL_CATEGORY);
	pHeartReport->results.heartReport = json_object_get_string(results_object, NATTY_USER_PROTOCOL_HEARTREPORT);
}


char * ntyJsonBind(JSON_Value *json, BindReq *pBindReq) {
	if (json == NULL || pBindReq == NULL) {
		ntylog("param is null.\n");
		return;
	}
	
	JSON_Object *root_object = json_value_get_object(json);
	pBindReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pBindReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);

	JSON_Object *bindReq_object = json_object_get_object(root_object, NATTY_USER_PROTOCOL_BINGREQ);
	pBindReq->bind.watchName = json_object_get_string(bindReq_object, NATTY_USER_PROTOCOL_WATCHNAME);
	pBindReq->bind.watchImage = json_object_get_string(bindReq_object, NATTY_USER_PROTOCOL_WATCHIMAGE);
	pBindReq->bind.userImage= json_object_get_string(bindReq_object, NATTY_USER_PROTOCOL_USERIMAGE);
	pBindReq->bind.userName= json_object_get_string(bindReq_object, NATTY_USER_PROTOCOL_USERNAME);
}

void ntyJsonBindConfirmReq(JSON_Value *json, BindConfirmReq *pBindConfirmReq) {
	if (json == NULL || pBindConfirmReq == NULL) {
		ntylog("param is null.\n");
		return;
	}

	JSON_Object *root_object = json_value_get_object(json);
	pBindConfirmReq->IMEI = json_object_get_string(root_object, NATTY_USER_PROTOCOL_IMEI);
	pBindConfirmReq->category = json_object_get_string(root_object, NATTY_USER_PROTOCOL_CATEGORY);
	pBindConfirmReq->answer = json_object_get_string(root_object, NATTY_USER_PROTOCOL_ANSWER);
	pBindConfirmReq->msgId = json_object_get_string(root_object, NATTY_USER_PROTOCOL_MSGID);
}

void ntyJsonTimeTablesItemRelease(TimeTablesItem *pTimeTables) {
	if (pTimeTables != NULL) {
		free(pTimeTables);
	}
}

char * ntyJsonWriteICCID(ICCIDAck *pICCIDAck) {
	if (pICCIDAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_IMEI, pICCIDAck->IMEI);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_PHONENUM, pICCIDAck->phone_num);
	
	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}


char * ntyJsonWriteCommon(CommonAck *pCommonAck) {
	if (pCommonAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *result_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(result_obj, NATTY_USER_PROTOCOL_CODE, pCommonAck->result.code);
	json_object_set_string(result_obj, NATTY_USER_PROTOCOL_MESSAGE, pCommonAck->result.message);
	
	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);

	ntylog("ntyJsonWriteCommon --> %s\n", jsonstring);
	
	return jsonstring;
}

char * ntyJsonWriteCommonExtend(CommonExtendAck *pCommonExtendAck) {
	if (pCommonExtendAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_ID, pCommonExtendAck->id);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *result_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(result_obj, NATTY_USER_PROTOCOL_CODE, pCommonExtendAck->result.code);
	json_object_set_string(result_obj, NATTY_USER_PROTOCOL_MESSAGE, pCommonExtendAck->result.message);
	
	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}

char * ntyJsonWriteCommonReqExtend(CommonReqExtend *pCommonReqExtend) {
	if (pCommonReqExtend == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_IMEI, pCommonReqExtend->IMEI);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_CATEGORY, pCommonReqExtend->category);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_ACTION, pCommonReqExtend->action);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_ID, pCommonReqExtend->id);
	
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
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, pLocationAck->results.IMEI);

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}

char *ntyJsonWriteWeather(WeatherAck *pWeatherAck) {
	if (pWeatherAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_WEATHER_PROTOCOL_RESULTS, json_value_init_array());
	JSON_Array *results_arr = json_object_get_array(schema_obj, NATTY_WEATHER_PROTOCOL_RESULTS);
	size_t i,j;
	for (i = 0; i < pWeatherAck->size; i++) {
		json_array_append_value(results_arr, json_value_init_object());
		JSON_Object *results_obj = json_array_get_object(results_arr, i);

		json_object_set_value(results_obj, NATTY_WEATHER_PROTOCOL_LOCATION, json_value_init_object());
		JSON_Object *location_obj = json_object_get_object(results_obj, NATTY_WEATHER_PROTOCOL_LOCATION);
		json_object_set_string(location_obj, NATTY_WEATHER_PROTOCOL_NAME, pWeatherAck->pResults[i].location.name);

		json_object_set_value(results_obj, NATTY_WEATHER_PROTOCOL_DAILY, json_value_init_array());
		JSON_Array *daily_arr = json_object_get_array(results_obj, NATTY_WEATHER_PROTOCOL_DAILY);
		for (j = 0; j < pWeatherAck->pResults[i].size; j++) {
			json_array_append_value(daily_arr, json_value_init_object());
			JSON_Object *daily_obj = json_array_get_object(daily_arr, j);
			json_object_set_string(daily_obj, NATTY_WEATHER_PROTOCOL_DATE, pWeatherAck->pResults[i].pDaily[j].date);
			//json_object_set_string(daily_obj, NATTY_WEATHER_PROTOCOL_TEXT_DAY, pWeatherAck->pResults[i].pDaily[j].text_day);
			json_object_set_string(daily_obj, NATTY_WEATHER_PROTOCOL_CODE_DAY, pWeatherAck->pResults[i].pDaily[j].code_day);
			//json_object_set_string(daily_obj, NATTY_WEATHER_PROTOCOL_TEXT_NIGHT, pWeatherAck->pResults[i].pDaily[j].text_night);
			json_object_set_string(daily_obj, NATTY_WEATHER_PROTOCOL_CODE_NIGHT, pWeatherAck->pResults[i].pDaily[j].code_night);
			json_object_set_string(daily_obj, NATTY_WEATHER_PROTOCOL_HIGH, pWeatherAck->pResults[i].pDaily[j].high);
			json_object_set_string(daily_obj, NATTY_WEATHER_PROTOCOL_LOW, pWeatherAck->pResults[i].pDaily[j].low);
		}
	}

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	
	return jsonstring;
}

char * ntyJsonWriteAddEfence(AddEfenceAck *pAddEfenceAck) {
	if (pAddEfenceAck == NULL) {
		return NULL;
	}
	
	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);

	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_ID, pAddEfenceAck->id);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_MSG, pAddEfenceAck->result.msg);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, pAddEfenceAck->result.IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_CATEGORY, pAddEfenceAck->result.category);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_ACTION, pAddEfenceAck->result.action);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_INDEX, pAddEfenceAck->result.index);
	
	json_object_set_value(results_obj, NATTY_USER_PROTOCOL_EFENCE, json_value_init_object());
	JSON_Object *efence_obj = json_object_get_object(results_obj, NATTY_USER_PROTOCOL_EFENCE);
	json_object_set_string(efence_obj, NATTY_USER_PROTOCOL_NUM, pAddEfenceAck->result.efence.num);
	
	json_object_set_value(efence_obj, NATTY_USER_PROTOCOL_POINTS, json_value_init_array());
	JSON_Array *points_arr = json_object_get_array(efence_obj, NATTY_USER_PROTOCOL_POINTS);
	
	size_t i;
	for (i = 0; i < pAddEfenceAck->result.efence.size; i++) {
		json_array_append_string(points_arr, pAddEfenceAck->result.efence.pPoints[i].point);
	}

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}

char * ntyJsonWriteDelEfence(DelEfenceAck *pDelEfenceAck) {
	if (pDelEfenceAck == NULL) {
		return NULL;
	}
	
	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);

	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, pDelEfenceAck->result.IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_CATEGORY, pDelEfenceAck->result.category);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_ACTION, pDelEfenceAck->result.action);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_INDEX, pDelEfenceAck->result.index);

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

char * ntyJsonWriteTurn(TurnAck *pTurnAck) {
	if (pTurnAck == NULL) {
		return NULL;
	}
	
	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_MSG, pTurnAck->result.msg);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, pTurnAck->result.IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_CATEGORY, pTurnAck->result.category);
	json_object_set_value(results_obj, NATTY_USER_PROTOCOL_TURN, json_value_init_object());
	JSON_Object *turn_obj = json_object_get_object(results_obj, NATTY_USER_PROTOCOL_TURN);
	json_object_set_value(turn_obj, NATTY_USER_PROTOCOL_ON, json_value_init_object());
	JSON_Object *on_obj = json_object_get_object(turn_obj, NATTY_USER_PROTOCOL_ON);
	json_object_set_value(turn_obj, NATTY_USER_PROTOCOL_OFF, json_value_init_object());
	JSON_Object *off_obj = json_object_get_object(turn_obj, NATTY_USER_PROTOCOL_OFF);
	json_object_set_string(turn_obj, NATTY_USER_PROTOCOL_STATUS, pTurnAck->result.turn.status);
	json_object_set_string(on_obj, NATTY_USER_PROTOCOL_TIME, pTurnAck->result.turn.on.time);
	json_object_set_string(off_obj, NATTY_USER_PROTOCOL_TIME, pTurnAck->result.turn.off.time);

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}


char * ntyJsonWriteDeviceAddSchedule(DeviceAddScheduleAck *pDeviceAddScheduleAck) {
	if (pDeviceAddScheduleAck == NULL) {
		return NULL;
	}
	
	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_MSG, pDeviceAddScheduleAck->msg);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_IMEI, pDeviceAddScheduleAck->IMEI);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_CATEGORY, pDeviceAddScheduleAck->category);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_ACTION, pDeviceAddScheduleAck->action);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_ID, pDeviceAddScheduleAck->id);

	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_SCHEDULE, json_value_init_object());
	JSON_Object *schedule_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_SCHEDULE);
	json_object_set_string(schedule_obj, NATTY_USER_PROTOCOL_DAILY, pDeviceAddScheduleAck->schedule.daily);
	json_object_set_string(schedule_obj, NATTY_USER_PROTOCOL_TIME, pDeviceAddScheduleAck->schedule.time);
	json_object_set_string(schedule_obj, NATTY_USER_PROTOCOL_DETAILS, pDeviceAddScheduleAck->schedule.details);
	json_object_set_string(schedule_obj, NATTY_USER_PROTOCOL_STATUS, pDeviceAddScheduleAck->schedule.status);
	
	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}

char * ntyJsonWriteDeviceAddContacts(DeviceAddContactsAck *pDeviceAddContactsAck) {
	if (pDeviceAddContactsAck == NULL) {
		return NULL;
	}
	
	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_MSG, pDeviceAddContactsAck->msg);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_IMEI, pDeviceAddContactsAck->IMEI);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_CATEGORY, pDeviceAddContactsAck->category);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_ACTION, pDeviceAddContactsAck->action);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_ID, pDeviceAddContactsAck->id);

	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_CONTACTS, json_value_init_object());
	JSON_Object *contacts_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_CONTACTS);
	json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_ID, pDeviceAddContactsAck->contacts.id);
	json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_NAME, pDeviceAddContactsAck->contacts.name);
	json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_IMAGE, pDeviceAddContactsAck->contacts.image);
	json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_TELPHONE, pDeviceAddContactsAck->contacts.telphone);
	
	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}

char * ntyJsonWriteAddSchedule(AddScheduleAck *pAddScheduleAck) {
	if (pAddScheduleAck == NULL) {
		return NULL;
	}
	
	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, pAddScheduleAck->result.IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_CATEGORY, pAddScheduleAck->result.category);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_ACTION, pAddScheduleAck->result.action);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_ID, pAddScheduleAck->result.id);

	json_object_set_value(results_obj, NATTY_USER_PROTOCOL_SCHEDULE, json_value_init_object());
	JSON_Object *schedule_obj = json_object_get_object(results_obj, NATTY_USER_PROTOCOL_SCHEDULE);
	json_object_set_string(schedule_obj, NATTY_USER_PROTOCOL_DAILY, pAddScheduleAck->result.schedule.daily);
	json_object_set_string(schedule_obj, NATTY_USER_PROTOCOL_TIME, pAddScheduleAck->result.schedule.time);
	json_object_set_string(schedule_obj, NATTY_USER_PROTOCOL_DETAILS, pAddScheduleAck->result.schedule.details);
	json_object_set_string(schedule_obj, NATTY_USER_PROTOCOL_STATUS, pAddScheduleAck->result.schedule.status);

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}

char * ntyJsonWriteDelSchedule(DelScheduleAck *pDelScheduleAck) {
	if (pDelScheduleAck == NULL) {
		return NULL;
	}
	
	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, pDelScheduleAck->result.IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_CATEGORY, pDelScheduleAck->result.category);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_ACTION, pDelScheduleAck->result.action);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_ID, pDelScheduleAck->result.id);

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}

char * ntyJsonWriteUpdateSchedule(UpdateScheduleAck *pUpdateScheduleAck) {
	if (pUpdateScheduleAck == NULL) {
		return NULL;
	}
	
	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, pUpdateScheduleAck->result.IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_CATEGORY, pUpdateScheduleAck->result.category);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_ACTION, pUpdateScheduleAck->result.action);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_ID, pUpdateScheduleAck->result.id);

	json_object_set_value(results_obj, NATTY_USER_PROTOCOL_SCHEDULE, json_value_init_object());
	JSON_Object *schedule_obj = json_object_get_object(results_obj, NATTY_USER_PROTOCOL_SCHEDULE);
	json_object_set_string(schedule_obj, NATTY_USER_PROTOCOL_DAILY, pUpdateScheduleAck->result.schedule.daily);
	json_object_set_string(schedule_obj, NATTY_USER_PROTOCOL_TIME, pUpdateScheduleAck->result.schedule.time);
	json_object_set_string(schedule_obj, NATTY_USER_PROTOCOL_DETAILS, pUpdateScheduleAck->result.schedule.details);
	json_object_set_string(schedule_obj, NATTY_USER_PROTOCOL_STATUS, pUpdateScheduleAck->result.schedule.status);

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
		//json_object_set_string(timetables_obj, NATTY_USER_PROTOCOL_ID, pTimeTablesAck->results.pTimeTables[i].id);
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


char * ntyJsonWriteAddContacts(AddContactsAck *pAddContactsAck) {
	if (pAddContactsAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, pAddContactsAck->results.IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_CATEGORY, pAddContactsAck->results.category);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_ACTION, pAddContactsAck->results.action);

	json_object_set_value(results_obj, NATTY_USER_PROTOCOL_CONTACTS, json_value_init_object());
	JSON_Object *contacts_obj = json_object_get_object(results_obj, NATTY_USER_PROTOCOL_CONTACTS);
	json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_ID, pAddContactsAck->results.contacts.id);
	json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_NAME, pAddContactsAck->results.contacts.name);
	json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_IMAGE, pAddContactsAck->results.contacts.image);
	json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_TELPHONE, pAddContactsAck->results.contacts.telphone);

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}

/*
char * ntyJsonWriteWatchAddContacts(AddContactsAck *pAddContactsAck) {
	if (pAddContactsAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_IMEI, pAddContactsAck->results.IMEI);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_CATEGORY, pAddContactsAck->results.category);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_ACTION, pAddContactsAck->results.action);
	
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_CONTACTS, json_value_init_object());
	JSON_Object *contacts_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_CONTACTS);
	json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_ID, pAddContactsAck->results.contacts.id);
	json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_NAME, pAddContactsAck->results.contacts.name);
	json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_IMAGE, pAddContactsAck->results.contacts.image);
	json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_TELPHONE, pAddContactsAck->results.contacts.telphone);

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;

}
*/

char * ntyJsonWriteUpdateContacts(UpdateContactsAck *pUpdateContactsAck) {
	if (pUpdateContactsAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, pUpdateContactsAck->results.IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_CATEGORY, pUpdateContactsAck->results.category);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_ACTION, pUpdateContactsAck->results.action);

	json_object_set_value(results_obj, NATTY_USER_PROTOCOL_CONTACTS, json_value_init_object());
	JSON_Object *contacts_obj = json_object_get_object(results_obj, NATTY_USER_PROTOCOL_CONTACTS);
	json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_ID, pUpdateContactsAck->results.contacts.id);
	json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_NAME, pUpdateContactsAck->results.contacts.name);
	json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_IMAGE, pUpdateContactsAck->results.contacts.image);
	json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_TELPHONE, pUpdateContactsAck->results.contacts.telphone);

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}

char * ntyJsonWriteDelContacts(DelContactsAck *pDelContactsAck) {
	if (pDelContactsAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, pDelContactsAck->results.IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_CATEGORY, pDelContactsAck->results.category);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_ACTION, pDelContactsAck->results.action);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_ID, pDelContactsAck->results.id);
	
	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}

char * ntyJsonWriteDeviceDelContacts(DeviceDelContactsAck *pDeviceDelContactsAck) {
	if (pDeviceDelContactsAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_IMEI, pDeviceDelContactsAck->IMEI);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_CATEGORY, pDeviceDelContactsAck->category);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_ACTION, pDeviceDelContactsAck->action);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_ID, pDeviceDelContactsAck->id);
	
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
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_ACTION, pContactsAck->results.action);
	//json_object_set_string(results_obj, NATTY_USER_PROTOCOL_NUM, pContactsAck->results.num);
	json_object_set_value(results_obj, NATTY_USER_PROTOCOL_CONTACTS, json_value_init_array());
	JSON_Array *contacts_arr = json_object_get_array(results_obj, NATTY_USER_PROTOCOL_CONTACTS);
	
	size_t i;
	for (i = 0; i < pContactsAck->results.size; i++) {
		json_array_append_value(contacts_arr, json_value_init_object());
		JSON_Object *contacts_obj = json_array_get_object(contacts_arr, i);
		json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_ID, pContactsAck->results.pContacts[i].id);
		json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_NAME, pContactsAck->results.pContacts[i].name);
		json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_IMAGE, pContactsAck->results.pContacts[i].image);
		json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_TELPHONE, pContactsAck->results.pContacts[i].telphone);
		//json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_APP, pContactsAck->results.pContacts[i].app);
		//json_object_set_string(contacts_obj, NATTY_USER_PROTOCOL_ADMIN, pContactsAck->results.pContacts[i].admin);
	}

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}

char * ntyJsonWriteBindConfirmPush(BindConfirmPush *pBindConfirmPush) {
	if (pBindConfirmPush == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, pBindConfirmPush->result.IMEI);
	//json_object_set_string(results_obj, NATTY_USER_PROTOCOL_CATEGORY, pBindConfirmPush->result.category);
	//json_object_set_string(results_obj, NATTY_USER_PROTOCOL_ACTION, pBindConfirmPush->result.action);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_PROPOSER, pBindConfirmPush->result.proposer);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_USERNAME, pBindConfirmPush->result.userName);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_MSGID, pBindConfirmPush->result.msgId);

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}

char * ntyJsonWriteBindBroadCast(BindBroadCast *pBindBroadCast) {
	if (pBindBroadCast == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, pBindBroadCast->result.IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_CATEGORY, pBindBroadCast->result.category);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_PROPOSER, pBindBroadCast->result.proposer);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_ANSWER, pBindBroadCast->result.answer);

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}

char * ntyJsonWriteBindConfirmAck(BindConfirmAck *pBindConfirmAck) {
	if (pBindConfirmAck == NULL) {
		return NULL;
	}
	
	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_IMEI, pBindConfirmAck->IMEI);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_CATEGORY, pBindConfirmAck->category);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_MSGID, pBindConfirmAck->msgId);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_ANSWER, pBindConfirmAck->answer);

	char *jsonstring =	json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}

char * ntyJsonWriteBindOfflineMsgToAdmin(BindOfflineMsgToAdmin *pMsgToAdmin, char *phonenum) {
	if (pMsgToAdmin == NULL) {
		return NULL;
	}
	
	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);

	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_PROPOSER, phonenum);
	
	char imei[64] = {0};
	sprintf(imei, "%llx", pMsgToAdmin->IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, imei);
	char msgIds[64] = {0};
	sprintf(msgIds, "%ld", pMsgToAdmin->msgId);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_MSGID, msgIds);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_USERNAME, pMsgToAdmin->userName);
	char *jsonstring = json_serialize_to_string(schema);
	json_value_free(schema);
	
	return jsonstring;
}

char * ntyJsonWriteBindOfflineMsgToProposer(BindOfflineMsgToProposer *pMsgToProposer) {
	if (pMsgToProposer == NULL) {
		return NULL;
	}
	
	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);

	
	return NULL;
}

/**
 * 手表收到管理员同意的协议
 * {\"IMEI\":\"355637052788450\",\"Category\":\"BindAgree\",\"AdminId\":\"11782\",\"MsgId\":\"13453\"}
 *
 */
char * ntyJsonWriteBindAgree(BindAgreeAck *pBindAgreeAck) {
	if (pBindAgreeAck == NULL) {
		return NULL;
	}
	
	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_IMEI, pBindAgreeAck->IMEI);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_CATEGORY, pBindAgreeAck->category);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_MSGID, pBindAgreeAck->msgId);
	json_object_set_string(schema_obj, NATTY_USER_PROTOCOL_ADMINID, pBindAgreeAck->adminId);

	char *jsonstring =	json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}


//add by luoyb.add begin
/*************************************************************************************************
** Function: ntyClientReqJsonParse 
** Description: parse the json obejct to save to the struct object. 
** Input: JSON_Value *json
** Output: ClientSelectReq *pclientSelectReq
** Return: int -1:error 0:success
** Author: luoyb
** Date: 2017-04-28
** Others:
*************************************************************************************************/

int ntyClientReqJsonParse( JSON_Value *json, ClientSelectReq *pclientSelectReq ) {
	if ( json == NULL || pclientSelectReq == NULL) {
		ntylog(" ntyClientReqJsonParse param is null.\n");
		return -1;
	}
	JSON_Object *root_object = json_value_get_object( json );  //json to json object
	pclientSelectReq->IMEI = json_object_get_string( root_object, NATTY_USER_PROTOCOL_IMEI );
	pclientSelectReq->Category = json_object_get_string( root_object, NATTY_USER_PROTOCOL_CATEGORY );
	pclientSelectReq->Action = json_object_get_string( root_object, NATTY_USER_PROTOCOL_ACTION );
	return 0;
}

int ntyClientContactsAckJsonCompose( ClientContactsAck *pClientContactsAck, unsigned char *buffer) {
	if ( pClientContactsAck == NULL || buffer == NULL ) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object( schema );
	
	json_object_set_value( schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object() );
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS );
	
	json_object_set_string( results_obj, NATTY_USER_PROTOCOL_IMEI, pClientContactsAck->IMEI );
	json_object_set_string( results_obj, NATTY_USER_PROTOCOL_NUM, pClientContactsAck->Num );
	json_object_set_string( results_obj, NATTY_USER_PROTOCOL_CATEGORY, pClientContactsAck->Category );

	json_object_set_value( results_obj, NATTY_USER_PROTOCOL_CONTACTS, json_value_init_array() );
	JSON_Array *contacts_arr = json_object_get_array( results_obj, NATTY_USER_PROTOCOL_CONTACTS );
	
	int i;
	for ( i = 0; i < pClientContactsAck->size; i++ ) {
		json_array_append_value( contacts_arr, json_value_init_object() );
		JSON_Object *contacts_obj = json_array_get_object( contacts_arr, i );
		json_object_set_string( contacts_obj, NATTY_USER_PROTOCOL_ADMIN, pClientContactsAck->objClientContactsAckItem[i].Admin );
		json_object_set_string( contacts_obj, NATTY_USER_PROTOCOL_APP, pClientContactsAck->objClientContactsAckItem[i].App );
		json_object_set_string( contacts_obj, NATTY_USER_PROTOCOL_ID, pClientContactsAck->objClientContactsAckItem[i].Id );
		json_object_set_string( contacts_obj, NATTY_USER_PROTOCOL_IMAGE, pClientContactsAck->objClientContactsAckItem[i].Image );
		json_object_set_string( contacts_obj, NATTY_USER_PROTOCOL_NAME, pClientContactsAck->objClientContactsAckItem[i].Name );	
		json_object_set_string( contacts_obj, NATTY_USER_PROTOCOL_TELPHONE, pClientContactsAck->objClientContactsAckItem[i].Tel );
	}

	ntylog("ntyClientContactsAckJsonCompose --> NATTY_USER_PROTOCOL_TELPHONE\n");

	char *jsonstring =  json_serialize_to_string( schema );
	if (jsonstring == NULL) { 
		json_value_free( schema );
		return NTY_RESULT_ERROR;
	}
		
	int jsonLength = strlen(jsonstring);
	if (jsonLength > NTY_PACKET_BUFFER_SIZE-1) { 
		
		json_value_free( schema );
		ntyJsonFree(jsonstring);
		
		return NTY_RESULT_FAILED;
	}
	
	memcpy(buffer, jsonstring, jsonLength);
	
	ntylog("ntyClientContactsAckJsonCompose --> end:%s\n", buffer);
	json_value_free( schema );
	ntyJsonFree(jsonstring);

	return NTY_RESULT_SUCCESS;
}

char *ntyClientTurnAckJsonCompose( ClientTurnAck *pClientTurnAck ) {
	if ( pClientTurnAck == NULL ) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object( schema );
	json_object_set_value( schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object() );
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS );
	json_object_set_string( results_obj, NATTY_USER_PROTOCOL_IMEI, pClientTurnAck->IMEI );
	json_object_set_string( results_obj, NATTY_USER_PROTOCOL_CATEGORY, pClientTurnAck->Category );

	json_object_set_value( results_obj, NATTY_USER_PROTOCOL_TURN, json_value_init_object() );
	JSON_Object *turn_obj = json_object_get_object(results_obj, NATTY_USER_PROTOCOL_TURN );
	json_object_set_string( turn_obj, NATTY_USER_PROTOCOL_STATUS, pClientTurnAck->objClientTurnAckItem->Status);

	json_object_set_value( turn_obj, NATTY_USER_PROTOCOL_ON, json_value_init_object() );
	JSON_Object *turnOn_obj = json_object_get_object(turn_obj, NATTY_USER_PROTOCOL_ON );
	json_object_set_string( turnOn_obj, NATTY_USER_PROTOCOL_TIME, pClientTurnAck->objClientTurnAckItem->On);

	json_object_set_value( turn_obj, NATTY_USER_PROTOCOL_OFF, json_value_init_object() );
	JSON_Object *turnOff_obj = json_object_get_object(turn_obj, NATTY_USER_PROTOCOL_OFF );
	json_object_set_string( turnOff_obj, NATTY_USER_PROTOCOL_TIME, pClientTurnAck->objClientTurnAckItem->Off);


	char *jsonstring =  json_serialize_to_string( schema );
	json_value_free( schema );
	return jsonstring;
}

char *ntyClientRunTimeAckJsonCompose( ClientRunTimeAck *pClientRunTimeAck ){
	if ( pClientRunTimeAck == NULL ) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object( schema );
	json_object_set_value( schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object() );
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS );
	json_object_set_string( results_obj, NATTY_USER_PROTOCOL_IMEI, pClientRunTimeAck->IMEI );
	json_object_set_string( results_obj, NATTY_USER_PROTOCOL_CATEGORY, pClientRunTimeAck->Category );

	json_object_set_value( results_obj, NATTY_USER_PROTOCOL_RUNTIME, json_value_init_object() );
	JSON_Object *runtime_obj = json_object_get_object(results_obj, NATTY_USER_PROTOCOL_RUNTIME );
	json_object_set_string( runtime_obj, NATTY_USER_PROTOCOL_AUTOCONNECTION, pClientRunTimeAck->objClientRunTimeAckItem->AutoConnection );
	json_object_set_string( runtime_obj, NATTY_USER_PROTOCOL_LOSSREPORT, pClientRunTimeAck->objClientRunTimeAckItem->LossReport );
	json_object_set_string( runtime_obj, NATTY_USER_PROTOCOL_LIGHTPANEL, pClientRunTimeAck->objClientRunTimeAckItem->LightPanel );
	json_object_set_string( runtime_obj, NATTY_USER_PROTOCOL_WATCHBELL, pClientRunTimeAck->objClientRunTimeAckItem->WatchBell );
	json_object_set_string( runtime_obj, NATTY_USER_PROTOCOL_TAGETSTEP, pClientRunTimeAck->objClientRunTimeAckItem->TagetStep );

	char *jsonstring =  json_serialize_to_string( schema );
	json_value_free( schema );
	return jsonstring;
}

char *ntyClientTimeTablesAckJsonCompose( ClientTimeTablesAck *pClientTimeTablesAck ){
	if (pClientTimeTablesAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, pClientTimeTablesAck->IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_CATEGORY, pClientTimeTablesAck->Category);

	json_object_set_value(results_obj, NATTY_USER_PROTOCOL_TIMETABLES, json_value_init_array());
	JSON_Array *timetables_arr = json_object_get_array(results_obj, NATTY_USER_PROTOCOL_TIMETABLES);
	
	size_t i;
	for (i = 0; i < pClientTimeTablesAck->size; i++) {
		json_array_append_value(timetables_arr, json_value_init_object());
		JSON_Object *timetables_obj = json_array_get_object(timetables_arr, i);
		json_object_set_string(timetables_obj, NATTY_USER_PROTOCOL_DAILY, pClientTimeTablesAck->objClientTimeTablesAckItem[i].Daily);

		char morTimeLeft[32] = {0};
		char morTimeRight[32] = {0};
		char AftTimeLeft[32] = {0};
		char AftTimeRight[32] = {0};
		char *bufTmp = NULL;
		char *morning = pClientTimeTablesAck->objClientTimeTablesAckItem[i].Morning;
		char *afternoon = pClientTimeTablesAck->objClientTimeTablesAckItem[i].Afternoon;
		if( morning != NULL ){
			bufTmp = strstr(morning, "|");
			memcpy( morTimeLeft, morning, bufTmp-morning );
			memcpy( morTimeRight, bufTmp+1, sizeof(morTimeRight) );
		}
		if( afternoon != NULL ){
			bufTmp = strstr(afternoon, "|");
			memcpy( AftTimeLeft, afternoon, bufTmp-afternoon );
			memcpy( AftTimeRight, bufTmp+1, sizeof(AftTimeRight) );
		}
		
		json_object_set_value(timetables_obj, NATTY_USER_PROTOCOL_MORNING, json_value_init_object());
		JSON_Object *morning_obj = json_object_get_object(timetables_obj, NATTY_USER_PROTOCOL_MORNING);
		json_object_set_string(morning_obj, NATTY_USER_PROTOCOL_STATUS, pClientTimeTablesAck->objClientTimeTablesAckItem[i].MorningTurn);
		json_object_set_string(morning_obj, NATTY_USER_PROTOCOL_STARTTIME, morTimeLeft);
		json_object_set_string(morning_obj, NATTY_USER_PROTOCOL_ENDTIME, morTimeRight);

		json_object_set_value(timetables_obj, NATTY_USER_PROTOCOL_AFTERNOON, json_value_init_object());
		JSON_Object *afternoon_obj = json_object_get_object(timetables_obj, NATTY_USER_PROTOCOL_AFTERNOON);
		json_object_set_string(afternoon_obj, NATTY_USER_PROTOCOL_STATUS, pClientTimeTablesAck->objClientTimeTablesAckItem[i].AfternoonTurn);
		json_object_set_string(afternoon_obj, NATTY_USER_PROTOCOL_STARTTIME, AftTimeLeft);
		json_object_set_string(afternoon_obj, NATTY_USER_PROTOCOL_ENDTIME, AftTimeRight);
	}

	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}

/*
char *ntyClientTimeTablesAckJsonCompose( ClientTimeTablesAck *pClientTimeTablesAck ){
	if ( pClientTimeTablesAck == NULL ) {
		return NULL;
	}
	
	
	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object( schema );
	json_object_set_value( schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object() );
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS );
	json_object_set_string( results_obj, NATTY_USER_PROTOCOL_IMEI, pClientTimeTablesAck->IMEI );
	json_object_set_string( results_obj, NATTY_USER_PROTOCOL_CATEGORY, pClientTimeTablesAck->Category );

	json_object_set_value( results_obj, NATTY_USER_PROTOCOL_TIMETABLES, json_value_init_array() );
	JSON_Array *timetables_arr = json_object_get_array( results_obj, NATTY_USER_PROTOCOL_TIMETABLES );
	json_array_append_value( timetables_arr, json_value_init_object() );
	JSON_Object *timetables_obj = json_array_get_object( timetables_arr, 0 );

	//json_object_set_value( timetables_obj, NATTY_USER_PROTOCOL_TIMETABLES, json_value_init_object() );
	json_object_set_string( timetables_obj, NATTY_USER_PROTOCOL_DAILY, pClientTimeTablesAck->objClientTimeTablesAckItem->Daily );

	json_object_set_value( timetables_obj, NATTY_USER_PROTOCOL_MORNING, json_value_init_object() );
	JSON_Object *morning_obj = json_object_get_object( timetables_obj, NATTY_USER_PROTOCOL_MORNING );
	json_object_set_string( morning_obj, NATTY_USER_PROTOCOL_STATUS, pClientTimeTablesAck->objClientTimeTablesAckItem->MorningTurn );

	if( strcmp(morTimeLeft,"") != 0 ){
		json_object_set_string( morning_obj, NATTY_USER_PROTOCOL_STARTTIME, morTimeLeft );
	}else{
		json_object_set_string( morning_obj, NATTY_USER_PROTOCOL_STARTTIME, NULL );
	}
	if( strcmp(morTimeRight,"") != 0 ){
		json_object_set_string( morning_obj, NATTY_USER_PROTOCOL_ENDTIME, morTimeRight);
	}else{
		json_object_set_string( morning_obj, NATTY_USER_PROTOCOL_ENDTIME, NULL );
	}

	json_object_set_value( timetables_obj, NATTY_USER_PROTOCOL_AFTERNOON, json_value_init_object() );
	JSON_Object *afternoon_obj = json_object_get_object( timetables_obj, NATTY_USER_PROTOCOL_AFTERNOON );
	json_object_set_string( afternoon_obj, NATTY_USER_PROTOCOL_STATUS, pClientTimeTablesAck->objClientTimeTablesAckItem->AfternoonTurn );
	if( strcmp(AftTimeLeft,"") != 0 ){
		json_object_set_string( afternoon_obj, NATTY_USER_PROTOCOL_STARTTIME, AftTimeLeft );
	}else{
		json_object_set_string( afternoon_obj, NATTY_USER_PROTOCOL_STARTTIME, NULL );
	}
	if( strcmp(AftTimeRight,"") != 0 ){
		json_object_set_string( afternoon_obj, NATTY_USER_PROTOCOL_ENDTIME, AftTimeRight);
	}else{
		json_object_set_string( afternoon_obj, NATTY_USER_PROTOCOL_ENDTIME, NULL );
	}
	
	
	char *jsonstring =  json_serialize_to_string( schema );
	json_value_free( schema );
	return jsonstring;
}
*/


char *ntyClientLocationAckJsonCompose( ClientLocationAck *pClientLocationAck ){
	if ( pClientLocationAck == NULL ) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object( schema );
	json_object_set_value( schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object() );
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS );
	json_object_set_string( results_obj, NATTY_USER_PROTOCOL_IMEI, pClientLocationAck->IMEI );
	json_object_set_string( results_obj, NATTY_USER_PROTOCOL_CATEGORY, pClientLocationAck->Category );

	char type[32] = {0};
	if (pClientLocationAck->results->Type == 1) {
		strcat(type, "WIFI");
	} else if (pClientLocationAck->results->Type == 2) {
		strcat(type, "GPS");
	} else if (pClientLocationAck->results->Type == 3) {
		strcat(type, "LAB");
	} else {
		strcat(type, "LAB");
	}
	
	json_object_set_string( results_obj, NATTY_USER_PROTOCOL_TYPE, type );
	//json_object_set_string( results_obj, NATTY_USER_PROTOCOL_RADIUS, pClientLocationAck->results->Radius );
	json_object_set_string( results_obj, NATTY_USER_PROTOCOL_RADIUS, "550" );
	json_object_set_string( results_obj, NATTY_USER_PROTOCOL_LOCATION, pClientLocationAck->results->Location );
	
	char *jsonstring =  json_serialize_to_string( schema );
	json_value_free( schema );
	return jsonstring;
}

char *ntyClientURLAckJsonCompose( ClientURLAck *pClientURLAck ){
	if ( pClientURLAck == NULL ) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, pClientURLAck->IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_CATEGORY, pClientURLAck->Category);

	json_object_set_value(results_obj, NATTY_USER_PROTOCOL_URL, json_value_init_object());
	JSON_Object *url_obj = json_object_get_object(results_obj, NATTY_USER_PROTOCOL_URL);
	json_object_set_string(url_obj, NATTY_USER_PROTOCOL_QRCODE, pClientURLAck->objClientURLAckItem.QRCode);
	
	char *jsonstring =  json_serialize_to_string( schema );
	json_value_free( schema );
	return jsonstring;
}

char *ntyClientEfenceAckJsonCompose( ClientEfenceAck *pClientEfenceAck ){
	if (pClientEfenceAck == NULL) {
		return NULL;
	}

	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object(schema);
	json_object_set_value(schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object());
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_IMEI, pClientEfenceAck->results.IMEI);
	json_object_set_string(results_obj, NATTY_USER_PROTOCOL_CATEGORY, pClientEfenceAck->results.category);

	json_object_set_value(results_obj, NATTY_USER_PROTOCOL_EFENCELIST, json_value_init_array());
	JSON_Array *effencelist_arr = json_object_get_array(results_obj, NATTY_USER_PROTOCOL_EFENCELIST);
	
	size_t i;
	for (i = 0; i < pClientEfenceAck->results.efencelist_size; i++) {
		json_array_append_value(effencelist_arr, json_value_init_object());
		JSON_Object *effencelist_obj = json_array_get_object(effencelist_arr, i);

		json_object_set_string(effencelist_obj, NATTY_USER_PROTOCOL_INDEX, pClientEfenceAck->results.pClientEfenceListItem[i].index);
		json_object_set_value(effencelist_obj, NATTY_USER_PROTOCOL_EFENCE, json_value_init_object());
		JSON_Object *effence_obj = json_object_get_object(effencelist_obj, NATTY_USER_PROTOCOL_EFENCE);
		json_object_set_string(effence_obj, NATTY_USER_PROTOCOL_NUM, pClientEfenceAck->results.pClientEfenceListItem[i].num);

		json_object_set_value(effence_obj, NATTY_USER_PROTOCOL_POINTS, json_value_init_array());
		JSON_Array *point_arr = json_object_get_array(effence_obj, NATTY_USER_PROTOCOL_POINTS);
	
		size_t point_size = 0;
		char *points = pClientEfenceAck->results.pClientEfenceListItem[i].points;
		if (points != NULL) {
			char *ptr = NULL;
			ptr = strtok(points, ";");
			while(ptr != NULL){
				json_array_append_string(point_arr, ptr);
				ptr = strtok(NULL, ";");
	    	}
		}
	}
	
	char *jsonstring =  json_serialize_to_string(schema);
	json_value_free(schema);
	return jsonstring;
}


const U8 *u8CategoryToken[] = {
	NATTY_USER_PROTOCOL_CATEGORY_EFENCE,
	NATTY_USER_PROTOCOL_CATEGORY_RUNTIME,
	NATTY_USER_PROTOCOL_CATEGORY_TURN,
	NATTY_USER_PROTOCOL_CATEGORY_SCHEDULE,
	NATTY_USER_PROTOCOL_CATEGORY_TIMETABLES,
	NATTY_USER_PROTOCOL_CATEGORY_CONTACTS,
	NATTY_USER_PROTOCOL_SOSREPORT,
	NATTY_USER_PROTOCOL_LOCATIONREPORT,
	NATTY_USER_PROTOCOL_STEPSREPORT,
	NATTY_USER_PROTOCOL_EFENCEREPORT,
	NATTY_USER_PROTOCOL_WEARSTATUS,
}; 

int ntyCommonJsonCategory(char *json, int length) {
	int i = 0;
	U32 matches[128] = {0};

	for (i = NTY_CATEGORY_START;i < NTY_CATEGORY_END;i ++) {
		int plen = strlen(u8CategoryToken[i]);
		
		int ret = ntyKMP(json, length, u8CategoryToken[i], plen, matches);
		if (ret > 0) {
			return i;
		}
	}

	return NTY_RESULT_FAILED;
}

/*

char *ntyClientEfenceAckJsonCompose( ClientEfenceAck *pClientEfenceAck ){
	if ( pClientEfenceAck == NULL ) {
		return NULL;
	}
	/*
	char *ptrBuf[10];
	int count = 0;
	for( count=0; count<10; count++ ){
		ptrBuf[count] = (char*)malloc( sizeof(char)*100 );
	}
	char *ptrTmpBuf = NULL;
	char *ptrTmp = (char *)malloc(300);
	char *outBuf = NULL;
	int iNum = 0;
	*
	
	
	
	JSON_Value *schema = json_value_init_object();
	JSON_Object *schema_obj = json_value_get_object( schema );
	json_object_set_value( schema_obj, NATTY_USER_PROTOCOL_RESULTS, json_value_init_object() );
	JSON_Object *results_obj = json_object_get_object(schema_obj, NATTY_USER_PROTOCOL_RESULTS );
	json_object_set_string( results_obj, NATTY_USER_PROTOCOL_IMEI, pClientEfenceAck->IMEI );
	json_object_set_string( results_obj, NATTY_USER_PROTOCOL_CATEGORY, NATTY_USER_PROTOCOL_EFENCELIST );

	json_object_set_value( results_obj, NATTY_USER_PROTOCOL_EFENCELIST, json_value_init_array() );
	JSON_Array *fencelist_arr = json_object_get_array( results_obj, NATTY_USER_PROTOCOL_EFENCELIST );

	int i = 0;
	for ( i = 0; i < pClientEfenceAck->size ; i++ ) { //the list number
		json_array_append_value( fencelist_arr, json_value_init_object() );
		JSON_Object *fencelist_obj = json_array_get_object( fencelist_arr, i );
		json_object_set_string( fencelist_obj, NATTY_USER_PROTOCOL_INDEX, pClientEfenceAck->objClientEfenceAckItem[i].Index );
		
		json_object_set_value( fencelist_obj, NATTY_USER_PROTOCOL_EFENCE, json_value_init_object() );
		JSON_Object *fence_obj = json_object_get_object( fencelist_obj, NATTY_USER_PROTOCOL_EFENCE );
		json_object_set_string( fence_obj, NATTY_USER_PROTOCOL_NUM, pClientEfenceAck->objClientEfenceAckItem->Num );

		json_object_set_value( fence_obj, NATTY_USER_PROTOCOL_POINTS, json_value_init_array() );
		JSON_Array * points_arr = json_object_get_array( fence_obj, NATTY_USER_PROTOCOL_POINTS );

		ptrTmpBuf = pClientEfenceAck->objClientEfenceAckItem[i].Points; 
		ntylog( "...............ptrTmpBuf:%s\n",ptrTmpBuf );
		memset( ptrTmp,0,300 );
		snprintf( ptrTmp,300,"%s",pClientEfenceAck->objClientEfenceAckItem[i].Points );
		iNum = 0;
		memset( ptrBuf,0,300 );
		//ntylog( "...............ptrBuf:%s\n",ptrBuf );
		while( (ptrBuf[iNum]=strtok_r(ptrTmp,";",&outBuf)) != NULL ) //parse Points string,save to ptrBuf
		{
			ntylog( "...............ptrBuf[iNum]:%s\n",ptrBuf[iNum] );
			iNum++;			
			ptrTmp = NULL;			
		}

		int j = 0;
		for( j = 0; j < iNum; j++  ){ //add to object
			json_array_append_value( points_arr, json_value_init_object() );
			JSON_Object *points_obj = json_array_get_object( points_arr, j );
			ntylog( "...............ptrBuf[j]:%s\n",ptrBuf[j] );
			json_object_set_string( points_obj, NULL, ptrBuf[j] );
		}
	}	
	
	char *jsonstring =  json_serialize_to_string( schema );
	json_value_free( schema );
	return jsonstring;
}

*/


//add end
