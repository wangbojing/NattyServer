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

#include "NattyResult.h"
#include "NattyFilter.h"

#include <string.h>
#include <time.h>


void ntyJsonBroadCastRecvResult(C_DEVID fromId, C_DEVID toId, char *jsonresult, U32 index) {
	if (jsonresult == NULL) {
		return;
	}

	ntylog("ntyJsonBroadCastRecvResult --> %lld %lld  %s %d %d\n", fromId, toId, (U8*)jsonresult, (int)strlen(jsonresult), index);
	ntySendCommonBroadCastResult(fromId, toId, (U8*)jsonresult, (int)strlen(jsonresult), index);
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
void ntyJsonLocationWIFIAction(ActionParam *pActionParam) {
	WIFIReq *pWIFIReq = (WIFIReq*)malloc(sizeof(WIFIReq));
	ntyJsonWIFI(pActionParam->json, pWIFIReq);
	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;
	
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
void ntyJsonLocationLabAction(ActionParam *pActionParam) {
	ntydbg(" ntyJsonLocationLabAction begin --> \n");
	LABReq *pLABReq = (LABReq*)malloc(sizeof(LABReq));
	ntyJsonLAB(pActionParam->json, pLABReq);
	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;
	
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

void ntyJsonWeatherAction(ActionParam *pActionParam) {
	WeatherLocationReq *pWeatherLocationReq = (WeatherLocationReq*)malloc(sizeof(WeatherLocationReq));
	ntyJsonWeatherLocation(pActionParam->json, pWeatherLocationReq);
	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;
	
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

//将离线数据保存到数据库
int ntySaveCommonMsgData(ActionParam *pActionParam) {
	C_DEVID sid = pActionParam->fromId;
	C_DEVID gid = pActionParam->toId;
	const char *jsonstring = pActionParam->jsonstring;

	int msg = 0;
	int retmsg = ntyExecuteCommonMsgInsertHandle(sid, gid, jsonstring, &msg);
	if (retmsg<0) {
		ntyJsonCommonResult(sid, NATTY_RESULT_CODE_ERR_DB_SAVE_OFFLINE);
		return -2;
	}

	pActionParam->index = msg;
	return msg;
}

void ntySendRecodeJsonAndSaveCommonMsgData(ActionParam *pActionParam) {
	C_DEVID sid = pActionParam->fromId;
	C_DEVID gid = pActionParam->toId;
	const char *jsonstring = pActionParam->jsonstring;

	//int ret = ntySendRecodeJsonPacket(gid, jsonstring, strlen(jsonstring));
	//if (ret < 0) {
	//	ntyJsonCommonResult(sid, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
	//	return;
	//}

	int msg = 0;
	int retmsg = ntyExecuteCommonMsgInsertHandle(sid, gid, jsonstring, &msg);
	if (retmsg<0) {
		ntyJsonCommonResult(sid, NATTY_RESULT_CODE_ERR_DB_SAVE_OFFLINE);
		return;
	}

	pActionParam->index = msg;
}

void ntyCommonReqAction(ActionParam *pActionParam) {
	ntydbg("ntyCommonReqPacketHandleRequest --> fromId:%lld    toId:%lld\n", pActionParam->fromId, pActionParam->toId);
	ntydbg("ntyCommonReqPacketHandleRequest --> json : %s\n", pActionParam->jsonstring);

	const char *category = ntyJsonAppCategory(pActionParam->json);
	if (strcmp(category, NATTY_USER_PROTOCOL_CATEGORY_EFENCE) == 0) {
		const char *action = ntyJsonAction(pActionParam->json);
		if (strcmp(action, NATTY_USER_PROTOCOL_CATEGORY_ADD) == 0) {
			ntySendRecodeJsonAndSaveCommonMsgData(pActionParam);
			if (pActionParam->index > 0) {
				ntyJsonAddEfenceAction(pActionParam);
			}
		} else if (strcmp(action, NATTY_USER_PROTOCOL_CATEGORY_DELETE) == 0) {
			ntySendRecodeJsonAndSaveCommonMsgData(pActionParam);
			if (pActionParam->index > 0) {
				ntyJsonDelEfenceAction(pActionParam);
			}
		} else {
			ntylog("Can't find action with: %s\n", action);
		}
	} else if (strcmp(category, NATTY_USER_PROTOCOL_CATEGORY_RUNTIME) == 0) {
		ntySendRecodeJsonAndSaveCommonMsgData(pActionParam);
		if (pActionParam->index > 0) {
			ntyJsonRunTimeAction(pActionParam);
		}
	} else if (strcmp(category, NATTY_USER_PROTOCOL_CATEGORY_TURN) == 0) {
		ntySendRecodeJsonAndSaveCommonMsgData(pActionParam);
		if (pActionParam->index > 0) {
			ntyJsonTurnAction(pActionParam);
		}
	} else if (strcmp(category, NATTY_USER_PROTOCOL_CATEGORY_SCHEDULE) == 0) {
		const char *action = ntyJsonAction(pActionParam->json);
		if (strcmp(action, NATTY_USER_PROTOCOL_CATEGORY_ADD) == 0) {
			ntySendRecodeJsonAndSaveCommonMsgData(pActionParam);
			if (pActionParam->index > 0) {
				ntyJsonAddScheduleAction(pActionParam);
			}
		} else if (strcmp(action, NATTY_USER_PROTOCOL_CATEGORY_DELETE) == 0) {
			ntySendRecodeJsonAndSaveCommonMsgData(pActionParam);
			if (pActionParam->index > 0) {
				ntyJsonDelScheduleAction(pActionParam);
			}
		} else if (strcmp(action, NATTY_USER_PROTOCOL_CATEGORY_UPDATE) == 0) {
			ntySendRecodeJsonAndSaveCommonMsgData(pActionParam);
			if (pActionParam->index > 0) {
				ntyJsonUpdateScheduleAction(pActionParam);
			}
		} else if (strcmp(action, NATTY_USER_PROTOCOL_CATEGORY_SCHEDULE) == 0) {
			ntySendRecodeJsonAndSaveCommonMsgData(pActionParam);
			if (pActionParam->index > 0) {
				ntyJsonSelectScheduleAction(pActionParam);
			}
		} else {
			ntylog("Can't find action with: %s\n", action);
		}
	} else if (strcmp(category, NATTY_USER_PROTOCOL_CATEGORY_TIMETABLES) == 0) {
		ntySendRecodeJsonAndSaveCommonMsgData(pActionParam);
		if (pActionParam->index > 0) {
			ntyJsonTimeTablesAction(pActionParam);
		}
	} else if (strcmp(category, NATTY_USER_PROTOCOL_CATEGORY_CONTACTS) == 0) {
		const char *action = ntyJsonAction(pActionParam->json);
		if (strcmp(action, NATTY_USER_PROTOCOL_CATEGORY_ADD) == 0) {
			ntySendRecodeJsonAndSaveCommonMsgData(pActionParam);
			if (pActionParam->index > 0) {
				ntyJsonAddContactsAction(pActionParam);
			}
		} else if (strcmp(action, NATTY_USER_PROTOCOL_CATEGORY_DELETE) == 0) {
			ntySendRecodeJsonAndSaveCommonMsgData(pActionParam);
			if (pActionParam->index > 0) {
				ntyJsonDelContactsAction(pActionParam);
			}
		} else if (strcmp(action, NATTY_USER_PROTOCOL_CATEGORY_UPDATE) == 0) {
			ntySendRecodeJsonAndSaveCommonMsgData(pActionParam);
			if (pActionParam->index > 0) {
				ntyJsonUpdateContactsAction(pActionParam);
			}
		} else {
			ntylog("Can't find action with: %s\n", action);
		}
	} else {
		ntylog("Can't find category with: %s\n", category);
	}
}

void ntyJsonAddEfenceAction(ActionParam *pActionParam) {
	/*
	AddEfenceReq *pAddEfenceReq = (AddEfenceReq*)malloc(sizeof(AddEfenceReq));
	ntyJsonAddEfence(pActionParam->json, pAddEfenceReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;

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

	
	int ret = ntySendRecodeJsonPacket(fromId, toId, pActionParam->jsonstring, pActionParam->jsonlen);
	if (ret >= 0) {
		int id = 0;
		ret = ntyExecuteEfenceInsertHandle(fromId, toId, index, pAddEfenceReq->efence.size, points, runtime, &id);
		if (ret == -1) {
			ntylog(" ntyJsonAddEfenceAction --> DB Exception\n");
			ret = 4;
		} else if (ret >= 0) {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);
			
			AddEfenceAck *pAddEfenceAck = (AddEfenceAck*)malloc(sizeof(AddEfenceAck));
			char ids[20] = {0};
			sprintf(ids, "%d", id);
			pAddEfenceAck->id = ids;
			pAddEfenceAck->result = *(AddEfenceResult*)pAddEfenceReq;
			char *jsonresult = ntyJsonWriteAddEfence(pAddEfenceAck);
			ntyJsonBroadCastRecvResult(fromId, toId, (U8*)jsonresult, pActionParam->index);
			ntyJsonFree(jsonresult);
			free(pAddEfenceAck);
		}
	} else {
		ntySaveCommonMsgData(pActionParam);
	}
	ntyJsonAddEfencePointRelease(pAddEfenceReq->efence.pPoints);
	free(pAddEfenceReq);
	*/


	AddEfenceReq *pAddEfenceReq = (AddEfenceReq*)malloc(sizeof(AddEfenceReq));
	ntyJsonAddEfence(pActionParam->json, pAddEfenceReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;

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
	int ret = ntyExecuteEfenceInsertHandle(fromId, toId, index, pAddEfenceReq->efence.size, points, runtime, &id);
	if (ret == -1) {
		ntylog(" ntyJsonEfenceAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ret = ntySendRecodeJsonPacket(fromId, toId, pActionParam->jsonstring, pActionParam->jsonlen);
		if (ret >= 0) {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);
			AddEfenceAck *pAddEfenceAck = (AddEfenceAck*)malloc(sizeof(AddEfenceAck));
			char ids[20] = {0};
			sprintf(ids, "%d", id);
			pAddEfenceAck->id = ids;
			pAddEfenceAck->result = *(AddEfenceResult*)pAddEfenceReq;
			char *jsonresult = ntyJsonWriteAddEfence(pAddEfenceAck);
			ntylog(" ntySendCommonBroadCastResult --> %lld, %lld, %s, %d\n", fromId, toId, jsonresult, (int)strlen(jsonresult));
			ntySendCommonBroadCastResult(fromId, toId, (U8*)jsonresult, strlen(jsonresult));
			ntyJsonFree(jsonresult);
			free(pAddEfenceAck);
		}
	}
	ntyJsonAddEfencePointRelease(pAddEfenceReq->efence.pPoints);
	free(pAddEfenceReq);
}

void ntyJsonDelEfenceAction(ActionParam *pActionParam) {
	DelEfenceReq *pDelEfenceReq = (DelEfenceReq*)malloc(sizeof(DelEfenceReq));
	ntyJsonDelEfence(pActionParam->json, pDelEfenceReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;
	
	C_DEVID devId = toId;
	int index = 0;
	if (pDelEfenceReq->index != NULL) {
		index = atoi(pDelEfenceReq->index);
	}
	int ret = ntyExecuteEfenceDeleteHandle(fromId, toId, index);
	if (ret == -1) {
		ntylog(" ntyJsonDelEfenceAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ret = ntySendRecodeJsonPacket(fromId, devId, pActionParam->jsonstring, pActionParam->jsonlen);
		if (ret >= 0) {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);
			DelEfenceAck *pDelEfenceAck = (DelEfenceAck*)malloc(sizeof(DelEfenceAck));
			pDelEfenceAck->result = *(DelEfenceResult*)pDelEfenceReq;
			char *jsonresult = ntyJsonWriteDelEfence(pDelEfenceAck);
			ntyJsonBroadCastRecvResult(fromId, toId, (U8*)jsonresult, pActionParam->index);
			ntyJsonFree(jsonresult);
			free(pDelEfenceAck);
		}
	}
	free(pDelEfenceReq);
}

void ntyJsonICCIDAction(ActionParam *pActionParam) {
	ICCIDReq *pICCIDReq = (ICCIDReq*)malloc(sizeof(ICCIDReq));
	ntyJsonICCID(pActionParam->json, pICCIDReq);
	C_DEVID devId = pActionParam->toId;
	
	if (pICCIDReq==NULL) {
		ntyJsonCommonResult(devId, NATTY_RESULT_CODE_ERR_JSON_CONVERT);
		return;
	}
	if (pICCIDReq->ICCID==NULL) {
		ntyJsonCommonResult(devId, NATTY_RESULT_CODE_ERR_JSON_CONVERT);
		return;
	}
	int len = strlen(pICCIDReq->ICCID);
	if (len!=20) {
		ntyJsonCommonResult(devId, NATTY_RESULT_CODE_ERR_JSON_CONVERT);
		return;
	}
	char iccid[14] = {0};
	memset(iccid, 0, 14);
	memcpy(iccid, pICCIDReq->ICCID+6, 13);
	char phonenum[20] = {0};
	memset(phonenum, 0, 20);
	int ret = ntyExecuteICCIDSelectHandle(devId, iccid, phonenum);
	ntydbg(" phonenum:%s\n", phonenum);
	if (ret == -1) {
		ntylog(" ntyJsonICCIDAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		if (strlen(phonenum) == 0) {
			ntyJsonCommonResult(devId, NATTY_RESULT_CODE_ERR_ICCID_NOTPHONENUM);
			return;
		}
		
		ICCIDAck *pICCIDAck = (ICCIDAck*)malloc(sizeof(ICCIDAck));
		pICCIDAck->IMEI = pICCIDReq->IMEI;
		pICCIDAck->phone_num = phonenum;
		char *jsonstringICCID = ntyJsonWriteICCID(pICCIDAck);
		ntySendRecodeJsonPacket(devId, jsonstringICCID, strlen(jsonstringICCID));
		free(pICCIDAck);
	}
	free(pICCIDReq);
}

void ntyJsonRunTimeAction(ActionParam *pActionParam) {
	RunTimeReq *pRunTimeReq = (RunTimeReq*)malloc(sizeof(RunTimeReq));
	ntyJsonRuntime(pActionParam->json, pRunTimeReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;
	
	int ret = 0;

	ntydbg("---------------------1----------------------\n");
	
	if (pRunTimeReq->runtime.auto_connection != NULL) {
		size_t len_auto_connection = strlen(pRunTimeReq->runtime.auto_connection);
		if (len_auto_connection != 0) {
			int auto_connection = atoi(pRunTimeReq->runtime.auto_connection);
			ret = ntyExecuteRuntimeAutoConnUpdateHandle(fromId, toId, auto_connection);
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
			ret = ntyExecuteRuntimeLossReportUpdateHandle(fromId, toId, len_loss_report);
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
			ret = ntyExecuteRuntimeLightPanelUpdateHandle(fromId, toId, len_light_panel);
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
			ret = ntyExecuteRuntimeBellUpdateHandle(fromId, toId, pRunTimeReq->runtime.watch_bell);
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
			ret = ntyExecuteRuntimeTargetStepUpdateHandle(fromId, toId, len_taget_step);
			if (ret == -1) {
				ntylog(" ntyJsonRunTimeAction:TargetStep --> DB Exception\n");
				return;
			}
		}
	}
	ntydbg("---------------------6----------------------\n");
	if (ret == -1) {
		ntylog(" ntyJsonRunTimeAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ret = ntySendRecodeJsonPacket(fromId, toId, pActionParam->jsonstring, pActionParam->jsonlen);
		if (ret >= 0) {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);
			RunTimeAck *pRunTimeAck = (RunTimeAck*)malloc(sizeof(RunTimeAck));
			pRunTimeAck->result = *(RunTimeResult*)pRunTimeReq;
			char *jsonresult = ntyJsonWriteRunTime(pRunTimeAck);
			ntyJsonBroadCastRecvResult(fromId, toId, jsonresult, pActionParam->index);
			ntyJsonFree(jsonresult);
			free(pRunTimeAck);
		}
	}
	ntydbg("---------------------7----------------------\n");
	free(pRunTimeReq);
}

void ntyJsonTurnAction(ActionParam *pActionParam) {
	TurnReq *pTurnReq = (TurnReq*)malloc(sizeof(TurnReq));
	ntyJsonTurn(pActionParam->json, pTurnReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;
	
	U8 status = 0;
	if (pTurnReq->turn.status != NULL) {
		status = atoi(pTurnReq->turn.status);
	}
	int ret = ntyExecuteTurnUpdateHandle(fromId, toId, status, pTurnReq->turn.on.time, pTurnReq->turn.off.time);
	if (ret == -1) {
		ntylog(" ntyJsonTurnAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ret = ntySendRecodeJsonPacket(fromId, toId, pActionParam->jsonstring, pActionParam->jsonlen);
		if (ret >= 0) {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);
			TurnAck *pTurnAck = (TurnAck*)malloc(sizeof(TurnAck));
			pTurnAck->result = *(TurnResult*)pTurnReq;
			char *jsonresult = ntyJsonWriteTurn(pTurnAck);
			ntyJsonBroadCastRecvResult(fromId, toId, jsonresult, pActionParam->index);
			ntyJsonFree(jsonresult);
			free(pTurnAck);
		}
	}
	free(pTurnReq);
}

void ntyJsonAddScheduleAction(ActionParam *pActionParam) {
	AddScheduleReq *pAddScheduleReq = (AddScheduleReq*)malloc(sizeof(AddScheduleReq));
	ntyJsonAddSchedule(pActionParam->json, pAddScheduleReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;
	
	int scheduleId = 0;
	int ret = ntyExecuteScheduleInsertHandle(fromId, toId,
		pAddScheduleReq->schedule.daily,
		pAddScheduleReq->schedule.time,
		pAddScheduleReq->schedule.details,
		&scheduleId);
	if (ret == -1) {
		ntylog(" ntyJsonAddScheduleAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		char ids[20] = {0};
		sprintf(ids, "%d", scheduleId);
		pAddScheduleReq->id = ids;

		DeviceAddScheduleAck *pDeviceAddScheduleAck = (DeviceAddScheduleAck*)malloc(sizeof(DeviceAddScheduleAck));
		pDeviceAddScheduleAck = (DeviceAddScheduleAck*)pAddScheduleReq;
		char *jsondeviceresult = ntyJsonWriteDeviceAddSchedule(pDeviceAddScheduleAck);
		ntyJsonFree(jsondeviceresult);
		free(pDeviceAddScheduleAck);

		ret = ntySendCommonReq(toId, jsondeviceresult, strlen(jsondeviceresult));
		if (ret >= 0) {
			AddScheduleAck *pAddScheduleAck = (AddScheduleAck*)malloc(sizeof(AddScheduleAck));
			pAddScheduleAck->result = *(AddScheduleResult*)pAddScheduleReq;
			pAddScheduleAck->result.id = ids;
			char *jsonresult = ntyJsonWriteAddSchedule(pAddScheduleAck);
			ntyJsonCommonExtendResult(fromId, NATTY_RESULT_CODE_SUCCESS, scheduleId);
			ntyJsonBroadCastRecvResult(fromId, toId, jsonresult, pActionParam->index);
			ntyJsonFree(jsonresult);
			free(pAddScheduleAck);
		}
	}
	free(pAddScheduleReq);
	
	/*
	AddScheduleReq *pAddScheduleReq = (AddScheduleReq*)malloc(sizeof(AddScheduleReq));
	ntyJsonAddSchedule(pActionParam->json, pAddScheduleReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;
	
	int scheduleId = 0;
	int ret = ntyExecuteScheduleInsertHandle(fromId, toId,
		pAddScheduleReq->schedule.daily,
		pAddScheduleReq->schedule.time,
		pAddScheduleReq->schedule.details,
		&scheduleId);
	if (ret == -1) {
		ntylog(" ntyJsonAddScheduleAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		JSON_Value *json = ntyMallocJsonValue(pActionParam->jsonstring);
		if (json == NULL) {
			ntyJsonCommonResult(toId, NATTY_RESULT_CODE_ERR_JSON_FORMAT);
			return;
		}

		char ids[20] = {0};
		memset(ids, 0, 20);
		sprintf(ids, "%d", scheduleId);
		pAddScheduleReq->id = ids;
		DeviceAddScheduleAck *pDeviceAddScheduleAck = (DeviceAddScheduleAck*)malloc(sizeof(DeviceAddScheduleAck));
		pDeviceAddScheduleAck = (DeviceAddScheduleAck*)pAddScheduleReq;

		//Encode Json and Add ScheduleId 
		char *jsondeviceresult = ntyJsonWriteDeviceAddSchedule(pDeviceAddScheduleAck);
		ret = ntySendRecodeJsonPacket(fromId, toId, jsondeviceresult, strlen(jsondeviceresult));
		ntylog(" ntySendCommonReq ret : %d\n", ret);
		ntyJsonFree(jsondeviceresult);
		free(pDeviceAddScheduleAck);
		
		if (ret >= 0) { //broadcast to All App
			AddScheduleAck *pAddScheduleAck = (AddScheduleAck*)malloc(sizeof(AddScheduleAck));
			pAddScheduleAck->result = *(AddScheduleResult*)pAddScheduleReq;
			char *jsonresult = ntyJsonWriteAddSchedule(pAddScheduleAck);
			ntyJsonCommonExtendResult(fromId, NATTY_RESULT_CODE_SUCCESS, scheduleId);
			ntyJsonBroadCastRecvResult(fromId, toId, jsonresult, pActionParam->index);
			ntyJsonFree(jsonresult);
			free(pAddScheduleAck);
		} else {
			ntySaveCommonMsgData(pActionParam);
		}
	}
	free(pAddScheduleReq);
	*/
	
	/*
	AddScheduleReq *pAddScheduleReq = (AddScheduleReq*)malloc(sizeof(AddScheduleReq));
	ntyJsonAddSchedule(pActionParam->json, pAddScheduleReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;

	DeviceAddScheduleAck *pDeviceAddScheduleAck = (DeviceAddScheduleAck*)malloc(sizeof(DeviceAddScheduleAck));
	pDeviceAddScheduleAck = (DeviceAddScheduleAck*)pAddScheduleReq;
	char *jsondeviceresult = ntyJsonWriteDeviceAddSchedule(pDeviceAddScheduleAck);
	int ret = ntySendRecodeJsonPacket(fromId, toId, jsondeviceresult, strlen(jsondeviceresult));
	ntylog(" ntySendCommonReq ret : %d\n", ret);
	ntyJsonFree(jsondeviceresult);
	free(pDeviceAddScheduleAck);
	
	if (ret < 0) {
		ntySaveCommonMsgData(pActionParam);
		ntylog(" ntyJsonAddScheduleAction --> DB Exception\n");
		ret = 4;
	} else {
		int scheduleId = 0;
		ret = ntyExecuteScheduleInsertHandle(fromId, toId,
		pAddScheduleReq->schedule.daily,
		pAddScheduleReq->schedule.time,
		pAddScheduleReq->schedule.details,
		&scheduleId);
		if (ret >= 0) { //broadcast to All App
			char ids[20] = {0};
			memset(ids, 0, 20);
			sprintf(ids, "%d", scheduleId);
			pAddScheduleReq->id = ids;
		
			AddScheduleAck *pAddScheduleAck = (AddScheduleAck*)malloc(sizeof(AddScheduleAck));
			pAddScheduleAck->result = *(AddScheduleResult*)pAddScheduleReq;
			char *jsonresult = ntyJsonWriteAddSchedule(pAddScheduleAck);
			ntyJsonCommonExtendResult(fromId, NATTY_RESULT_CODE_SUCCESS, scheduleId);
			ntyJsonBroadCastRecvResult(fromId, toId, jsonresult, pActionParam->index);
			ntyJsonFree(jsonresult);
			free(pAddScheduleAck);
		}
	}

	free(pAddScheduleReq);
	*/
}

void ntyJsonDelScheduleAction(ActionParam *pActionParam) {
	DelScheduleReq *pDelScheduleReq = (DelScheduleReq*)malloc(sizeof(DelScheduleReq));
	ntyJsonDelSchedule(pActionParam->json, pDelScheduleReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;
	
	int scheduleId = 0;
	if (pDelScheduleReq->id != NULL) {
		if (strlen(pDelScheduleReq->id) != 0) {
			scheduleId = atoi(pDelScheduleReq->id);
		}
	}
	int ret = ntyExecuteScheduleDeleteHandle(fromId, toId, scheduleId);
	if (ret == -1) {
		ntylog(" ntyJsonDelScheduleAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ret = ntySendRecodeJsonPacket(fromId, toId, pActionParam->jsonstring, pActionParam->jsonlen);
		if (ret >= 0) {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);

			DelScheduleAck *pDelScheduleAck = (DelScheduleAck*)malloc(sizeof(DelScheduleAck));
			pDelScheduleAck->result = *(DelScheduleResult*)pDelScheduleReq;
			char *jsonresult = ntyJsonWriteDelSchedule(pDelScheduleAck);
			ntyJsonBroadCastRecvResult(fromId, toId, jsonresult, pActionParam->index);
			ntyJsonFree(jsonresult);
			free(pDelScheduleAck);
		}
	}
	free(pDelScheduleReq);
}

void ntyJsonUpdateScheduleAction(ActionParam *pActionParam) {
	UpdateScheduleReq *pUpdateScheduleReq = (UpdateScheduleReq*)malloc(sizeof(UpdateScheduleReq));
	ntyJsonUpdateSchedule(pActionParam->json, pUpdateScheduleReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;

	int scheduleId = 0;
	if (pUpdateScheduleReq->id != NULL) {
		if (strlen(pUpdateScheduleReq->id) != 0) {
			scheduleId = atoi(pUpdateScheduleReq->id);
		}
	}
	int ret = ntyExecuteScheduleUpdateHandle(fromId, toId, 
		scheduleId,
		pUpdateScheduleReq->schedule.daily,
		pUpdateScheduleReq->schedule.time,
		pUpdateScheduleReq->schedule.details);
	if (ret == -1) {
		ntylog(" ntyJsonUpdateScheduleAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ret = ntySendRecodeJsonPacket(fromId, toId, pActionParam->jsonstring, pActionParam->jsonlen);
		if (ret >= 0) {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);

			UpdateScheduleAck *pUpdateScheduleAck = (UpdateScheduleAck*)malloc(sizeof(UpdateScheduleAck));
			pUpdateScheduleAck->result = *(UpdateScheduleResult*)pUpdateScheduleReq;
			char *jsonresult = ntyJsonWriteUpdateSchedule(pUpdateScheduleAck);
			ntyJsonBroadCastRecvResult(fromId, toId, jsonresult, pActionParam->index);
			ntyJsonFree(jsonresult);
			free(pUpdateScheduleAck);
		}
	}
	free(pUpdateScheduleReq);
}


void ntyJsonSelectScheduleAction(ActionParam *pActionParam) {
	CommonReq *pCommonReq = (CommonReq*)malloc(sizeof(CommonReq));
	ntyJsonCommon(pActionParam->json, pCommonReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;

	ScheduleAck *pScheduleAck = (ScheduleAck*)malloc(sizeof(ScheduleAck));
	pScheduleAck->results.IMEI = pCommonReq->IMEI;
	pScheduleAck->results.category = pCommonReq->category;

	size_t size = 50;
	ScheduleItem *pSchedule = malloc(sizeof(ScheduleItem)*size);
	pScheduleAck->results.pSchedule = pSchedule;
	
	int ret = ntyExecuteScheduleSelectHandle(fromId, toId, pScheduleAck, size);
	if (ret == -1) {
		ntylog(" ntyJsonSelectScheduleAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ret = ntySendRecodeJsonPacket(fromId, toId, pActionParam->jsonstring, pActionParam->jsonlen);
		if (ret >= 0) {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);
			char *jsonresult = ntyJsonWriteSchedule(pScheduleAck);
			ntyJsonBroadCastRecvResult(fromId, toId, (U8*)jsonresult, pActionParam->index);
			ntyJsonFree(jsonresult);
			free(pScheduleAck);
		}
	}
	free(pSchedule);
	free(pCommonReq);
}


void ntyJsonTimeTablesAction(ActionParam *pActionParam) {
	TimeTablesReq *pTimeTablesReq = (TimeTablesReq*)malloc(sizeof(TimeTablesReq));
	ntyJsonTimeTables(pActionParam->json, pTimeTablesReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;
	
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
		ret = ntyExecuteTimeTablesUpdateHandle(fromId, toId, 
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
		ret = ntySendRecodeJsonPacket(fromId, toId, pActionParam->jsonstring, pActionParam->jsonlen);
		if (ret >= 0) {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);
			
			TimeTablesAck *pTimeTablesAck = malloc(sizeof(TimeTablesAck));
			pTimeTablesAck->results = *(TimeTablesResults*)pTimeTablesReq;
			char *jsonresult = ntyJsonWriteTimeTables(pTimeTablesAck);
			ntyJsonBroadCastRecvResult(fromId, toId, (U8*)jsonresult, pActionParam->index);
			ntyJsonFree(jsonresult);
			free(pTimeTablesAck);
		}
	}
	ntyJsonTimeTablesItemRelease(pTimeTablesReq->pTimeTables);
	free(pTimeTablesReq);
}

int checkStringIsAllNumber(const char *content) {
	if (content == NULL) {
		return 0;
	}

	size_t len = strlen(content);
	size_t i;
	for (i=0; i<len; i++) {
		char c = content[i];
		if (c<0x30 || c>0x39) {
			return -1;
		}
	}

	return 1;
}

void ntyJsonAddContactsAction(ActionParam *pActionParam) {
	AddContactsReq *pAddContactsReq = (AddContactsReq*)malloc(sizeof(AddContactsReq));
	ntyJsonAddContacts(pActionParam->json, pAddContactsReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;
	
	int contactsId = 0;
	int ret = ntyExecuteContactsInsertHandle(fromId, toId, &pAddContactsReq->contacts, &contactsId);
	char ids[20] = {0};
	sprintf(ids, "%d", contactsId);
	pAddContactsReq->contacts.id = ids;
	if (ret == -1) {
		ntylog(" ntyJsonAddContactsAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		CommonReq *pCommonReq = malloc(sizeof(CommonReq));
		ntyJsonCommon(pActionParam->json, pCommonReq);
		CommonReqExtend *pCommonReqExtend = malloc(sizeof(CommonReqExtend));
		pCommonReqExtend->IMEI = pCommonReq->IMEI;
		pCommonReqExtend->category = pCommonReq->category;
		pCommonReqExtend->action = pCommonReq->action;
		char ids[20] = {0};
		sprintf(ids, "%d", contactsId);
		char *jsonstringTemp = ntyJsonWriteCommonReqExtend(pCommonReqExtend);
		pCommonReqExtend->id = ids;
		ret = ntySendRecodeJsonPacket(fromId, toId, jsonstringTemp, strlen(jsonstringTemp));
		free(pCommonReq);
		free(pCommonReqExtend);
		if (ret >= 0) {
			ntyJsonCommonExtendResult(fromId, NATTY_RESULT_CODE_SUCCESS, contactsId);
			AddContactsAck *pAddContactsAck = malloc(sizeof(AddContactsAck));
			pAddContactsAck->results = *(AddContactsResults*)pAddContactsReq;
			char *jsonresult = ntyJsonWriteAddContacts(pAddContactsAck);
			ntySendCommonBroadCastResult(fromId, toId, (U8*)jsonresult, strlen(jsonresult));
			ntyJsonFree(jsonresult);
			free(pAddContactsAck);
		}
	}
	free(pAddContactsReq);

	/*
	AddContactsReq *pAddContactsReq = (AddContactsReq*)malloc(sizeof(AddContactsReq));
	ntyJsonAddContacts(pActionParam->json, pAddContactsReq);
	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;
	if (checkStringIsAllNumber(pAddContactsReq->contacts.telphone) != 1) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_JSON_CONVERT);
	} else {
		int contactsId = 0;
		int ret = ntyExecuteContactsInsertHandle(fromId, toId, &pAddContactsReq->contacts, &contactsId);
		char ids[20] = {0};
		sprintf(ids, "%d", contactsId);
		pAddContactsReq->contacts.id = ids;
		if (ret == -1) {
			ntylog(" ntyJsonAddContactsAction --> DB Exception\n");
			ret = 4;
		} else if (ret >= 0) {
			CommonReq *pCommonReq = malloc(sizeof(CommonReq));
			ntyJsonCommon(pActionParam->json, pCommonReq);
			CommonReqExtend *pCommonReqExtend = malloc(sizeof(CommonReqExtend));
			pCommonReqExtend->IMEI = pCommonReq->IMEI;
			pCommonReqExtend->category = pCommonReq->category;
			pCommonReqExtend->action = pCommonReq->action;
			char ids[20] = {0};
			sprintf(ids, "%d", contactsId);
			pCommonReqExtend->id = ids;
			char *jsonstringTemp = ntyJsonWriteCommonReqExtend(pCommonReqExtend);
			ret = ntySendRecodeJsonPacket(fromId, toId, jsonstringTemp, strlen(jsonstringTemp));
			if (ret >= 0) {
				ntyJsonCommonExtendResult(fromId, NATTY_RESULT_CODE_SUCCESS, contactsId);
				AddContactsAck *pAddContactsAck = malloc(sizeof(AddContactsAck));
				pAddContactsAck->results = *(AddContactsResults*)pAddContactsReq;
				char *jsonresult = ntyJsonWriteAddContacts(pAddContactsAck);
				ntyJsonBroadCastRecvResult(fromId, toId, (U8*)jsonresult, pActionParam->index);
				ntyJsonFree(jsonresult);
				free(pAddContactsAck);
			} else {
				ntySaveCommonMsgData(pActionParam);
			}
			free(pCommonReq);
			free(pCommonReqExtend);
		}
	}
	free(pAddContactsReq);
	*/
}

void ntyJsonUpdateContactsAction(ActionParam *pActionParam) {
	UpdateContactsReq *pUpdateContactsReq = (UpdateContactsReq*)malloc(sizeof(UpdateContactsReq));
	ntyJsonUpdateContacts(pActionParam->json, pUpdateContactsReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;
	
	int contactsId = 0;
	if (pUpdateContactsReq->contacts.id != NULL) {
		if (strlen(pUpdateContactsReq->contacts.id) != 0) {
			contactsId = atoi(pUpdateContactsReq->contacts.id);
		}
	}
	int ret = ntyExecuteContactsUpdateHandle(fromId, toId, &pUpdateContactsReq->contacts, contactsId);
	if (ret == -1) {
		ntylog(" ntyJsonUpdateContactsAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ret = ntySendRecodeJsonPacket(fromId, toId, pActionParam->jsonstring, pActionParam->jsonlen);
		if (ret >= 0) {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);

			UpdateContactsAck *pUpdateContactsAck = malloc(sizeof(UpdateContactsAck));
			pUpdateContactsAck->results = *(UpdateContactsResults*)pUpdateContactsReq;
			char *jsonresult = ntyJsonWriteUpdateContacts(pUpdateContactsAck);
			ntyJsonBroadCastRecvResult(fromId, toId, (U8*)jsonresult, pActionParam->index);
			ntyJsonFree(jsonresult);
			free(pUpdateContactsAck);
		}
	}
	free(pUpdateContactsReq);
}

void ntyJsonDelContactsAction(ActionParam *pActionParam) {
	DelContactsReq *pDelContactsReq = (DelContactsReq*)malloc(sizeof(DelContactsReq));
	ntyJsonDelContacts(pActionParam->json, pDelContactsReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;

	int contactsId = 0;
	if (pDelContactsReq->id != NULL) {
		if (strlen(pDelContactsReq->id) != 0) {
			contactsId = atoi(pDelContactsReq->id);
		}
	}
	int ret = ntyExecuteContactsDeleteHandle(fromId, toId, contactsId);
	if (ret == -1) {
		ntylog(" ntyJsonDelContactsAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ret = ntySendRecodeJsonPacket(fromId, toId, pActionParam->jsonstring, pActionParam->jsonlen);
		if (ret >= 0) {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);

			DelContactsAck *pDelContactsAck = malloc(sizeof(DelContactsAck));
			pDelContactsAck->results = *(DelContactsResults*)pDelContactsReq;
			char *jsonresult = ntyJsonWriteDelContacts(pDelContactsAck);
			ntyJsonBroadCastRecvResult(fromId, toId, (U8*)jsonresult, pActionParam->index);
			ntyJsonFree(jsonresult);
			free(pDelContactsAck);
		}
	}
	free(pDelContactsReq);
}


void ntyJsonOfflineMsgReqAction(ActionParam *pActionParam) {
	DelContactsReq *pDelContactsReq = (DelContactsReq*)malloc(sizeof(DelContactsReq));
	ntyJsonDelContacts(pActionParam->json, pDelContactsReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;
	
	int contactsId = 0;
	if (pDelContactsReq->id != NULL) {
		if (strlen(pDelContactsReq->id) != 0) {
			contactsId = atoi(pDelContactsReq->id);
		}
	}
	int ret = ntyExecuteContactsDeleteHandle(fromId, toId, contactsId);
	if (ret == -1) {
		ntylog(" ntyJsonDelContactsAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ret = ntySendRecodeJsonPacket(fromId, toId, pActionParam->jsonstring, pActionParam->jsonlen);
		if (ret >= 0) {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);

			DelContactsAck *pDelContactsAck = malloc(sizeof(DelContactsAck));
			pDelContactsAck->results = *(DelContactsResults*)pDelContactsReq;
			char *jsonresult = ntyJsonWriteDelContacts(pDelContactsAck);
			ntyJsonBroadCastRecvResult(fromId, toId, (U8*)jsonresult, pActionParam->index);
			ntyJsonFree(jsonresult);
			free(pDelContactsAck);
		}
	}
	free(pDelContactsReq);
}

int ntyVoiceDataReqAction(C_DEVID senderId, C_DEVID gId, char *filename) {

	U32 msgId = 0;

	//insert voice msg to db
	int ret = ntyQueryVoiceMsgInsertHandle(senderId, gId, filename, &msgId);
	if (ret != NTY_RESULT_FAILED) {
		ntyJsonCommonResult(senderId, NATTY_RESULT_CODE_SUCCESS);
	} else { //
		ntyJsonCommonResult(senderId, NATTY_RESULT_CODE_ERR_DB_OPERATION);
		return ret;
	}
	//broadcast to all id
	//json is null, length is 0
	ntySendVoiceBroadCastResult(senderId, gId, NULL, 0, msgId);

}

int ntyVoiceReqAction(C_DEVID fromId, U32 msgId) {

	C_DEVID senderId = 0;
	C_DEVID gId = 0;
	U8 filename[NTY_VOICE_FILENAME_LENGTH] = {0};
	long stamp = 0;
	
	int ret = ntyQueryVoiceMsgSelectHandle(msgId, &senderId, &gId, filename, &stamp);
	if (ret == NTY_RESULT_FAILED) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DB_NOEXIST);
		return ret;
	}

	ntylog(" ntyVoiceReqAction --> senderId:%lld, gId:%lld, filename:%s\n", senderId, gId, filename);
	U8 *pData = (U8 *)malloc(NTY_VOICEREQ_COUNT_LENGTH*NTY_VOICEREQ_PACKET_LENGTH);
	int size = ntyReadDat(filename, pData, NTY_VOICEREQ_COUNT_LENGTH*NTY_VOICEREQ_PACKET_LENGTH);
	
	ntylog(" ntyVoiceReqAction --> size:%d\n", size);

	ret = ntySendVoiceBufferResult(pData, size, senderId, gId, fromId, msgId);

	free(pData);

	return ret;
}

int ntyVoiceAckAction(C_DEVID fromId, U32 msgId) {

	int ret = ntyExecuteVoiceOfflineMsgDeleteHandle(msgId, fromId);
	if (ret == NTY_RESULT_FAILED) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DB_NOEXIST);
		return ret;
	}

	ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);

	//next voice packet
	//
	return NTY_RESULT_SUCCESS;
}

int ntyCommonReqSaveDBAction(C_DEVID fromId, C_DEVID gId, U8 *json) {

	
	
}


