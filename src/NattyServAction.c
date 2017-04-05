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
#include "NattyDaveMQ.h"

#include "NattyVector.h"

#include "NattyResult.h"
#include "NattyFilter.h"

#include <string.h>
#include <time.h>

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

int checkStringIsAllTimeChar(const char *content) {
	if (content == NULL) {
		return 0;
	}

	size_t len = strlen(content);
	size_t i;
	for (i=0; i<len; i++) {
		char c = content[i];
		if (!((c>=0x30 && c<=0x39) || c==0x3A)) {
			return -1;
		}
	}

	return 1;
}

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

void ntyJsonCommonContextResult(C_DEVID devId, const char *context) {
	ntylog("ntyJsonCommonExtendResult -> %s  %d \n", context, (int)strlen(context));
	ntySendDataResult(devId, (U8*)context, strlen(context), 200);
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
int ntySaveCommonMsgData(C_DEVID sid, C_DEVID gid, const char *jsonstring, int *msgId) {
	//C_DEVID sid = pActionParam->fromId;
	//C_DEVID gid = pActionParam->toId;
	//const char *jsonstring = pActionParam->jsonstring;

	if (jsonstring == NULL) {
		return -1;
	}

	//int ret = ntyExecuteCommonMsgInsertHandle(sid, gid, jsonstring, &pActionParam->index);
	//return ret;

	int ret = ntyExecuteCommonMsgInsertHandle(sid, gid, jsonstring, msgId);
	return ret;
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
	if (category == NULL) {
		ntylog("Can't find category, because category is null\n");
		return;
	}
	if (strcmp(category, NATTY_USER_PROTOCOL_CATEGORY_EFENCE) == 0) {
		const char *action = ntyJsonAction(pActionParam->json);
		if (action == NULL) {
			ntylog("Can't find action, because action is null\n");
			return;
		}
		if (strcmp(action, NATTY_USER_PROTOCOL_CATEGORY_ADD) == 0) {
			ntyJsonAddEfenceAction(pActionParam);
		} else if (strcmp(action, NATTY_USER_PROTOCOL_CATEGORY_DELETE) == 0) {
			ntyJsonDelEfenceAction(pActionParam);
		} else {
			ntylog("Can't find action with: %s\n", action);
		}
	} else if (strcmp(category, NATTY_USER_PROTOCOL_CATEGORY_RUNTIME) == 0) {
			ntyJsonRunTimeAction(pActionParam);
	} else if (strcmp(category, NATTY_USER_PROTOCOL_CATEGORY_TURN) == 0) {
			ntyJsonTurnAction(pActionParam);
	} else if (strcmp(category, NATTY_USER_PROTOCOL_CATEGORY_SCHEDULE) == 0) {
		const char *action = ntyJsonAction(pActionParam->json);
		if (action == NULL) {
			ntylog("Can't find action, because action is null\n");
			return;
		}
		if (strcmp(action, NATTY_USER_PROTOCOL_CATEGORY_ADD) == 0) {
			ntyJsonAddScheduleAction(pActionParam);
		} else if (strcmp(action, NATTY_USER_PROTOCOL_CATEGORY_DELETE) == 0) {
			ntyJsonDelScheduleAction(pActionParam);
		} else if (strcmp(action, NATTY_USER_PROTOCOL_CATEGORY_UPDATE) == 0) {
			ntyJsonUpdateScheduleAction(pActionParam);
		} else if (strcmp(action, NATTY_USER_PROTOCOL_CATEGORY_SCHEDULE) == 0) {
			ntyJsonSelectScheduleAction(pActionParam);
		} else {
			ntylog("Can't find action with: %s\n", action);
		}
	} else if (strcmp(category, NATTY_USER_PROTOCOL_CATEGORY_TIMETABLES) == 0) {
		ntyJsonTimeTablesAction(pActionParam);
	} else if (strcmp(category, NATTY_USER_PROTOCOL_CATEGORY_CONTACTS) == 0) {
		const char *action = ntyJsonAction(pActionParam->json);
		if (action == NULL) {
			ntylog("Can't find action, because action is null\n");
			return;
		}
		if (strcmp(action, NATTY_USER_PROTOCOL_CATEGORY_ADD) == 0) {
			ntyJsonAddContactsAction(pActionParam);
		} else if (strcmp(action, NATTY_USER_PROTOCOL_CATEGORY_DELETE) == 0) {
			ntyJsonDelContactsAction(pActionParam);
		} else if (strcmp(action, NATTY_USER_PROTOCOL_CATEGORY_UPDATE) == 0) {
			ntyJsonUpdateContactsAction(pActionParam);
		} else {
			ntylog("Can't find action with: %s\n", action);
		}
	} else {
		ntylog("Can't find category with: %s\n", category);
	}
}

void ntyJsonAddEfenceAction(ActionParam *pActionParam) {
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
	sprintf(runtime, "%04d/%02d/%02d %02d:%02d:%02d", 1900+p->tm_year, 1+p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	int index = 0;
	int check_index = checkStringIsAllNumber(pAddEfenceReq->index);
	if (check_index == 1) {
		index = atoi(pAddEfenceReq->index);
	}
	if (index<=0 || index > 3) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_JSON_DATA);
		goto exit;
	}

	int ret = ntySendRecodeJsonPacket(fromId, toId, pActionParam->jsonstring, pActionParam->jsonlen);
	if (ret < 0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
		goto exit;

	}
	int id = 0;
	ret = ntyExecuteEfenceInsertHandle(fromId, toId, index, pAddEfenceReq->efence.size, points, runtime, &id);
	if (ret == -1) {
		ntylog(" ntyJsonEfenceAction --> DB Exception\n");
		ret = 4;
	} else if (ret == 0 && id == -2) { 
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DB_SAVE_REPEATE_DATA);
	} else if (ret >= 0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);

		AddEfenceAck *pAddEfenceAck = (AddEfenceAck*)malloc(sizeof(AddEfenceAck));
		char ids[20] = {0};
		sprintf(ids, "%d", id);
		pAddEfenceAck->id = ids;

		char msgs[20] = {0};
		sprintf(msgs, "%d", pActionParam->index);
		pAddEfenceReq->msg = msgs;
		
		pAddEfenceAck->result = *(AddEfenceResult*)pAddEfenceReq;
		char *jsonresult = ntyJsonWriteAddEfence(pAddEfenceAck);
		
		ret = ntySaveCommonMsgData(
			pActionParam->fromId,
			pActionParam->toId,
			jsonresult,
			&pActionParam->index);
		//ret = ntySaveCommonMsgData(pActionParam);
		if (ret>=0) {
			ntyJsonCommonContextResult(toId, jsonresult);			
			ntylog(" ntySendCommonBroadCastResult --> %lld, %lld, %s, %d\n", fromId, toId, jsonresult, (int)strlen(jsonresult));
			ntySendCommonBroadCastResult(fromId, toId, (U8*)jsonresult, strlen(jsonresult));
		} else {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DB_SAVE_OFFLINE);
		}
		ntyJsonFree(jsonresult);
		free(pAddEfenceAck);
	}

exit:
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
	int check_index = checkStringIsAllNumber(pDelEfenceReq->index);
	if (check_index == 1) {
		index = atoi(pDelEfenceReq->index);
	}

	int ret = ntySendRecodeJsonPacket(fromId, devId, pActionParam->jsonstring, pActionParam->jsonlen);
	if (ret<0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
		goto exit;
	}
	ret = ntyExecuteEfenceDeleteHandle(fromId, toId, index);
	if (ret == -1) {
		ntylog(" ntyJsonDelEfenceAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		//ret = ntySaveCommonMsgData(pActionParam);
		
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);
		DelEfenceAck *pDelEfenceAck = (DelEfenceAck*)malloc(sizeof(DelEfenceAck));
		pDelEfenceAck->result = *(DelEfenceResult*)pDelEfenceReq;
		char msgs[20] = {0};
		sprintf(msgs, "%d", pActionParam->index);
		pDelEfenceReq->msg = msgs;
		
		char *jsonresult = ntyJsonWriteDelEfence(pDelEfenceAck);
		ret = ntySaveCommonMsgData(
			pActionParam->fromId,
			pActionParam->toId,
			jsonresult,
			&pActionParam->index);
		if (ret >= 0) {
			ntyJsonBroadCastRecvResult(fromId, toId, (U8*)jsonresult, pActionParam->index);
		} else {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DB_SAVE_OFFLINE);
		}
		ntyJsonFree(jsonresult);
		free(pDelEfenceAck);
	}

exit:
	free(pDelEfenceReq);
}

void ntyJsonICCIDAction(ActionParam *pActionParam) {
	ICCIDReq *pICCIDReq = (ICCIDReq*)malloc(sizeof(ICCIDReq));
	ntyJsonICCID(pActionParam->json, pICCIDReq);
	C_DEVID devId = pActionParam->toId;
	
	if (pICCIDReq==NULL) {
		ntyJsonCommonResult(devId, NATTY_RESULT_CODE_ERR_JSON_CONVERT);
		goto exit;
	}
	if (pICCIDReq->ICCID==NULL) {
		ntyJsonCommonResult(devId, NATTY_RESULT_CODE_ERR_JSON_CONVERT);
		goto exit;
	}
	int len = strlen(pICCIDReq->ICCID);
	if (len!=20) {
		ntyJsonCommonResult(devId, NATTY_RESULT_CODE_ERR_JSON_CONVERT);
		goto exit;
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
			goto exit;
		}

		//ret = ntySaveCommonMsgData(pActionParam);
		
		ICCIDAck *pICCIDAck = (ICCIDAck*)malloc(sizeof(ICCIDAck));
		pICCIDAck->IMEI = pICCIDReq->IMEI;
		pICCIDAck->phone_num = phonenum;

		char msgs[20] = {0};
		sprintf(msgs, "%d", pActionParam->index);
		pICCIDAck->msg = msgs;
		char *jsonstringICCID = ntyJsonWriteICCID(pICCIDAck);

		ret = ntySaveCommonMsgData(
			pActionParam->fromId,
			pActionParam->toId,
			jsonstringICCID,
			&pActionParam->index);
		if (ret >= 0) {
			ntySendICCIDAckResult(devId, (U8*)jsonstringICCID, strlen(jsonstringICCID), 200);
		} else {
			ntyJsonCommonResult(devId, NATTY_RESULT_CODE_ERR_DB_SAVE_OFFLINE);
		}
		ntyJsonFree(jsonstringICCID);
		free(pICCIDAck);
	}

exit:
	free(pICCIDReq);
}

void ntyJsonRunTimeAction(ActionParam *pActionParam) {
	RunTimeReq *pRunTimeReq = (RunTimeReq*)malloc(sizeof(RunTimeReq));
	ntyJsonRuntime(pActionParam->json, pRunTimeReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;
	
	int ret = 0;

	ret = ntySendRecodeJsonPacket(fromId, toId, pActionParam->jsonstring, pActionParam->jsonlen);
	if (ret < 0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
		goto exit;
	}
	
	ntydbg("---------------------1----------------------\n");
	
	if (pRunTimeReq->runtime.auto_connection != NULL) {
		size_t len_auto_connection = strlen(pRunTimeReq->runtime.auto_connection);
		if (len_auto_connection != 0) {
			int auto_connection = atoi(pRunTimeReq->runtime.auto_connection);
			ret = ntyExecuteRuntimeAutoConnUpdateHandle(fromId, toId, auto_connection);
			if (ret == -1) {
				ntylog(" ntyJsonRunTimeAction:AutoConnection --> DB Exception\n");
				goto exit;
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
				goto exit;
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
				goto exit;
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
				goto exit;
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
				goto exit;
			}
		}
	}
	ntydbg("---------------------6----------------------\n");
	if (ret == -1) {
		ntylog(" ntyJsonRunTimeAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		//ret = ntySaveCommonMsgData(pActionParam);
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);

		char msgs[20] = {0};
		sprintf(msgs, "%d", pActionParam->index);
		pRunTimeReq->msg = msgs;
		
		RunTimeAck *pRunTimeAck = (RunTimeAck*)malloc(sizeof(RunTimeAck));
		pRunTimeAck->result = *(RunTimeResult*)pRunTimeReq;			
		char *jsonresult = ntyJsonWriteRunTime(pRunTimeAck);

		ret = ntySaveCommonMsgData(
			pActionParam->fromId,
			pActionParam->toId,
			jsonresult,
			&pActionParam->index);
		if (ret >= 0) {
			ntyJsonBroadCastRecvResult(fromId, toId, jsonresult, pActionParam->index);
		} else {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
		}
		ntyJsonFree(jsonresult);
		free(pRunTimeAck);
	}
	ntydbg("---------------------7----------------------\n");

exit:
	free(pRunTimeReq);
}

void ntyJsonTurnAction(ActionParam *pActionParam) {
	TurnReq *pTurnReq = (TurnReq*)malloc(sizeof(TurnReq));
	ntyJsonTurn(pActionParam->json, pTurnReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;
	
	U8 status = 0;
	int check_status = checkStringIsAllNumber(pTurnReq->turn.status);
	if (check_status == 1) {
		status = atoi(pTurnReq->turn.status);
	}

	char *turn_on = malloc(50);
	memset(turn_on, 0, 50);
	int check_on = checkStringIsAllTimeChar(pTurnReq->turn.on.time);
	if (check_on != 1) {
		strcat(turn_on, "09:00:00");
	} else {
		memcpy(turn_on, pTurnReq->turn.on.time, sizeof(pTurnReq->turn.on.time));
	}
	char *turn_off = malloc(50);
	memset(turn_off, 0, 50);
	int check_off = checkStringIsAllTimeChar(pTurnReq->turn.off.time);
	if (check_off != 1) {
		strcat(turn_off, "18:00:00");
	} else {
		memcpy(turn_off, pTurnReq->turn.off.time, sizeof(pTurnReq->turn.off.time));
	}
	int ret = ntySendRecodeJsonPacket(fromId, toId, pActionParam->jsonstring, pActionParam->jsonlen);
	if (ret<0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
		goto exit;
	}
	ret = ntyExecuteTurnUpdateHandle(fromId, toId, status, turn_on, turn_off);
	if (ret == -1) {
		ntylog(" ntyJsonTurnAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		//ret = ntySaveCommonMsgData(pActionParam);
		
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);
		char msgs[20] = {0};
		sprintf(msgs, "%d", pActionParam->index);
		pTurnReq->msg = msgs;
		
		TurnAck *pTurnAck = (TurnAck*)malloc(sizeof(TurnAck));
		pTurnAck->result = *(TurnResult*)pTurnReq;
		char *jsonresult = ntyJsonWriteTurn(pTurnAck);

		ret = ntySaveCommonMsgData(
			pActionParam->fromId,
			pActionParam->toId,
			jsonresult,
			&pActionParam->index);
		if (ret >= 0) {
			ntyJsonBroadCastRecvResult(fromId, toId, jsonresult, pActionParam->index);
		} else {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DB_SAVE_OFFLINE);
		}

		ntyJsonFree(jsonresult);
		free(pTurnAck);
	}
	free(turn_on);
	free(turn_off);
	
exit:
	free(pTurnReq);
}

void ntyJsonAddScheduleAction(ActionParam *pActionParam) {
	AddScheduleReq *pAddScheduleReq = (AddScheduleReq*)malloc(sizeof(AddScheduleReq));
	ntyJsonAddSchedule(pActionParam->json, pAddScheduleReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;

	int status = 0;
	int check_status = checkStringIsAllNumber(pAddScheduleReq->schedule.status);
	if (check_status == 1) {
		status = atoi(pAddScheduleReq->schedule.status);
	}
	
	int scheduleId = 0;
	int ret = ntyExecuteScheduleInsertHandle(fromId, toId,
		pAddScheduleReq->schedule.daily,
		pAddScheduleReq->schedule.time,
		status,
		pAddScheduleReq->schedule.details,
		&scheduleId);
	if (ret == -1) {
		ntylog(" ntyJsonAddScheduleAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		//ret = ntySaveCommonMsgData(pActionParam);
		//if (ret >= 0) {
		char ids[20] = {0};
		sprintf(ids, "%d", scheduleId);
		pAddScheduleReq->id = ids;

		char msgs[20] = {0};
		sprintf(msgs, "%d", pActionParam->index);
		pAddScheduleReq->msg = msgs;
		
		DeviceAddScheduleAck *pDeviceAddScheduleAck = (DeviceAddScheduleAck*)malloc(sizeof(DeviceAddScheduleAck));
		pDeviceAddScheduleAck = (DeviceAddScheduleAck*)pAddScheduleReq;
		pDeviceAddScheduleAck->id = ids;
		char *jsondeviceresult = ntyJsonWriteDeviceAddSchedule(pDeviceAddScheduleAck);
		ret = ntySendRecodeJsonPacket(fromId, toId, jsondeviceresult, strlen(jsondeviceresult));
		if (ret<0) {
			ntyExecuteScheduleDeleteHandle(fromId,toId,scheduleId);
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
		} else {
			AddScheduleAck *pAddScheduleAck = (AddScheduleAck*)malloc(sizeof(AddScheduleAck));
			pAddScheduleAck->result = *(AddScheduleResult*)pAddScheduleReq;
			pAddScheduleAck->result.id = ids;
			char *jsonresult = ntyJsonWriteAddSchedule(pAddScheduleAck);

			ret = ntySaveCommonMsgData(
				pActionParam->fromId,
				pActionParam->toId,
				jsonresult,
				&pActionParam->index);
			if (ret >= 0) {
				ntyJsonCommonExtendResult(fromId, NATTY_RESULT_CODE_SUCCESS, scheduleId);
				ntyJsonBroadCastRecvResult(fromId, toId, jsonresult, pActionParam->index);
			} else {
				ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DB_SAVE_OFFLINE);
			}
			ntyJsonFree(jsonresult);
			free(pAddScheduleAck);
		}
		ntyJsonFree(jsondeviceresult);
		free(pDeviceAddScheduleAck);
	}
exit:
	free(pAddScheduleReq);
}

void ntyJsonDelScheduleAction(ActionParam *pActionParam) {
	DelScheduleReq *pDelScheduleReq = (DelScheduleReq*)malloc(sizeof(DelScheduleReq));
	ntyJsonDelSchedule(pActionParam->json, pDelScheduleReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;
	
	int scheduleId = 0;
	int check_schedule = checkStringIsAllNumber(pDelScheduleReq->id);
	if (check_schedule == 1) {
		scheduleId = atoi(pDelScheduleReq->id);
	}

	int ret = ntySendRecodeJsonPacket(fromId, toId, pActionParam->jsonstring, pActionParam->jsonlen);
	if (ret < 0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
		goto exit;
	}
	ret = ntyExecuteScheduleDeleteHandle(fromId, toId, scheduleId);
	if (ret == -1) {
		ntylog(" ntyJsonDelScheduleAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		//ret = ntySaveCommonMsgData(pActionParam);

		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);

		char msgs[20] = {0};
		sprintf(msgs, "%d", pActionParam->index);
		pDelScheduleReq->msg = msgs;
		
		DelScheduleAck *pDelScheduleAck = (DelScheduleAck*)malloc(sizeof(DelScheduleAck));
		pDelScheduleAck->result = *(DelScheduleResult*)pDelScheduleReq;
		
		char *jsonresult = ntyJsonWriteDelSchedule(pDelScheduleAck);
		ret = ntySaveCommonMsgData(
			pActionParam->fromId,
			pActionParam->toId,
			jsonresult,
			&pActionParam->index);
		if (ret >= 0) {
			ntyJsonBroadCastRecvResult(fromId, toId, jsonresult, pActionParam->index);
		} else {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DB_SAVE_OFFLINE);
		}
		ntyJsonFree(jsonresult);
		free(pDelScheduleAck);
	}
	
exit:
	free(pDelScheduleReq);
}

void ntyJsonUpdateScheduleAction(ActionParam *pActionParam) {
	UpdateScheduleReq *pUpdateScheduleReq = (UpdateScheduleReq*)malloc(sizeof(UpdateScheduleReq));
	ntyJsonUpdateSchedule(pActionParam->json, pUpdateScheduleReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;

	int status = 0;
	int check_status = checkStringIsAllNumber(pUpdateScheduleReq->schedule.status);
	if (check_status == 1) {
		status = atoi(pUpdateScheduleReq->schedule.status);
	}
	
	int scheduleId = 0;
	int check_schedule = checkStringIsAllNumber(pUpdateScheduleReq->id);
	if (check_schedule == 1) {
		scheduleId = atoi(pUpdateScheduleReq->id);
	}

	int ret = ntySendRecodeJsonPacket(fromId, toId, pActionParam->jsonstring, pActionParam->jsonlen);
	if (ret < 0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
		goto exit;
	}
	ret = ntyExecuteScheduleUpdateHandle(fromId, toId, 
		scheduleId,
		pUpdateScheduleReq->schedule.daily,
		pUpdateScheduleReq->schedule.time,
		status,
		pUpdateScheduleReq->schedule.details);
	if (ret == -1) {
		ntylog(" ntyJsonUpdateScheduleAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		//ret = ntySaveCommonMsgData(pActionParam);

		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);

		char msgs[20] = {0};
		sprintf(msgs, "%d", pActionParam->index);
		pUpdateScheduleReq->msg = msgs;
		
		UpdateScheduleAck *pUpdateScheduleAck = (UpdateScheduleAck*)malloc(sizeof(UpdateScheduleAck));
		pUpdateScheduleAck->result = *(UpdateScheduleResult*)pUpdateScheduleReq;
		char *jsonresult = ntyJsonWriteUpdateSchedule(pUpdateScheduleAck);

		ret = ntySaveCommonMsgData(
			pActionParam->fromId,
			pActionParam->toId,
			jsonresult,
			&pActionParam->index);
		if (ret >= 0) {
			ntyJsonBroadCastRecvResult(fromId, toId, jsonresult, pActionParam->index);
		} else {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DB_SAVE_OFFLINE);
		}
		ntyJsonFree(jsonresult);
		free(pUpdateScheduleAck);
	}
	
exit:
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

	int ret = ntySendRecodeJsonPacket(fromId, toId, pActionParam->jsonstring, pActionParam->jsonlen);
	if (ret < 0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
		goto exit;
	}
	
	ret = ntyExecuteScheduleSelectHandle(fromId, toId, pScheduleAck, size);
	if (ret == -1) {
		ntylog(" ntyJsonSelectScheduleAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);

		char msgs[20] = {0};
		sprintf(msgs, "%d", pActionParam->index);
		pScheduleAck->results.msg = msgs;
			
		char *jsonresult = ntyJsonWriteSchedule(pScheduleAck);
		ret = ntySaveCommonMsgData(
			pActionParam->fromId,
			pActionParam->toId,
			jsonresult,
			&pActionParam->index);
		if (ret >= 0) {
			ntyJsonBroadCastRecvResult(fromId, toId, (U8*)jsonresult, pActionParam->index);
		} else {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DB_SAVE_OFFLINE);
		}
		ntyJsonFree(jsonresult);
		free(pScheduleAck);
	}
	
exit:
	free(pSchedule);
	free(pCommonReq);
}


void ntyJsonTimeTablesAction(ActionParam *pActionParam) {
	TimeTablesReq *pTimeTablesReq = (TimeTablesReq*)malloc(sizeof(TimeTablesReq));
	ntyJsonTimeTables(pActionParam->json, pTimeTablesReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;
	
	char ids[20] = {0};

	int result = 0;
	int ret = ntySendRecodeJsonPacket(fromId, toId, pActionParam->jsonstring, pActionParam->jsonlen);
	if (ret < 0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
		goto exit;
	}
	
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
			&result);

		sprintf(ids, "%d", result);
		pTimeTablesReq->pTimeTables[i].id = ids;
		
		if (ret == -1) {
			ntylog(" ntyJsonTimeTables --> DB Exception\n");
			ret = 4;
		}
	}
	
	if (ret == -1) {
		ntylog(" ntyJsonTimeTables --> DB Exception\n");
		ret = 4;
	} else if (ret == 0 && result == -2) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DB_NOEXIST);
	} else if (ret >= 0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);
		
		TimeTablesAck *pTimeTablesAck = malloc(sizeof(TimeTablesAck));
		pTimeTablesAck->results = *(TimeTablesResults*)pTimeTablesReq;
		char msgs[20] = {0};
		sprintf(msgs, "%d", pActionParam->index);
		pTimeTablesAck->results.msg = msgs;
		
		char *jsonresult = ntyJsonWriteTimeTables(pTimeTablesAck);
		ret = ntySaveCommonMsgData(
			pActionParam->fromId,
			pActionParam->toId,
			jsonresult,
			&pActionParam->index);
		if (ret >= 0) {
			ntyJsonBroadCastRecvResult(fromId, toId, (U8*)jsonresult, pActionParam->index);
		} else {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DB_SAVE_OFFLINE);
		}
		ntyJsonFree(jsonresult);
		free(pTimeTablesAck);
	}

exit:
	ntyJsonTimeTablesItemRelease(pTimeTablesReq->pTimeTables);
	free(pTimeTablesReq);
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
	} else if (ret == 0 && contactsId==-2) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DB_SAVE_REPEATE_DATA);
	} else if (ret >= 0) {
		char ids[20] = {0};
		sprintf(ids, "%d", contactsId);
		pAddContactsReq->id = ids;

		char msgs[20] = {0};
		sprintf(msgs, "%d", pActionParam->index);
		pAddContactsReq->msg = msgs;
		
		DeviceAddContactsAck *pDeviceAddContactsAck = (DeviceAddContactsAck*)malloc(sizeof(DeviceAddContactsAck));
		pDeviceAddContactsAck = (DeviceAddContactsAck*)pAddContactsReq;
		pDeviceAddContactsAck->id = ids;
		char *jsondeviceresult = ntyJsonWriteDeviceAddContacts(pDeviceAddContactsAck);
		ret = ntySendRecodeJsonPacket(fromId, toId, (U8*)jsondeviceresult, strlen(jsondeviceresult));
		if (ret < 0) {
			ntyExecuteContactsDeleteHandle(fromId, toId, contactsId);
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
		} else {
			ntyJsonCommonExtendResult(fromId, NATTY_RESULT_CODE_SUCCESS, contactsId);
			
			AddContactsAck *pAddContactsAck = malloc(sizeof(AddContactsAck));
			pAddContactsAck->results = *(AddContactsResults*)pAddContactsReq;
			char *jsonresult = ntyJsonWriteAddContacts(pAddContactsAck);
			ret = ntySaveCommonMsgData(
				pActionParam->fromId,
				pActionParam->toId, 
				jsonresult,
				&pActionParam->index);
			ntylog(" ntyJsonAddContactsAction--->ret %d %s %d\n", ret, jsonresult,strlen(jsonresult));
			if (ret >= 0) {
				ntySendCommonBroadCastResult(fromId, toId, (U8*)jsonresult, strlen(jsonresult));
			}// else {
				//ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DB_SAVE_OFFLINE);
			//}
			ntyJsonFree(jsonresult);
			free(pAddContactsAck);
		}
		free(jsondeviceresult);
		free(pDeviceAddContactsAck);
	}

exit:
	free(pAddContactsReq);
}

void ntyJsonUpdateContactsAction(ActionParam *pActionParam) {
	UpdateContactsReq *pUpdateContactsReq = (UpdateContactsReq*)malloc(sizeof(UpdateContactsReq));
	ntyJsonUpdateContacts(pActionParam->json, pUpdateContactsReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;
	
	int contactsId = 0;
	int check_contacts = checkStringIsAllNumber(pUpdateContactsReq->contacts.id);
	if (check_contacts == 1) {
		contactsId = atoi(pUpdateContactsReq->contacts.id);
	}

	int ret = ntySendRecodeJsonPacket(fromId, toId, pActionParam->jsonstring, pActionParam->jsonlen);
	if (ret < 0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
		goto exit;
	}
	
	ret = ntyExecuteContactsUpdateHandle(fromId, toId, &pUpdateContactsReq->contacts, contactsId);
	if (ret == -1) {
		ntylog(" ntyJsonUpdateContactsAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);

		char msgs[20] = {0};
		sprintf(msgs, "%d", pActionParam->index);
		pUpdateContactsReq->msg = msgs;
		
		UpdateContactsAck *pUpdateContactsAck = malloc(sizeof(UpdateContactsAck));
		pUpdateContactsAck->results = *(UpdateContactsResults*)pUpdateContactsReq;
		char *jsonresult = ntyJsonWriteUpdateContacts(pUpdateContactsAck);
		ret = ntySaveCommonMsgData(
			pActionParam->fromId,
			pActionParam->toId,
			jsonresult,
			&pActionParam->index);
		if (ret >= 0) {
			ntyJsonBroadCastRecvResult(fromId, toId, (U8*)jsonresult, pActionParam->index);
		} else {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DB_SAVE_OFFLINE);
		}
		ntyJsonFree(jsonresult);
		free(pUpdateContactsAck);
	}

exit:
	free(pUpdateContactsReq);
}

void ntyJsonDelContactsAction(ActionParam *pActionParam) {
	DelContactsReq *pDelContactsReq = (DelContactsReq*)malloc(sizeof(DelContactsReq));
	ntyJsonDelContacts(pActionParam->json, pDelContactsReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;

	int contactsId = 0;
	int check_contacts = checkStringIsAllNumber(pDelContactsReq->id);
	if (check_contacts == 1) {
		contactsId = atoi(pDelContactsReq->id);
	}

	int ret = ntySendRecodeJsonPacket(fromId, toId, pActionParam->jsonstring, pActionParam->jsonlen);
	if (ret < 0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
		goto exit;
	}
	
	ret = ntyExecuteContactsDeleteHandle(fromId, toId, contactsId);
	if (ret == -1) {
		ntylog(" ntyJsonDelContactsAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);
		char msgs[20] = {0};
		sprintf(msgs, "%d", pActionParam->index);
		pDelContactsReq->msg = msgs;
			
		DelContactsAck *pDelContactsAck = malloc(sizeof(DelContactsAck));
		pDelContactsAck->results = *(DelContactsResults*)pDelContactsReq;
		char *jsonresult = ntyJsonWriteDelContacts(pDelContactsAck);
		ret = ntySaveCommonMsgData(
			pActionParam->fromId,
			pActionParam->toId,
			jsonresult,
			&pActionParam->index);
		if (ret >= 0) {
			ntyJsonBroadCastRecvResult(fromId, toId, (U8*)jsonresult, pActionParam->index);
		} else {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
		}
		ntyJsonFree(jsonresult);
		free(pDelContactsAck);
	}

exit:
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
		} else {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
		}
	}
	free(pDelContactsReq);
}

int ntyVoiceDataReqAction(C_DEVID senderId, C_DEVID gId, char *filename) {
#if 0
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
#else

	VALUE_TYPE *tag = malloc(sizeof(VALUE_TYPE));
	if (tag == NULL) return NTY_RESULT_FAILED;

	int sLen = strlen(filename);
	tag->Tag = malloc(sLen+1);
	if (tag->Tag == NULL) {
		free(tag);
		return ;
	}
	memset(tag->Tag, 0, sLen+1);

	memcpy(tag->Tag, filename, sLen);
	tag->length = sLen;			
	tag->fromId = senderId;
	tag->toId = gId;
	tag->Type = MSG_TYPE_VOICE_DATA_REQ_HANDLE;
	tag->cb = ntyVoiceDataReqHandle;

	ntyDaveMqPushMessage(tag);

#endif
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
	int filename_len = (int)strlen(filename);
	if (filename_len==0) {
		ntydbg("filename length is %d\n", filename_len);
		return -1;
	}

	ntylog(" ntyVoiceReqAction --> senderId:%lld, gId:%lld, filename:%s\n", senderId, gId, filename);
	U8 *pData = (U8 *)malloc(NTY_VOICEREQ_COUNT_LENGTH*NTY_VOICEREQ_PACKET_LENGTH);
	int size = ntyReadDat(filename, pData, NTY_VOICEREQ_COUNT_LENGTH*NTY_VOICEREQ_PACKET_LENGTH);
	
	ntylog(" ntyVoiceReqAction --> size:%d\n", size);
	if (size == -1) {
		free(pData);
		return NTY_RESULT_FAILED;
	}
	ret = ntySendVoiceBufferResult(pData, size, senderId, gId, fromId, msgId);

	free(pData);

	return ret;
}

int ntyVoiceAckAction(C_DEVID fromId, U32 msgId) {

#if 0
	int ret = ntyExecuteVoiceOfflineMsgDeleteHandle(msgId, fromId);
	if (ret == NTY_RESULT_FAILED) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DB_NOEXIST);
		return ret;
	}

	ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);
#else

	VALUE_TYPE *tag = malloc(sizeof(VALUE_TYPE));
	memset(tag, 0, sizeof(VALUE_TYPE));

	tag->fromId = fromId;
	tag->arg = msgId;
	tag->Type = MSG_TYPE_VOICE_ACK_HANDLE;
	tag->cb = ntyVoiceAckHandle;

	ntyDaveMqPushMessage(tag);
	
#endif
	//next voice packet
	//
	return NTY_RESULT_SUCCESS;
}

int ntyCommonReqSaveDBAction(C_DEVID fromId, C_DEVID gId, U8 *json) {
	return 0;
}

int ntyBindReqAction(ActionParam *pActionParam) {
	C_DEVID admin = 0;

	if (pActionParam == NULL) return NTY_RESULT_ERROR;
	
	C_DEVID fromId = pActionParam->fromId;
	C_DEVID devId = pActionParam->toId;
	
	int ret = ntyQueryDevAppGroupCheckSelectHandle(fromId, devId);
	if (NTY_RESULT_SUCCESS != ret) {
		if (NTY_RESULT_EXIST == ret) {		//代表 UserId不存在
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_USER_NOTONLINE);
		} else if (NTY_RESULT_PROCESS == ret) {		//代表DeviceId不存在
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
		} else if (NTY_RESULT_NOEXIST == ret) {		//代表UserId与DeviceId已经绑定过了
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_BIND_REPEATE_DATA);
		} else if (NTY_RESULT_NEEDINSERT == ret) {	//代表设备没有激活
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_NOACTIVATE);
		}
		return;
	}

	BindReq *pBindReq = malloc(sizeof(BindReq));
	if (pBindReq == NULL) return NTY_RESULT_ERROR;
	ntyJsonBind(pActionParam->json, pBindReq);
	
	//Parser JSON and write here
	//查询管理员
	if(NTY_RESULT_FAILED == ntyQueryAdminSelectHandle(devId, &admin)) {
		ret = 4;
	} else {
		//insert to db and write here
		C_DEVID proposer = fromId;
		U8 *name = (U8*)pBindReq->bind.watchName;
		U8 *wimage = (U8*)pBindReq->bind.watchImage;
		U8 *call = (U8*)pBindReq->bind.userName;
		U8 *uimage = (U8*)pBindReq->bind.userImage;

		ntylog("WatchName:%s, UserName:%s\n", pBindReq->bind.watchName, pBindReq->bind.userName);
	
		if (admin == 0 || admin == fromId) { //first bind device
			//将管理员添加到新创建的群组
			ret = ntyQueryAdminGroupInsertHandle(devId, name, proposer, call, wimage, uimage);
			if (ret == 0) {
				ntyProtoBindAck(fromId, devId, 5);
			} else {
				ntyProtoBindAck(fromId, devId, ret);
			}
			return ret;
		} else {
			ntyProtoBindAck(fromId, devId, ret);
		}
		ntylog(" ntyQueryAdminSelectHandle --> %lld\n", admin);

		int msgId = 0;
		//绑定确认添加
		ntyQueryBindConfirmInsertHandle(admin, devId, name, wimage, proposer, call, uimage, &msgId);

		ntydbg("-----------------------query phonebook select---------------------------------\n");
		char phonenum[30] = {0};
		ntyQueryPhoneBookSelectHandle(devId, proposer, phonenum);

		ntydbg("-----------------------encode JSON write here---------------------------------\n");
		//Encode JSON write here
		BindConfirmPush *pBindConfirmPush = malloc(sizeof(BindConfirmPush));
		pBindConfirmPush->result.IMEI = pBindReq->IMEI;
		pBindConfirmPush->result.proposer = phonenum;
		pBindConfirmPush->result.userName = pBindReq->bind.userName;
		char ids[30] = {0};
		sprintf(ids, "%d", msgId);
		pBindConfirmPush->result.msgId = ids;
		char *jsonstring = ntyJsonWriteBindConfirmPush(pBindConfirmPush);

		ntydbg("-----------------------send json to admin---------------------------------\n");
		//send json to admin
		ntySendBindConfirmPushResult(proposer, admin, (U8*)jsonstring, strlen(jsonstring));

		//Radio broadcast
		ntydbg("-----------------------radio broadcast---------------------------------\n");

		free(pBindConfirmPush);
	}
	free(pBindReq);

	return ret;
}

int ntyOfflineAction(C_DEVID fromId, U32 msgId) {
	int ret = ntyExecuteCommonOfflineMsgDeleteHandle(msgId, fromId);
	if (ret == NTY_RESULT_FAILED) {
		ntylog("ntyExecuteCommonOfflineMsgDeleteHandle DB Error \n");
	}

	return 0;
}

int ntyReadOfflineCommonMsgIter(void *self, void *arg) {
	CommonOfflineMsg *pCommonOfflineMsg = (CommonOfflineMsg*)self;
	C_DEVID fromId = *(C_DEVID*)arg;
	char *json = pCommonOfflineMsg->details;
	if (json == NULL) {
		return -1;
	}
	
	int ret = ntySendCommonBroadCastItem(
		pCommonOfflineMsg->senderId, 
		fromId, 
		json, 
		strlen(json), 
		pCommonOfflineMsg->msgId);
	if (ret<0) {
		ntyJsonCommonResult(pCommonOfflineMsg->senderId, NATTY_RESULT_CODE_ERR_DB_SEND_OFFLINE);
	}

	free(json);
	
	return 0;
}


int ntyReadOfflineCommonMsgAction(C_DEVID devId) {
	NVector *container = ntyVectorCreator();
	if (NULL == container) return NTY_RESULT_FAILED;

	int ret = ntyQueryCommonOfflineMsgSelectHandle(devId, container);
	if (ret == NTY_RESULT_SUCCESS) {
		ntyVectorIter(container, ntyReadOfflineCommonMsgIter, &devId);
	} 
	if (container->num == 0) { //count == 0
		ret = NTY_RESULT_NOEXIST;
	} else {
		ret = 0;
	}
	
	ntyVectorDestory(container);

	return ret;
}





int ntyReadOfflineVoiceMsgIter(void *self, void *arg) {

	nOfflineMsg *pOfflineMsg = (nOfflineMsg*)self;
	C_DEVID senderId = pOfflineMsg->senderId;
	C_DEVID gId = pOfflineMsg->groupId;
	U32 msgId = pOfflineMsg->msgId;
	C_DEVID toId = *(C_DEVID*)arg;

	ntySendVoiceBroadCastItem(senderId, toId, NULL, 0, msgId);
}


int ntyReadOfflineVoiceMsgAction(C_DEVID devId) {
	NVector *container = ntyVectorCreator();
	if (NULL == container) return NTY_RESULT_FAILED;

	int ret = ntyQueryVoiceOfflineMsgSelectHandle(devId, container);
	if (ret == NTY_RESULT_SUCCESS) {
		ntyVectorIter(container, ntyReadOfflineVoiceMsgIter, &devId);
	} 
	if (container->num == 0) { //count == 0
		ret = NTY_RESULT_NOEXIST;
	} else {
		ret = 0;
	}
	
	ntyVectorDestory(container);

	return ret;
}



