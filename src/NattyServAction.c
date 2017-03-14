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

#include "NattyServAction.h"
#include "NattyDaveMQ.h"

#include <string.h>
#include <time.h>


void ntyJsonBroadCastRecvResult(C_DEVID selfId, C_DEVID gId, char *jsonresult) {
	if (jsonresult == NULL) {
		return;
	}
	U16 jsonlen = strlen(jsonresult);
	ntySendCommonBroadCastResult(selfId, gId, (U8*)jsonresult, jsonlen);
}

void ntyJsonResult(C_DEVID devId, const char * code) {
	CommonAck *pCommonAck = (CommonAck*)malloc(sizeof(CommonAck));
	pCommonAck->result.code = code;
	char *jsonresult = ntyJsonWriteCommon(pCommonAck);
	
	ntylog("ntyJsonSuccessResult-> %lld, %s  %d\n", devId, jsonresult, strlen(jsonresult));
	ntySendDataResult(devId, (U8*)jsonresult, strlen(jsonresult), 200);
	ntyJsonFree(jsonresult);
	free(pCommonAck);
}

/*
 * amap WIFI position
 *
 * http://apilocate.amap.com/position?accesstype=1&imei=352315052834187&smac=E0:DB:55:E4:C7:49&mmac=22:27:1d:20:08:d5,-55,CMCC-EDU&macs=22:27:1d:
 * 20:08:d5,-55,CMCC-EDU|5c:63:bf:a4:bf:56,-86,TP-LINK|d8:c7:c8:a8:1a:13,-42,TP-LINK&serverip=10.2.166.4&output=json&key=<用户 Key>
 *
 */
void ntyJsonLocationWIFIAction(C_DEVID fromId, C_DEVID toId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	WIFIReq *pWIFIReq = (WIFIReq*)malloc(sizeof(WIFIReq));
	ntyJsonWIFI(json, pWIFIReq);

	U8 macs[1024] = {0};
	size_t i;
	for (i=0; i<pWIFIReq->size; i++) {
		strcat(macs, pWIFIReq->pWIFI[i].MAC);
		strcat(macs, ",");
		strcat(macs, pWIFIReq->pWIFI[i].V);
		strcat(macs, ",");
		strcat(macs, pWIFIReq->pWIFI[i].SSID);
		strcat(macs, "|");
	}
	size_t macs_len = strlen(macs);
	if (macs_len!=0) {
		macs[macs_len-1] = '\0';
	}

	U8 wifibuf[1024] = {0};
	sprintf(wifibuf, "%s/position?accesstype=1&imei=%s&macs=%s&output=json&key=%s", 
		HTTP_GAODE_BASE_URL, pWIFIReq->IMEI, macs, HTTP_GAODE_KEY);
	ntydbg(" wifibuf --> %s\n", wifibuf);

	MessageTag *pMessageTag = malloc(sizeof(MessageTag));
	pMessageTag->fromId = fromId;
	pMessageTag->toId = toId;
	pMessageTag->Tag = wifibuf;
	pMessageTag->length = strlen(wifibuf);
	
	int ret = ntyHttpQJKLocation(pMessageTag);
	if (ret == -1) {
		ntylog(" ntyHttpQJKLocation --> Http Exception\n");
		ret = 4;
	} else if (ret > 0) {
		ntyJsonResult(fromId, NATTY_RESULT_CODE_SUCCESS);
	}
	
	ntyJsonWIFIItemRelease(pWIFIReq->pWIFI);
	free(pWIFIReq);
}


/**
 * amap Lab position
 *
 * http://apilocate.amap.com/position?accesstype=0&imei=352315052834187&cdma=0&bts=460,01,40977,2205409,-65&nearbts=460,01,
 * 40977,2205409,-65|460,01,40977,2205409,-65|460,01,40977,2205409,-65&serverip=10.2.166.4&output=json&key=<用户 Key>
 *
 */
void ntyJsonLocationLabAction(C_DEVID fromId, C_DEVID toId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	ntydbg(" ntyJsonLocationLabAction begin --> \n");
	LABReq *pLABReq = (LABReq*)malloc(sizeof(LABReq));
	ntyJsonLAB(json, pLABReq);
	
	U8 nearbts[200] = {0};
	size_t i;
	for (i=0; i<pLABReq->size; i++) {
		strcat(nearbts, pLABReq->lab.pNearbts[i].cell);
		strcat(nearbts, ",");
		strcat(nearbts, pLABReq->lab.pNearbts[i].signal);
		strcat(nearbts, "|");
	}
	size_t nearbts_len = strlen(nearbts);
	if (nearbts_len!=0) {
		nearbts[nearbts_len-1] = '\0';
	}
	
	U8 labbuf[500] = {0};
	sprintf(labbuf, "%s/position?accesstype=0&imei=%s&cdma=0&bts=%s&nearbts=%s&output=json&key=%s", 
		HTTP_GAODE_BASE_URL, pLABReq->IMEI, pLABReq->lab.bts, nearbts, HTTP_GAODE_KEY);
	ntydbg(" labbuf --> %s\n", labbuf);

	MessageTag *pMessageTag = malloc(sizeof(MessageTag));
	pMessageTag->fromId = fromId;
	pMessageTag->toId = toId;
	pMessageTag->Tag = labbuf;
	pMessageTag->length = strlen(labbuf);

	int n8LocationType = 0;
	ntyJsonSetLocationType(pLABReq->category, &n8LocationType);
	pMessageTag->u8LocationType = (U8)n8LocationType; 

	int ret = ntyHttpQJKLocation(pMessageTag);
	free(pMessageTag);
	if (ret == -1) {
		ntylog(" ntyHttpQJKLocation --> Http Exception\n");
		ret = 4;
	} else if (ret > 0) {
		ntyJsonResult(fromId, NATTY_RESULT_CODE_SUCCESS);
	}

	ntyJsonLABItemRelease(pLABReq->lab.pNearbts);
	free(pLABReq);
	ntydbg(" ntyJsonLocationLabAction end --> \n");
}

void ntyJsonWeatherAction(C_DEVID fromId, C_DEVID toId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	WeatherReq *pWeatherReq = (WeatherReq*)malloc(sizeof(WeatherReq));
	ntyJsonWeather(json, pWeatherReq);

	U8 weatherbuf[500] = {0};
	sprintf(weatherbuf, "%s/position?accesstype=0&imei=%s&cdma=0&bts=%s&output=json&key=%s", 
		HTTP_GAODE_BASE_URL, pWeatherReq->IMEI, pWeatherReq->bts, HTTP_GAODE_KEY);
	ntydbg(" weatherbuf --> %s\n", weatherbuf);

	MessageTag *pMessageTag = malloc(sizeof(MessageTag));
	pMessageTag->fromId = fromId;
	pMessageTag->toId = toId;
	pMessageTag->Tag = weatherbuf;
	pMessageTag->length = strlen(weatherbuf);
	
	int ret = ntyHttpQJKWeatherLocation(pMessageTag);
	if (ret == -1) {
		ntylog(" ntyHttpQJKWeatherLocation --> Http Exception\n");
		ret = 4;
	} else if (ret > 0) {
		ntyJsonResult(toId, NATTY_RESULT_CODE_SUCCESS);
	/*
		JSON_Value *json2 = NULL;
		AMap *pAMap = (AMap*)malloc(sizeof(AMap));
		ntyJsonAMap(json2, pAMap);
		if (strcmp(pAMap->status, "1") != 0) {
			return;	
		}
	
		char colon[2] = {0x3A, 0x00};
		char lng[50] = {0};
		char lat[50] = {0};
	    char *p2 = strchr(pAMap->result.location, ',');
		char *p1 = (char *)pAMap->result.location;
		if (p2 != NULL) {
			size_t len = p2-p1;
			memcpy(lng, p1, len);
			memcpy(lat, p1+len+1, (size_t)strlen(pAMap->result.location)-len);
		}
		char latlng[100] = {0};
		strncat(latlng, lat, (size_t)strlen(lat));
		strncat(latlng, colon, (size_t)strlen(colon));
		strncat(latlng, lng, (size_t)strlen(lng));
		
		U8 weatherbuf[500] = {0};
		sprintf(weatherbuf, "https://api.thinkpage.cn/v3/weather/daily.json?key=0pyd8z7jouficcil&location=%s&language=zh-Hans&unit=c&start=0&days=5", latlng);
		ntydbg(" weatherbuf --> %s\n", weatherbuf);
		
		ret = ntyHttpQJKWeather(weatherbuf);
		if (ret == -1) {
			ntylog(" ntyHttpQJKLocation --> Http Exception\n");
			ret = 4;
		} else if (ret > 0) {
			ntyJsonResult(toId, NATTY_RESULT_CODE_SUCCESS);
		}

		free(pAMap);
	*/
	}

	free(pWeatherReq);
}

void ntyJsonEfenceAction(C_DEVID AppId, C_DEVID toId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	EfenceReq *pEfenceReq = (EfenceReq*)malloc(sizeof(EfenceReq));
	ntyJsonEfence(json, pEfenceReq);
	C_DEVID devId = toId;

	U8 points[200] = {0};
	size_t i;
	for (i=0; i<pEfenceReq->efence.size; i++) {
		strcat(points, pEfenceReq->efence.pPoints[i].point);
		strcat(points, ";");
	}
	size_t points_len = strlen(points);
	if (points_len!=0) {
		points[points_len-1] = '\0';
	}

	time_t timep;
	struct tm *p;
	time(&timep);
	p = localtime(&timep);
	U8 runtime[50] = {0};
	sprintf(runtime, "%04d/%02d/%02d %02d:%02d:%02d", 1900+p->tm_year, 1+p->tm_mon, p->tm_hour, p->tm_hour, p->tm_min, p->tm_sec);
	
	int ret = ntyExecuteEfenceInsertHandle(AppId, devId, pEfenceReq->efence.size, points, runtime);
	if (ret == -1) {
		ntylog(" ntyJsonEfenceAction --> DB Exception\n");
		ret = 4;
	} else if (ret == 0) {
		ntyJsonResult(AppId, NATTY_RESULT_CODE_SUCCESS);

		EfenceAck *pEfenceAck = (EfenceAck*)malloc(sizeof(EfenceAck));
		pEfenceAck->result = *(EfenceResult*)pEfenceReq;
		char *jsonresult = ntyJsonWriteEfence(pEfenceAck);
		ntylog(" ntySendCommonBroadCastResult --> %lld, %lld, %s, %d\n", AppId, devId, jsonresult, strlen(jsonresult));
		ntySendCommonBroadCastResult(AppId, devId, (U8*)jsonresult, strlen(jsonresult));
		ntyJsonFree(jsonresult);
		free(pEfenceAck);
	}
	ntyJsonEfencePointRelease(pEfenceReq->efence.pPoints);
	free(pEfenceReq);
}

void ntyJsonRunTimeAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	RunTimeReq *pRunTimeReq = (RunTimeReq*)malloc(sizeof(RunTimeReq));
	ntyJsonRuntime(json, pRunTimeReq);

	int ret = 0;

	if (pRunTimeReq->runtime.auto_connection != NULL) {
		size_t len_auto_connection = strlen(pRunTimeReq->runtime.auto_connection);
		if (len_auto_connection != 0) {
			int auto_connection = atoi(pRunTimeReq->runtime.auto_connection);
			ret = ntyExecuteRuntimeAutoConnUpdateHandle(AppId, devId, auto_connection);
			if (ret == -1) {
				ntylog(" ntyJsonRunTimeAction:AutoConnection --> DB Exception\n");
				return;
			}
		}
	}

	if (pRunTimeReq->runtime.loss_report != NULL) {
		size_t len_loss_report = strlen(pRunTimeReq->runtime.loss_report);
		if (len_loss_report != 0) {
			U8 len_loss_report = atoi(pRunTimeReq->runtime.loss_report);
			ret = ntyExecuteRuntimeLossReportUpdateHandle(AppId, devId, len_loss_report);
			if (ret == -1) {
				ntylog(" ntyJsonRunTimeAction:LossReport --> DB Exception\n");
				return;
			}
		}
	}

	if (pRunTimeReq->runtime.light_panel != NULL) {
		size_t len_light_panel = strlen(pRunTimeReq->runtime.light_panel);
		if (len_light_panel != 0) {
			U8 len_light_panel = atoi(pRunTimeReq->runtime.light_panel);
			ret = ntyExecuteRuntimeLightPanelUpdateHandle(AppId, devId, len_light_panel);
			if (ret == -1) {
				ntylog(" ntyJsonRunTimeAction:LightPanel --> DB Exception\n");
				return;
			}
		}
	}

	if (pRunTimeReq->runtime.watch_bell != NULL) {
		size_t len_bell = strlen(pRunTimeReq->runtime.watch_bell);
		if (len_bell != 0) {
			ret = ntyExecuteRuntimeBellUpdateHandle(AppId, devId, pRunTimeReq->runtime.watch_bell);
			if (ret == -1) {
				ntylog(" ntyJsonRunTimeAction:Bell --> DB Exception\n");
				return;
			}
		}
	}

	if (pRunTimeReq->runtime.taget_step != NULL) {
		size_t len_taget_step = strlen(pRunTimeReq->runtime.taget_step);
		if (len_taget_step != 0) {
			int len_taget_step = atoi(pRunTimeReq->runtime.taget_step);
			ret = ntyExecuteRuntimeTargetStepUpdateHandle(AppId, devId, len_taget_step);
			if (ret == -1) {
				ntylog(" ntyJsonRunTimeAction:TargetStep --> DB Exception\n");
				return;
			}
		}
	}

	if (ret == 0) {
		ntyJsonResult(AppId, NATTY_RESULT_CODE_SUCCESS);
		RunTimeAck *pRunTimeAck = (RunTimeAck*)malloc(sizeof(RunTimeAck));
		pRunTimeAck->result = *(RunTimeResult*)pRunTimeReq;
		char *jsonresult = ntyJsonWriteRunTime(pRunTimeAck);
		ntyJsonBroadCastRecvResult(AppId, devId, jsonresult);
		ntyJsonFree(jsonresult);
		free(pRunTimeAck);
	}
	free(pRunTimeReq);
}

void ntyJsonTurnAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	ntylog(" =======================begin ntyJsonTurnAction====================================\n");
	TurnReq *pTurnReq = (TurnReq*)malloc(sizeof(TurnReq));
	ntyJsonTurn(json, pTurnReq);
	U8 status = 0;
	if (pTurnReq->turn.status != NULL) {
		status = atoi(pTurnReq->turn.status);
	}
	ntylog(" ntyJsonTurnAction --> status:%d\n", status);
	int ret = ntyExecuteTurnUpdateHandle(AppId, devId, status, pTurnReq->turn.on.time, pTurnReq->turn.off.time);
	if (ret == -1) {
		ntylog(" ntyJsonTurnAction --> DB Exception\n");
		ret = 4;
	} else if (ret == 0) {
		ntyJsonResult(AppId, NATTY_RESULT_CODE_SUCCESS);
	}
	free(pTurnReq);
	ntylog(" =======================end ntyJsonTurnAction====================================\n");
}

void ntyJsonICCIDAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	ICCIDReq *pICCIDReq = (ICCIDReq*)malloc(sizeof(ICCIDReq));
	ntyJsonICCID(json, pICCIDReq);

	int ret = ntyExecuteICCIDSelectHandle(AppId, devId);
	if (ret == -1) {
		ntylog(" ntyJsonICCIDAction --> DB Exception\n");
		ret = 4;
	} else if (ret == 0) {
		ntyJsonResult(AppId, NATTY_RESULT_CODE_SUCCESS);
	}
	free(pICCIDReq);
}

void ntyJsonAddScheduleAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	AddScheduleReq *pAddScheduleReq = (AddScheduleReq*)malloc(sizeof(AddScheduleReq));
	ntyJsonAddSchedule(json, pAddScheduleReq);
	int ret = ntyExecuteScheduleInsertHandle(AppId, devId,
		pAddScheduleReq->schedule.daily,
		pAddScheduleReq->schedule.time,
		pAddScheduleReq->schedule.details);
	if (ret == -1) {
		ntylog(" ntyJsonAddScheduleAction --> DB Exception\n");
		ret = 4;
	} else if (ret == 0) {
		ntyJsonResult(AppId, NATTY_RESULT_CODE_SUCCESS);
	}
	free(pAddScheduleReq);
}

void ntyJsonDelScheduleAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	DelScheduleReq *pDelScheduleReq = (DelScheduleReq*)malloc(sizeof(DelScheduleReq));
	ntyJsonDelSchedule(json, pDelScheduleReq);
	C_DEVID DeviceId = *(C_DEVID*)(pDelScheduleReq->IMEI);

	int scheduleId = 0;
	if (strlen(pDelScheduleReq->id) != 0) {
		scheduleId = atoi(pDelScheduleReq->id);
	}
	int ret = ntyExecuteScheduleDeleteHandle(AppId, DeviceId, scheduleId);
	if (ret == -1) {
		ntylog(" ntyJsonDelScheduleAction --> DB Exception\n");
		ret = 4;
	} else if (ret == 0) {
		ntyJsonResult(AppId, NATTY_RESULT_CODE_SUCCESS);
	}
	free(pDelScheduleReq);
}

void ntyJsonUpdateScheduleAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	UpdateScheduleReq *pUpdateScheduleReq = (UpdateScheduleReq*)malloc(sizeof(UpdateScheduleReq));
	ntyJsonUpdateSchedule(json, pUpdateScheduleReq);

	int scheduleId = 0;
	if (strlen(pUpdateScheduleReq->id) != 0) {
		scheduleId = atoi(pUpdateScheduleReq->id);
	}
	int ret = ntyExecuteScheduleUpdateHandle(AppId, devId, 
		scheduleId,
		pUpdateScheduleReq->schedule.daily,
		pUpdateScheduleReq->schedule.time,
		pUpdateScheduleReq->schedule.details);
	if (ret == -1) {
		ntylog(" ntyJsonUpdateScheduleAction --> DB Exception\n");
		ret = 4;
	} else if (ret == 0) {
		ntyJsonResult(AppId, NATTY_RESULT_CODE_SUCCESS);
	}
	free(pUpdateScheduleReq);
}


void ntyJsonSelectScheduleAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	CommonReq *pCommonReq = (CommonReq*)malloc(sizeof(CommonReq));
	ntyJsonCommon(json, pCommonReq);

	ScheduleAck *pScheduleAck = (ScheduleAck*)malloc(sizeof(ScheduleAck));
	pScheduleAck->results.IMEI = pCommonReq->IMEI;
	pScheduleAck->results.category = pCommonReq->category;

	size_t size = 50;
	ScheduleItem *pSchedule = malloc(sizeof(ScheduleItem)*size);
	pScheduleAck->results.pSchedule = pSchedule;
	
	int ret = ntyExecuteScheduleSelectHandle(AppId, devId, pScheduleAck, size);
	if (ret == -1) {
		ntylog(" ntyJsonDelScheduleAction --> DB Exception\n");
		ret = 4;
	} else if (ret == 0) {
		ntyJsonResult(AppId, NATTY_RESULT_CODE_SUCCESS);

		if (ret == 1) {
			char *jsonresult = ntyJsonWriteSchedule(pScheduleAck);
			ntySendCommonBroadCastResult(AppId, devId, (U8*)jsonresult, strlen(jsonresult));
			ntyJsonFree(jsonresult);
			free(pScheduleAck);
		}
	}
	free(pSchedule);
	free(pCommonReq);
}


void ntyJsonTimeTablesAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	TimeTablesReq *pTimeTablesReq = (TimeTablesReq*)malloc(sizeof(TimeTablesReq));
	ntyJsonTimeTables(json, pTimeTablesReq);

	ntylog(" ntyJsonTimeTablesAction 1111 --> %d \n", (int)pTimeTablesReq->size);

	int ret = 1;
	/*
	size_t i;
	for (i=0; i<pTimeTablesReq->size; i++) {
		U8 morning_status = atoi(pTimeTablesReq->pTimeTables[i].morning.status);
		U8 afternoon_status = atoi(pTimeTablesReq->pTimeTables[i].afternoon.status);

		U8 morning[200] = {0}, afternoon[200] = {0};
		sprintf(morning, "%s|%s", pTimeTablesReq->pTimeTables[i].morning.startTime, pTimeTablesReq->pTimeTables[i].morning.endTime);
		sprintf(afternoon, "%s|%s", pTimeTablesReq->pTimeTables[i].afternoon.startTime, pTimeTablesReq->pTimeTables[i].afternoon.endTime);
		
		ret = ntyExecuteTimeTablesUpdateHandle(AppId, DeviceId, 
			morning, morning_status,
			afternoon, afternoon_status,
			pTimeTablesReq->pTimeTables[i].daily);
		if (ret == -1) {
			ntylog(" ntyJsonTimeTables --> DB Exception\n");
			ret = 4;
		}
	}
	*/
	ntylog(" ntyJsonTimeTablesAction 2222 --> %d \n", (int)pTimeTablesReq->size);

	//if (ret == -1) {
	//	ntylog(" ntyJsonTimeTables --> DB Exception\n");
	//	ret = 4;
	//} else if (ret == 0) {
	if (ret > 0) {
		ntylog(" ntyJsonTimeTablesAction --> ok \n");
		ret = ntySendCommonReq(devId, jsonstring, strlen(jsonstring));
		if (ret>0) {
			ntyJsonResult(AppId, NATTY_RESULT_CODE_SUCCESS);
		}
	}
	ntyJsonTimeTablesItemRelease(pTimeTablesReq->pTimeTables);
	free(pTimeTablesReq);
}









