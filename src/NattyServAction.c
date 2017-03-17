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
#include "NattyMessage.h"
#include "NattyHttpCurl.h"


#include <string.h>
#include <time.h>


void ntyJsonBroadCastRecvResult(C_DEVID selfId, C_DEVID gId, char *jsonresult) {
	if (jsonresult == NULL) {
		return;
	}
	U16 jsonlen = strlen(jsonresult);
	ntySendCommonBroadCastResult(selfId, gId, (U8*)jsonresult, jsonlen);
}

void ntyJsonCommonResult(C_DEVID devId, const char * code) {
	CommonAck *pCommonAck = (CommonAck*)malloc(sizeof(CommonAck));
	pCommonAck->result.code = code;
	pCommonAck->result.message = ntyCommonResultMessage(pCommonAck->result.code);

	char *jsonresult = ntyJsonWriteCommon(pCommonAck);
	ntylog("ntyJsonCommonResult %s -> %lld, %s  %d\n", code, devId, jsonresult, (int)strlen(jsonresult));
	ntySendDataResult(devId, (U8*)jsonresult, strlen(jsonresult), 200);
	ntyJsonFree(jsonresult);
	free(pCommonAck);
}

void ntyJsonCommonExtendResult(C_DEVID devId, const char * code, int id) {
	char ids[20] = {0};
	CommonExtendAck *pCommonExtendAck = (CommonExtendAck*)malloc(sizeof(CommonExtendAck));
	pCommonExtendAck->id = ids;
	sprintf(ids, "%d", id);
	pCommonExtendAck->result.code = code;
	pCommonExtendAck->result.message = ntyCommonResultMessage(pCommonExtendAck->result.code);
	char *jsonresult = ntyJsonWriteCommonExtend(pCommonExtendAck);
	ntylog("ntyJsonCommonExtendResult %s -> %d %lld, %s  %d\n", code, id, devId, jsonresult, (int)strlen(jsonresult));
	ntySendDataResult(devId, (U8*)jsonresult, strlen(jsonresult), 200);
	ntyJsonFree(jsonresult);
	free(pCommonExtendAck);
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
	int length = strlen(wifibuf);

#if 1 //Push to MessageQueue
	MessageTag *pMessageTag = malloc(sizeof(MessageTag));
	pMessageTag->Type = MSG_TYPE_LOCATION_WIFI_API;
	pMessageTag->fromId = fromId;
	pMessageTag->toId = toId;
	pMessageTag->length = length;

#if ENABLE_DAVE_MSGQUEUE_MALLOC
	pMessageTag->Tag = malloc((length+1)*sizeof(U8));
	memset(pMessageTag->Tag, 0, length+1);
	memcpy(pMessageTag->Tag, wifibuf, length);
#else
	memset(pMessageTag->Tag, 0, length+1);
	memcpy(pMessageTag->Tag, wifibuf, length);
#endif
	
	pMessageTag->cb = ntyHttpQJKLocation;

	int ret = ntyDaveMqPushMessage(pMessageTag);
#endif
	//int ret = ntyHttpQJKLocation(pMessageTag);
	if (ret == -1) {
		ntylog(" ntyHttpQJKLocation --> Http Exception\n");
		ret = 4;
	} else if (ret > 0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);
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
	int length = strlen(labbuf);

	MessageTag *pMessageTag = malloc(sizeof(MessageTag));
	pMessageTag->Type = MSG_TYPE_LOCATION_LAB_API;
	pMessageTag->fromId = fromId;
	pMessageTag->toId = toId;

	pMessageTag->length = length;

#if ENABLE_DAVE_MSGQUEUE_MALLOC
	pMessageTag->Tag = malloc((length+1)*sizeof(U8));
	memset(pMessageTag->Tag, 0, length+1);
	memcpy(pMessageTag->Tag, labbuf, length);
#else
	memset(pMessageTag->Tag, 0, length+1);
	memcpy(pMessageTag->Tag, labbuf, length);
#endif
	
	pMessageTag->cb = ntyHttpQJKLocation;
	
	int n8LocationType = 0;
	ntyJsonSetLocationType(pLABReq->category, &n8LocationType);
	pMessageTag->u8LocationType = (U8)n8LocationType; 

	int ret = ntyDaveMqPushMessage(pMessageTag);
	//int ret = ntyHttpQJKLocation(pMessageTag);
	free(pMessageTag);
	if (ret == -1) {
		ntylog(" ntyHttpQJKLocation --> Http Exception\n");
		ret = 4;
	} else if (ret > 0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);
	}

	ntyJsonLABItemRelease(pLABReq->lab.pNearbts);
	free(pLABReq);
	ntydbg(" ntyJsonLocationLabAction end --> \n");
}

void ntyJsonWeatherAction(C_DEVID fromId, C_DEVID toId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	WeatherLocationReq *pWeatherLocationReq = (WeatherLocationReq*)malloc(sizeof(WeatherLocationReq));
	ntyJsonWeatherLocation(json, pWeatherLocationReq);

	U8 weatherbuf[500] = {0};
	sprintf(weatherbuf, "%s/position?accesstype=0&imei=%s&cdma=0&bts=%s&output=json&key=%s", 
		HTTP_GAODE_BASE_URL, pWeatherLocationReq->IMEI, pWeatherLocationReq->bts, HTTP_GAODE_KEY);
	ntydbg(" weatherbuf --> %s\n", weatherbuf);
	int length = strlen(weatherbuf);

	MessageTag *pMessageTag = malloc(sizeof(MessageTag));
	pMessageTag->Type = MSG_TYPE_WEATHER_API;
	pMessageTag->fromId = fromId;
	pMessageTag->toId = toId;
	pMessageTag->length = length;
	
#if ENABLE_DAVE_MSGQUEUE_MALLOC
	pMessageTag->Tag = malloc((length+1)*sizeof(U8));
	memset(pMessageTag->Tag, 0, length+1);
	memcpy(pMessageTag->Tag, weatherbuf, length);
#else
	memset(pMessageTag->Tag, 0, length+1);
	memcpy(pMessageTag->Tag, weatherbuf, length);
#endif
	
	pMessageTag->cb = ntyHttpQJKWeatherLocation;

	int ret = ntyDaveMqPushMessage(pMessageTag);
	//int ret = ntyHttpQJKWeatherLocation(pMessageTag);
	if (ret == -1) {
		ntylog(" ntyHttpQJKWeatherLocation --> Http Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ntyJsonCommonResult(toId, NATTY_RESULT_CODE_SUCCESS);
	}

	free(pWeatherLocationReq);
}

#if 1 //
int CheckNumber(char *num, int length) {
	int i = 0;
	for (i = 0;i < length;i ++) {
		if (num[i] < 0x30 || num[i] > 0x39) {
			return -1;
		}
	}
	return 0;
}
#endif

void ntyJsonAddEfenceAction(C_DEVID AppId, C_DEVID toId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	AddEfenceReq *pAddEfenceReq = (AddEfenceReq*)malloc(sizeof(AddEfenceReq));
	ntyJsonAddEfence(json, pAddEfenceReq);
	C_DEVID devId = toId;

	U8 points[200] = {0};
	size_t i;
	for (i=0; i<pAddEfenceReq->efence.size; i++) {
		strcat(points, pAddEfenceReq->efence.pPoints[i].point);
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

	int index = atoi(pAddEfenceReq->index);

	int id = 0;
	int ret = ntyExecuteEfenceInsertHandle(AppId, devId, index, pAddEfenceReq->efence.size, points, runtime, &id);
	if (ret == -1) {
		ntylog(" ntyJsonEfenceAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ret = ntySendCommonReq(devId, jsonstring, strlen(jsonstring));
		if (ret >= 0) {
			ntyJsonCommonResult(AppId, NATTY_RESULT_CODE_SUCCESS);
			
			AddEfenceAck *pAddEfenceAck = (AddEfenceAck*)malloc(sizeof(AddEfenceAck));
			char ids[20] = {0};
			sprintf(ids, "%d", id);
			pAddEfenceAck->id = ids;
			pAddEfenceAck->result = *(AddEfenceResult*)pAddEfenceReq;
			char *jsonresult = ntyJsonWriteAddEfence(pAddEfenceAck);
			ntylog(" ntySendCommonBroadCastResult --> %lld, %lld, %s, %d\n", AppId, devId, jsonresult, (int)strlen(jsonresult));
			ntySendCommonBroadCastResult(AppId, devId, (U8*)jsonresult, strlen(jsonresult));
			ntyJsonFree(jsonresult);
			free(pAddEfenceAck);
		}
	}
	ntyJsonAddEfencePointRelease(pAddEfenceReq->efence.pPoints);
	free(pAddEfenceReq);
}

void ntyJsonDelEfenceAction(C_DEVID AppId, C_DEVID toId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	DelEfenceReq *pDelEfenceReq = (DelEfenceReq*)malloc(sizeof(DelEfenceReq));
	ntyJsonDelEfence(json, pDelEfenceReq);
	C_DEVID devId = toId;
	int index = 0;
	if (pDelEfenceReq->index != NULL) {
		index = atoi(pDelEfenceReq->index);
	}
	int ret = ntyExecuteEfenceDeleteHandle(AppId, devId, index);
	if (ret == -1) {
		ntylog(" ntyJsonEfenceAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ntyJsonCommonResult(AppId, NATTY_RESULT_CODE_SUCCESS);
		DelEfenceAck *pDelEfenceAck = (DelEfenceAck*)malloc(sizeof(DelEfenceAck));
		pDelEfenceAck->result = *(DelEfenceResult*)pDelEfenceReq;
		char *jsonresult = ntyJsonWriteDelEfence(pDelEfenceAck);
		ntylog(" ntySendCommonBroadCastResult --> %lld, %lld, %s, %d\n", AppId, devId, jsonresult, (int)strlen(jsonresult));
		ntySendCommonBroadCastResult(AppId, devId, (U8*)jsonresult, strlen(jsonresult));
		ntyJsonFree(jsonresult);
		free(pDelEfenceAck);
	}
	free(pDelEfenceReq);
}

void ntyJsonRunTimeAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	RunTimeReq *pRunTimeReq = (RunTimeReq*)malloc(sizeof(RunTimeReq));
	ntyJsonRuntime(json, pRunTimeReq);

	int ret = 0;

	ntydbg("---------------------1----------------------\n");
	
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
	ntydbg("---------------------2----------------------\n");

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
	ntydbg("---------------------3----------------------\n");
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
	ntydbg("---------------------4----------------------\n");
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
	ntydbg("---------------------5----------------------\n");

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
	ntydbg("---------------------6----------------------\n");
	if (ret == 0) {
		ret = ntySendCommonReq(devId, jsonstring, strlen(jsonstring));
		if (ret >= 0) {
			ntyJsonCommonResult(AppId, NATTY_RESULT_CODE_SUCCESS);
			RunTimeAck *pRunTimeAck = (RunTimeAck*)malloc(sizeof(RunTimeAck));
			pRunTimeAck->result = *(RunTimeResult*)pRunTimeReq;
			char *jsonresult = ntyJsonWriteRunTime(pRunTimeAck);
			ntyJsonBroadCastRecvResult(AppId, devId, jsonresult);
			ntyJsonFree(jsonresult);
			free(pRunTimeAck);
		}
	}
	ntydbg("---------------------7----------------------\n");
	free(pRunTimeReq);
}

void ntyJsonTurnAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	TurnReq *pTurnReq = (TurnReq*)malloc(sizeof(TurnReq));
	ntyJsonTurn(json, pTurnReq);
	U8 status = 0;
	if (pTurnReq->turn.status != NULL) {
		status = atoi(pTurnReq->turn.status);
	}
	int ret = ntyExecuteTurnUpdateHandle(AppId, devId, status, pTurnReq->turn.on.time, pTurnReq->turn.off.time);
	if (ret == -1) {
		ntylog(" ntyJsonTurnAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ret = ntySendCommonReq(devId, jsonstring, strlen(jsonstring));
		if (ret >= 0) {
			ntyJsonCommonResult(AppId, NATTY_RESULT_CODE_SUCCESS);
		}
	}
	free(pTurnReq);
}

void ntyJsonICCIDAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	ICCIDReq *pICCIDReq = (ICCIDReq*)malloc(sizeof(ICCIDReq));
	ntyJsonICCID(json, pICCIDReq);

	int ret = ntyExecuteICCIDSelectHandle(AppId, devId);
	if (ret == -1) {
		ntylog(" ntyJsonICCIDAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ret = ntySendCommonReq(devId, jsonstring, strlen(jsonstring));
		if (ret >= 0) {
			ntyJsonCommonResult(devId, NATTY_RESULT_CODE_SUCCESS);
		}
	}
	free(pICCIDReq);
}

void ntyJsonAddScheduleAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	AddScheduleReq *pAddScheduleReq = (AddScheduleReq*)malloc(sizeof(AddScheduleReq));
	ntyJsonAddSchedule(json, pAddScheduleReq);
	int id = 0;
	int ret = ntyExecuteScheduleInsertHandle(AppId, devId,
		pAddScheduleReq->schedule.daily,
		pAddScheduleReq->schedule.time,
		pAddScheduleReq->schedule.details,
		id);
	if (ret == -1) {
		ntylog(" ntyJsonAddScheduleAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ret = ntySendCommonReq(devId, jsonstring, strlen(jsonstring));
		if (ret >= 0) {
			ntyJsonCommonExtendResult(AppId, NATTY_RESULT_CODE_SUCCESS, id);
		}
	}
	free(pAddScheduleReq);
}

void ntyJsonDelScheduleAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	DelScheduleReq *pDelScheduleReq = (DelScheduleReq*)malloc(sizeof(DelScheduleReq));
	ntyJsonDelSchedule(json, pDelScheduleReq);

	int scheduleId = 0;
	if (pDelScheduleReq->id != NULL) {
		if (strlen(pDelScheduleReq->id) != 0) {
			scheduleId = atoi(pDelScheduleReq->id);
		}
	}
	int ret = ntyExecuteScheduleDeleteHandle(AppId, devId, scheduleId);
	if (ret == -1) {
		ntylog(" ntyJsonDelScheduleAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ret = ntySendCommonReq(devId, jsonstring, strlen(jsonstring));
		if (ret >= 0) {
			ntyJsonCommonResult(AppId, NATTY_RESULT_CODE_SUCCESS);
		}
	}
	free(pDelScheduleReq);
}

void ntyJsonUpdateScheduleAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	UpdateScheduleReq *pUpdateScheduleReq = (UpdateScheduleReq*)malloc(sizeof(UpdateScheduleReq));
	ntyJsonUpdateSchedule(json, pUpdateScheduleReq);

	int scheduleId = 0;
	if (pUpdateScheduleReq->id != NULL) {
		if (strlen(pUpdateScheduleReq->id) != 0) {
			scheduleId = atoi(pUpdateScheduleReq->id);
		}
	}
	int ret = ntyExecuteScheduleUpdateHandle(AppId, devId, 
		scheduleId,
		pUpdateScheduleReq->schedule.daily,
		pUpdateScheduleReq->schedule.time,
		pUpdateScheduleReq->schedule.details);
	if (ret == -1) {
		ntylog(" ntyJsonUpdateScheduleAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ret = ntySendCommonReq(devId, jsonstring, strlen(jsonstring));
		if (ret >= 0) {
			ntyJsonCommonResult(AppId, NATTY_RESULT_CODE_SUCCESS);
		}
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
		ntylog(" ntyJsonSelectScheduleAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ret = ntySendCommonReq(devId, jsonstring, strlen(jsonstring));
		if (ret >= 0) {
			ntyJsonCommonResult(AppId, NATTY_RESULT_CODE_SUCCESS);

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
	ntylog(" ntyJsonTimeTablesAction --> %s  \n", jsonstring);

	char ids[20] = {0};

	int id = 0;
	int ret = 0;
	size_t i;
	for (i=0; i<pTimeTablesReq->size; i++) {	
		U8 morning_status = atoi(pTimeTablesReq->pTimeTables[i].morning.status);
		U8 afternoon_status = atoi(pTimeTablesReq->pTimeTables[i].afternoon.status);
		U8 morning[200] = {0}, afternoon[200] = {0};
		sprintf(morning, "%s|%s", pTimeTablesReq->pTimeTables[i].morning.startTime, pTimeTablesReq->pTimeTables[i].morning.endTime);
		sprintf(afternoon, "%s|%s", pTimeTablesReq->pTimeTables[i].afternoon.startTime, pTimeTablesReq->pTimeTables[i].afternoon.endTime);
		ret = ntyExecuteTimeTablesUpdateHandle(AppId, devId, 
			morning, morning_status,
			afternoon, afternoon_status,
			pTimeTablesReq->pTimeTables[i].daily,
			&id);

		sprintf(ids, "%d", id);
		pTimeTablesReq->pTimeTables[i].id = ids;
		
		if (ret == -1) {
			ntylog(" ntyJsonTimeTables --> DB Exception\n");
			ret = 4;
		}
	}
	
	if (ret == -1) {
		ntylog(" ntyJsonTimeTables --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ntylog(" ntyJsonTimeTablesAction --> ok \n");
		ret = ntySendCommonReq(devId, jsonstring, strlen(jsonstring));
		if (ret >= 0) {
			ntyJsonCommonResult(AppId, NATTY_RESULT_CODE_SUCCESS);
			
			TimeTablesAck *pTimeTablesAck = malloc(sizeof(TimeTablesAck));
			pTimeTablesAck->results = *(TimeTablesResults*)pTimeTablesReq;
			char *jsonresult = ntyJsonWriteTimeTables(pTimeTablesAck);
			ntySendCommonBroadCastResult(AppId, devId, (U8*)jsonresult, strlen(jsonresult));
			ntyJsonFree(jsonresult);
			free(pTimeTablesAck);
		}
	}
	ntyJsonTimeTablesItemRelease(pTimeTablesReq->pTimeTables);
	free(pTimeTablesReq);
}



void ntyJsonAddContactsAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	AddContactsReq *pAddContactsReq = (AddContactsReq*)malloc(sizeof(AddContactsReq));
	ntyJsonAddContacts(json, pAddContactsReq);
	int contactsId = 0;
	int ret = ntyExecuteContactsInsertHandle(AppId, devId, &pAddContactsReq->contacts, contactsId);
	if (ret == -1) {
		ntylog(" ntyJsonAddContactsAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ret = ntySendCommonReq(devId, jsonstring, strlen(jsonstring));
		if (ret >= 0) {
			ntyJsonCommonExtendResult(AppId, NATTY_RESULT_CODE_SUCCESS, contactsId);
		}
	}
	free(pAddContactsReq);
}

void ntyJsonUpdateContactsAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	UpdateContactsReq *pUpdateContactsReq = (UpdateContactsReq*)malloc(sizeof(UpdateContactsReq));

	int contactsId = 0;
	if (pUpdateContactsReq->id != NULL) {
		if (strlen(pUpdateContactsReq->id) != 0) {
			contactsId = atoi(pUpdateContactsReq->id);
		}
	}
	ntyJsonUpdateContacts(json, pUpdateContactsReq);
	int ret = ntyExecuteContactsUpdateHandle(AppId, devId, &pUpdateContactsReq->contacts, contactsId);
	if (ret == -1) {
		ntylog(" ntyJsonUpdateContactsAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ret = ntySendCommonReq(devId, jsonstring, strlen(jsonstring));
		if (ret >= 0) {
			ntyJsonCommonExtendResult(AppId, NATTY_RESULT_CODE_SUCCESS, contactsId);
		}
	}
	free(pUpdateContactsReq);
}

void ntyJsonDelContactsAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen) {
	DelContactsReq *pDelContactsReq = (DelContactsReq*)malloc(sizeof(DelContactsReq));
	ntyJsonDelContacts(json, pDelContactsReq);

	int contactsId = 0;
	if (pDelContactsReq->id != NULL) {
		if (strlen(pDelContactsReq->id) != 0) {
			contactsId = atoi(pDelContactsReq->id);
		}
	}
	int ret = ntyExecuteContactsDeleteHandle(AppId, devId, contactsId);
	if (ret == -1) {
		ntylog(" ntyJsonDelContactsAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ret = ntySendCommonReq(devId, jsonstring, strlen(jsonstring));
		if (ret >= 0) {
			ntyJsonCommonResult(AppId, NATTY_RESULT_CODE_SUCCESS);
		}
	}
	free(pDelContactsReq);
}





