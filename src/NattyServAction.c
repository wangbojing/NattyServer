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


int checkStringIsDouble(const char *content) {
	if (content == NULL) {
		return 0;
	}

	size_t len = strlen(content);
	size_t i;
	for (i=0; i<len; i++) {
		char c = content[i];
		if ((c<0x30 || c>0x39) || c == 0x2E) {
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

int ntyJsonBroadCastRecvResult(C_DEVID fromId, C_DEVID toId, char *jsonresult, U32 index) {
	if (jsonresult == NULL) {
		return - 1;
	}

	ntylog("ntyJsonBroadCastRecvResult --> %lld %lld  %s %d %d\n", fromId, toId, (U8*)jsonresult, (int)strlen(jsonresult), index);
	return ntySendCommonBroadCastResult(fromId, toId, (U8*)jsonresult, (int)strlen(jsonresult), index);
}

void ntyJsonCommonResult(C_DEVID devId, const char * code) {
	CommonAck *pCommonAck = (CommonAck*)malloc(sizeof(CommonAck));
	if (pCommonAck == NULL) {
		ntylog("ntyJsonCommonResult --> malloc CommonAck failed ");
		return ;
	}
	memset(pCommonAck, 0, sizeof(CommonAck));

	ntylog("ntyJsonCommonResult --> ntyCommonResultMessage \n");
	pCommonAck->result.code = code;
#if 0
	pCommonAck->result.message = ntyCommonResultMessage(pCommonAck->result.code);
#else
	char message[128] = {0};
	ntyCommonResultMessage(pCommonAck->result.code, message);
	pCommonAck->result.message = message;
#endif

	ntylog("ntyJsonCommonResult --> ntyJsonWriteCommon : %s\n", message);

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
	if (pCommonExtendAck == NULL) {
		ntylog("ntyJsonCommonExtendResult --> malloc CommonExtendAck failed\n");
		return ;
	}
	memset(pCommonExtendAck, 0, sizeof(CommonExtendAck));
	
	pCommonExtendAck->id = ids;
	sprintf(ids, "%d", id);
	pCommonExtendAck->result.code = code;
#if 0
	pCommonExtendAck->result.message = ntyCommonResultMessage(pCommonExtendAck->result.code);
#else
	char message[128] = {0};
	ntyCommonResultMessage(pCommonExtendAck->result.code, message);
	pCommonExtendAck->result.message = message;
#endif
	
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
int ntyJsonLocationWIFIAction(ActionParam *pActionParam) {
	if (pActionParam == NULL) return NTY_RESULT_FAILED;
	
	WIFIReq *pWIFIReq = (WIFIReq*)malloc(sizeof(WIFIReq));
	if (pWIFIReq == NULL) {
		return NTY_RESULT_ERROR;
	}
	memset(pWIFIReq, 0, sizeof(WIFIReq));

	ntylog("ntyJsonLocationWIFIAction --> ntyJsonWIFI\n");
	
	ntyJsonWIFI(pActionParam->json, pWIFIReq);
	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;

	ntylog("ntyJsonLocationWIFIAction --> macs\n");
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
	ntylog(" wifibuf --> %s\n", wifibuf);
	int length = strlen(wifibuf);

#if 1 //Push to MessageQueue
	MessageTag *pMessageTag = malloc(sizeof(MessageTag));
	if (pMessageTag == NULL) {
		free(pWIFIReq);
		
		return NTY_RESULT_ERROR;
	}
	memset(pMessageTag, 0, sizeof(MessageTag));
		
	pMessageTag->Type = MSG_TYPE_LOCATION_WIFI_API;
	pMessageTag->fromId = fromId;
	pMessageTag->toId = toId;
	pMessageTag->length = length;

#if ENABLE_DAVE_MSGQUEUE_MALLOC
	pMessageTag->Tag = malloc((length+1)*sizeof(U8));
	if (pMessageTag->Tag == NULL) {
		free(pMessageTag);
		free(pWIFIReq);
		
		return NTY_RESULT_ERROR;
	}
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
int ntyJsonLocationLabAction(ActionParam *pActionParam) {
	ntylog(" ntyJsonLocationLabAction begin --> \n");
	if (pActionParam == NULL) {
		return NTY_RESULT_FAILED;
	}
	
	LABReq *pLABReq = (LABReq*)malloc(sizeof(LABReq));
	if (pLABReq == NULL) {
		return NTY_RESULT_ERROR;
	}
	memset(pLABReq, 0, sizeof(LABReq));
	
	ntyJsonLAB(pActionParam->json, pLABReq);
	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;

	ntylog("-----------ntyJsonLocationLabAction nearbts----------- \n");
	U8 nearbts[PACKET_BUFFER_SIZE] = {0};
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

	ntylog("-----------ntyJsonLocationLabAction labbuf----------- \n");
	U8 labbuf[PACKET_BUFFER_SIZE] = {0};
	sprintf(labbuf, "%s/position?accesstype=0&imei=%s&cdma=0&bts=%s&nearbts=%s&output=json&key=%s", 
		HTTP_GAODE_BASE_URL, pLABReq->IMEI, pLABReq->lab.bts, nearbts, HTTP_GAODE_KEY);
	ntylog(" labbuf --> %s\n", labbuf);
	int length = strlen(labbuf);

	MessageTag *pMessageTag = malloc(sizeof(MessageTag));
	if (pMessageTag == NULL) {
		free(pLABReq);
		return NTY_RESULT_ERROR;
	}
	memset(pMessageTag, 0, sizeof(MessageTag));
	
	pMessageTag->Type = MSG_TYPE_LOCATION_LAB_API;
	pMessageTag->fromId = fromId;
	pMessageTag->toId = toId;

	pMessageTag->length = length;

#if ENABLE_DAVE_MSGQUEUE_MALLOC
	pMessageTag->Tag = malloc((length+1)*sizeof(U8));
	if (pMessageTag->Tag == NULL) {
		free(pLABReq);
		free(pMessageTag);
		return NTY_RESULT_ERROR;
	}
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

	ntyJsonLABItemRelease(pLABReq->lab.pNearbts);
	free(pLABReq);
	ntydbg(" ntyJsonLocationLabAction end --> \n");
}

int ntyJsonWeatherAction(ActionParam *pActionParam) {
	if (pActionParam == NULL)  {
		return NTY_RESULT_FAILED;
	}
	
	WeatherLocationReq *pWeatherLocationReq = (WeatherLocationReq*)malloc(sizeof(WeatherLocationReq));
	if (pWeatherLocationReq == NULL) {
		ntylog("ntyJsonWeatherAction --> WeatherLocationReq malloc failed \n");
		return NTY_RESULT_ERROR;
	}
	memset(pWeatherLocationReq, 0, sizeof(WeatherLocationReq));
	
	ntyJsonWeatherLocation(pActionParam->json, pWeatherLocationReq);
	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;
	
	U8 weatherbuf[500] = {0};
	sprintf(weatherbuf, "%s/position?accesstype=0&imei=%s&cdma=0&bts=%s&output=json&key=%s", 
		HTTP_GAODE_BASE_URL, pWeatherLocationReq->IMEI, pWeatherLocationReq->bts, HTTP_GAODE_KEY);
	ntylog(" weatherbuf --> %s\n", weatherbuf);
	int length = strlen(weatherbuf);

	MessageTag *pMessageTag = malloc(sizeof(MessageTag));
	if (pMessageTag == NULL) {
		free(pWeatherLocationReq);
		return NTY_RESULT_ERROR;
	}
	memset(pMessageTag, 0, sizeof(MessageTag));
	
	pMessageTag->Type = MSG_TYPE_WEATHER_API;
	pMessageTag->fromId = fromId;
	pMessageTag->toId = toId;
	pMessageTag->length = length;
	
#if ENABLE_DAVE_MSGQUEUE_MALLOC
	pMessageTag->Tag = malloc((length+1)*sizeof(U8));
	if (pMessageTag->Tag == NULL) {
		free(pMessageTag);
		free(pWeatherLocationReq);

		return NTY_RESULT_ERROR;
	}

	memset(pMessageTag->Tag, 0, length+1);
	memcpy(pMessageTag->Tag, weatherbuf, length);
#else
	memset(pMessageTag->Tag, 0, length+1);
	memcpy(pMessageTag->Tag, weatherbuf, length);
#endif
	
	pMessageTag->cb = ntyHttpQJKWeatherLocation;

	int ret = ntyDaveMqPushMessage(pMessageTag);
	//int ret = ntyHttpQJKWeatherLocation(pMessageTag);

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
int ntySaveCommonMsgData(C_DEVID sid, C_DEVID gid, char *jsonstring, int *msgId) {
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
	if (pActionParam == NULL) return ;

	const char *app_category = ntyJsonAppCategory(pActionParam->json);
	if (app_category != NULL) {
		if (strcmp(app_category, NATTY_USER_PROTOCOL_CATEGORY_AUTHORIZEPUSH) == 0) {
			ntyJsonAuthorizePushAction(pActionParam);
		} else if (strcmp(app_category, NATTY_USER_PROTOCOL_CATEGORY_AUTHORIZEREPLY) == 0) {
			ntyJsonAuthorizeReplyAction(pActionParam);
		} else if (strcmp(app_category, NATTY_USER_PROTOCOL_CATEGORY_EFENCE) == 0) {
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
		} else if (strcmp(app_category, NATTY_USER_PROTOCOL_CATEGORY_RUNTIME) == 0) {
				ntyJsonRunTimeAction(pActionParam);
		} else if (strcmp(app_category, NATTY_USER_PROTOCOL_CATEGORY_TURN) == 0) {
				ntyJsonTurnAction(pActionParam);
		} else if (strcmp(app_category, NATTY_USER_PROTOCOL_CATEGORY_SCHEDULE) == 0) {
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
			} else {
				ntylog("Can't find action with: %s\n", action);
			}
		} else if (strcmp(app_category, NATTY_USER_PROTOCOL_CATEGORY_TIMETABLES) == 0) {
			ntyJsonTimeTablesAction(pActionParam);
		} else if (strcmp(app_category, NATTY_USER_PROTOCOL_CATEGORY_CONTACTS) == 0) {
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
		} else if ( strcmp(app_category, NATTY_USER_PROTOCOL_CATEGORY_RESET) == 0 ){
			ntylog( "********** Reset common ***********\n" );
			ntyJsonResetAction( pActionParam );	
		}else if ( strcmp(app_category, NATTY_USER_PROTOCOL_CATEGORY_RESTORE) == 0 ){
			ntylog( "********** Restore common ***********\n" );
			ntyJsonRestoreAction( pActionParam );		
		}else {
			ntylog("Can't find app category with: %s\n", app_category);
		}
	}else {
		ntylog("Can't find app category, because app category is null\n");
	}

	const char *watch_category = ntyJsonWatchCategory(pActionParam->json);
	if (watch_category != NULL) {
		if (strcmp(watch_category, NATTY_USER_PROTOCOL_SOSREPORT) == 0) {
			ntyJsonSOSReportAction(pActionParam);
		} else if (strcmp(watch_category, NATTY_USER_PROTOCOL_LOCATIONREPORT) == 0) {
			ntyJsonLocationReportAction(pActionParam);
		} else if (strcmp(watch_category, NATTY_USER_PROTOCOL_STEPSREPORT) == 0) {
			ntyJsonStepsReportAction(pActionParam);
		} else if (strcmp(watch_category, NATTY_USER_PROTOCOL_HEARTREPORT) == 0) {
			ntyJsonHeartReportAction(pActionParam);
		} else if (strcmp(watch_category, NATTY_USER_PROTOCOL_EFENCEREPORT) == 0) {
			ntyJsonEfenceReportAction(pActionParam);
		} else if (strcmp(watch_category, NATTY_USER_PROTOCOL_WEARSTATUS) == 0) {
			ntyJsonWearStatusAction(pActionParam);
		} else if (strcmp(watch_category, NATTY_USER_PROTOCOL_FALLDOWNREPORT) == 0) {
			ntyJsonFallDownReportAction(pActionParam);
		} else {
			ntylog("Can't find watch category with: %s\n", watch_category);
		}
	} else {
		ntylog("Can't find watch category, because watch category is null\n");
	}
}

void ntyUserDataReqAction(ActionParam *pActionParam) {
	ntydbg("ntyCommonReqPacketHandleRequest --> fromId:%lld   \n", pActionParam->fromId);
	ntydbg("ntyCommonReqPacketHandleRequest --> json : %s\n", pActionParam->jsonstring);
	if (pActionParam == NULL) {
		return;
	}

	const char *device_category = ntyJsonAppCategory(pActionParam->json);
	if (device_category == NULL) {
		ntylog("Can't find device category, because device category is null\n");
		return;
	}

	if (strcmp(device_category, NATTY_USER_PROTOCOL_CATEGORY_ICCID) == 0) {
		const char *action = ntyJsonAction(pActionParam->json);
		if (action == NULL) {
			ntylog("Can't find action, because action is null\n");
			return;
		}
		if (strcmp(action, NATTY_USER_PROTOCOL_CATEGORY_SET) == 0) {
			ntyJsonICCIDSetAction(pActionParam);
		} else {
			ntylog("Can't find action with: %s\n", action);
		}
	}
}

void ntyJsonICCIDSetAction(ActionParam *pActionParam) {
	if (pActionParam == NULL) return ;

	ICCIDSet *pICCIDSet = (ICCIDSet*)malloc(sizeof(ICCIDSet));
	if(pICCIDSet == NULL) {
		ntylog("ntyJsonICCIDSetAction --> malloc failed ICCIDSet\n");
		return ;
	}
	memset(pICCIDSet, 0, sizeof(ICCIDSet));
	
	ntyJsonICCIDSet(pActionParam->json, pICCIDSet);
	C_DEVID fromId = pActionParam->fromId;
	
	int ret = ntyExecuteICCIDSetInsertHandle(fromId, pICCIDSet->ICCID,  pICCIDSet->phone_num);
	if (ret == -1) {
		ntylog(" ntyJsonICCIDSetAction --> DB Exception\n");
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DB_EXCEPTION);
	} else if (ret >= 0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);
	}
	free(pICCIDSet);
}


void ntyJsonAuthorizePushAction(ActionParam *pActionParam) {
	AuthorizePush *pAuthorizePush = (AuthorizePush*)malloc(sizeof(AuthorizePush));
	if (pAuthorizePush == NULL) {
		ntylog("ntyJsonAuthorizePushAction --> malloc AuthorizePush failed\n");
		return;
	}
	memset(pAuthorizePush, 0, sizeof(AuthorizePush));
	
	ntyJsonAuthorizePush(pActionParam->json, pAuthorizePush);

	C_DEVID did = pActionParam->toId;
	C_DEVID adminId = 0;
	int ret = ntyQueryAuthorizeAdminHandle(did, &adminId);
	if (ret == -1) {
		ntyJsonCommonResult(pActionParam->fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
	} else {
		if (adminId == 0) {
			ntyJsonCommonContextResult(pActionParam->fromId, NATTY_RESULT_CODE_ERR_NOT_FIND_ADMIN);
		} else {
			ntyJsonCommonContextResult(adminId, pActionParam->jsonstring);
		}
	}
	
	free(pAuthorizePush);
}

void ntyJsonAuthorizeReplyAction(ActionParam *pActionParam) {
	AuthorizeReply *pAuthorizeReply = (AuthorizeReply*)malloc(sizeof(AuthorizeReply));
	if (pAuthorizeReply == NULL) {
		ntylog("ntyJsonAuthorizeReplyAction --> malloc AuthorizeReply failed\n");
		return;
	}
	memset(pAuthorizeReply, 0, sizeof(AuthorizeReply));
	
	//ntyJsonAuthorizeReply(pActionParam->json, pAuthorizeReply);
	//ntyJsonBroadCastRecvResult(pActionParam->toId, pActionParam->fromId, (U8*)pActionParam->json, 0);

	ntyJsonCommonContextResult(pActionParam->toId, pActionParam->jsonstring);
	
	free(pAuthorizeReply);
}

void ntyJsonAddEfenceAction(ActionParam *pActionParam) {
	AddEfenceReq *pAddEfenceReq = (AddEfenceReq*)malloc(sizeof(AddEfenceReq));
	if (pAddEfenceReq == NULL) {
		ntylog("ntyJsonAddEfenceAction --> malloc AddEfenceReq failed\n");
		return ;
	}
	memset(pAddEfenceReq, 0, sizeof(AddEfenceReq));

	ntyJsonAddEfence(pActionParam->json, pAddEfenceReq);
	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;

	U8 points[256] = {0};
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
	U8 runtime[64] = {0};
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

	//电子围栏点数验证
	if (pAddEfenceReq->efence.size < 2 || pAddEfenceReq->efence.size > 6) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_EFENCE_POINTS);
		goto exit;
	}
	
	int id = 0;
	ret = ntyExecuteEfenceInsertHandle(fromId, toId, index, pAddEfenceReq->efence.size, points, runtime, &id);
	if (ret == -1) {
		ntylog(" ntyJsonEfenceAction --> DB Exception\n");
	} else if (ret == 0 && id == -2) { 
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DB_SAVE_REPEATE_DATA);
	} else if (ret >= 0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);

		AddEfenceAck *pAddEfenceAck = (AddEfenceAck*)malloc(sizeof(AddEfenceAck));
		if (pAddEfenceAck == NULL) {
			ntylog("ntyJsonAddEfenceAction --> malloc AddEfenceAck failed\n");
			free(pAddEfenceReq);
			return;
		}
		memset(pAddEfenceAck, 0, sizeof(AddEfenceAck));
		
		char ids[32] = {0};
		sprintf(ids, "%d", id);
		pAddEfenceAck->id = ids;

		char msgs[32] = {0};
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
	if (pActionParam == NULL) return ;
	DelEfenceReq *pDelEfenceReq = (DelEfenceReq*)malloc(sizeof(DelEfenceReq));
	if (pDelEfenceReq == NULL) {
		ntylog("ntyJsonDelEfenceAction --> malloc failed DelEfenceReq\n");
		return ;
	}
	memset(pDelEfenceReq, 0, sizeof(DelEfenceReq));
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
		if (pDelEfenceAck == NULL) {
			ntylog("ntyJsonDelEfenceAction --> malloc DelEfenceAck failed\n");
			goto exit;
		}
		memset(pDelEfenceAck, 0, sizeof(DelEfenceAck));
		
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
	if (pActionParam == NULL) return;
	
	ICCIDReq *pICCIDReq = (ICCIDReq*)malloc(sizeof(ICCIDReq));
	if (pICCIDReq == NULL) {
		ntylog("ntyJsonICCIDAction --> malloc failed ICCIDReq\n");
		
		return ;
	}
	memset(pICCIDReq, 0, sizeof(ICCIDReq));
	
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
	
	char iccid[16] = {0};
	memset(iccid, 0, 16);
	memcpy(iccid, pICCIDReq->ICCID+6, 13);
	ntylog(" ntyJsonICCIDAction --> iccid : %s\n", iccid);
	
	char phonenum[20] = {0};
	int ret = ntyExecuteICCIDSelectHandle(devId, iccid, phonenum);
	ntylog(" ntyJsonICCIDAction --> phonenum:%s\n", phonenum);

	ICCIDAck *pICCIDAck = (ICCIDAck*)malloc(sizeof(ICCIDAck));
	if (pICCIDAck == NULL) {
		ntylog(" ntyJsonICCIDAction --> malloc failed ICCIDAck");
		goto exit;
	}
	memset(pICCIDAck, 0, sizeof(ICCIDAck));
	pICCIDAck->IMEI = pICCIDReq->IMEI;
	if (ret == -1) {
		ntylog(" ntyJsonICCIDAction --> DB Exception\n");
		strcat(phonenum, "0");
		pICCIDAck->phone_num = phonenum;
		char *jsonstringICCID = ntyJsonWriteICCID(pICCIDAck);
		ntySendICCIDAckResult(devId, (U8*)jsonstringICCID, strlen(jsonstringICCID), 500);
		ntyJsonFree(jsonstringICCID);
		ret = 4;
	} else if (ret >= 0) {
		if (strlen(phonenum) == 0) {
			ntyJsonCommonResult(devId, NATTY_RESULT_CODE_ERR_ICCID_NOTPHONENUM);
			goto exit;
		}

		//ret = ntySaveCommonMsgData(pActionParam);
		
		pICCIDAck->phone_num = phonenum;
		char msgs[20] = {0};
		sprintf(msgs, "%d", pActionParam->index);
		pICCIDAck->msg = msgs;
		char *jsonstringICCID = ntyJsonWriteICCID(pICCIDAck);

		ntylog("ntyJsonICCIDAction --> %s\n", jsonstringICCID);
		ntySendICCIDAckResult(devId, (U8*)jsonstringICCID, strlen(jsonstringICCID), 200);
#if 0
		ret = ntySaveCommonMsgData(
			pActionParam->fromId,
			pActionParam->toId,
			jsonstringICCID,
			&pActionParam->index);
		if (ret >= 0) {
		} else {
			ntyJsonCommonResult(devId, NATTY_RESULT_CODE_ERR_DB_SAVE_OFFLINE);
		}
#endif
		ntyJsonFree(jsonstringICCID);
		
	}


	free(pICCIDAck);
exit:
	free(pICCIDReq);
}

void ntyJsonRunTimeAction(ActionParam *pActionParam) {
	if (pActionParam == NULL) return ;
	RunTimeReq *pRunTimeReq = (RunTimeReq*)malloc(sizeof(RunTimeReq));
	if (pRunTimeReq == NULL) {
		ntylog("ntyJsonRunTimeAction --> malloc failed RunTimeReq\n");
		return ;
	}
	memset(pRunTimeReq, 0, sizeof(RunTimeReq));
	
	ntyJsonRuntime(pActionParam->json, pRunTimeReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;
	
	int ret = 0;
	
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

	if ( pRunTimeReq->runtime.model != NULL ) {
		size_t len_model = strlen( pRunTimeReq->runtime.model );
		if ( len_model != 0 ) {
			int lenModel = atoi( pRunTimeReq->runtime.model );
			ret = ntyExecuteRuntimeModelUpdateHandle( fromId, toId, lenModel );
			if ( ret == -1 ) {
				ntylog(" ntyJsonRunTimeAction:model --> DB Exception\n");
				goto exit;
			}
		}
	}

	ret = ntySendRecodeJsonPacket(fromId, toId, pActionParam->jsonstring, pActionParam->jsonlen);
	if (ret < 0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
		goto exit;
	}else{
		//ret = ntySaveCommonMsgData(pActionParam);
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);

		char msgs[20] = {0};
		snprintf(msgs, 20, "%d", pActionParam->index);
		pRunTimeReq->msg = msgs;
		
		RunTimeAck *pRunTimeAck = (RunTimeAck*)malloc(sizeof(RunTimeAck));
		if (pRunTimeAck == NULL) {
			ntylog("ntyJsonRunTimeAction --> malloc failed RunTimeAck\n");
			goto exit;
		}
		memset(pRunTimeAck, 0, sizeof(RunTimeAck));
		
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

exit:
	free(pRunTimeReq);
}

void ntyJsonTurnAction(ActionParam *pActionParam) {
	if (pActionParam == NULL) return ;
	
	TurnReq *pTurnReq = (TurnReq*)malloc(sizeof(TurnReq));
	if (pTurnReq == NULL) {
		ntylog("ntyJsonTurnAction --> malloc failed TurnReq\n");
		return ;
	}
	memset(pTurnReq, 0, sizeof(TurnReq));
	
	ntyJsonTurn(pActionParam->json, pTurnReq);

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;
	
	U8 status = 0;
	int check_status = checkStringIsAllNumber(pTurnReq->turn.status);
	if (check_status == 1) {
		status = atoi(pTurnReq->turn.status);
	}

	char *turn_on = malloc(64);
	if (turn_on == NULL) {
		ntylog("ntyJsonTurnAction --> malloc failed turn_on \n");
		free(pTurnReq);
		return ;
	}
	memset(turn_on, 0, 64);
	int check_on = checkStringIsAllTimeChar(pTurnReq->turn.on.time);
	if (check_on != 1) {
		strcat(turn_on, "09:00:00");
	} else {
		strcat(turn_on, pTurnReq->turn.on.time);
	}
	char *turn_off = malloc(64);
	if (turn_off == NULL) {
		free(pTurnReq);
		free(turn_off);
		ntylog("ntyJsonTurnAction --> malloc failed turn_off \n");
		return ;
	}
	memset(turn_off, 0, 64);
	int check_off = checkStringIsAllTimeChar(pTurnReq->turn.off.time);
	if (check_off != 1) {
		strcat(turn_off, "18:00:00");
	} else {
		strcat(turn_off, pTurnReq->turn.off.time);
	}
	int ret = ntySendRecodeJsonPacket(fromId, toId, pActionParam->jsonstring, pActionParam->jsonlen);
	if (ret<0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
		goto exit;
	}

	ntylog(" ntyJsonTurnAction --> fromId:%lld, toId:%lld status:%d, turn_on:%s, turn_off:%s\n", fromId, toId, status, turn_on, turn_off);
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
		if (pTurnAck == NULL) {
			ntylog(" ntyJsonTurnAction --> malloc failed TurnAck\n");
			goto exit;
		}
		memset(pTurnAck, 0, sizeof(TurnAck));
		
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

exit:
	if (turn_on != NULL) {
		free(turn_on);
	}
	if (turn_off != NULL) {
		free(turn_off);
	}
	if (pTurnReq != NULL) {
		free(pTurnReq);
	}
	return ;
}

int ntyJsonResetAction( ActionParam *pActionParam ){
	int nRet = 0;
	if ( pActionParam == NULL) return NTY_RESULT_ERROR;

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;
	ntylog( " ********ntyJsonResetAction --> fromId:%lld, toId:%lld, json:%s\n", fromId, toId, pActionParam->jsonstring );
	nRet = ntyExecuteResetHandle( fromId, toId );
	if ( nRet == -1 ){
		ntylog(" ntyJsonResetAction --> DB Exception\n");
		goto EXIT;
	}

	//first send to deviceId,then send to fromId(appId,webId).
	nRet = ntySendRecodeJsonPacketVersionB( fromId, toId, (U8*)pActionParam->jsonstring, strlen(pActionParam->jsonstring) );
	if ( nRet < 0 ){ //send to deviceId failed.send back to fromId(appId,webId) with offline message.
		ntyJsonCommonResult( fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE );
		nRet = -2;
	}else{	//send back to fromId(appId,webId) with sucess message.		
		ntyJsonCommonResult( fromId, NATTY_RESULT_CODE_SUCCESS );
		nRet = 0;
	}
	
EXIT:
	return nRet;
}

int ntyJsonRestoreAction( ActionParam *pActionParam ){
	int nRet = 0;
	int ret = 0;
	if ( pActionParam == NULL) return NTY_RESULT_ERROR;

	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;  //deviceId
	ntylog( " ********ntyJsonRestoreAction --> fromId:%lld, toId:%lld, json:%s\n", fromId, toId, pActionParam->jsonstring );

	//delete b+tree, deviceId to the list of AppId
	void *heap = ntyBHeapInstance();
	NRecord *record = ntyBHeapSelect( heap, toId );
	if ( record != NULL ) {
		Client *aclient = (Client *)record->value;
		if( aclient != NULL ){
			ntyVectorIterator( aclient->friends, ntyAppIdToDeviceIdDeleteCb, &toId );				
			ntylog( "*********ntyJsonRestoreAction destroy vector of client->friends before\n" );
			aclient->friends = ntyVectorDestory( aclient->friends );
			ntylog("*********ntyJsonRestoreAction destroy vector of client->friends after\n" );	
#if 1 //recreator friends vector
			aclient->friends = ntyVectorCreator();
#endif 
		}
	}
	
	nRet = ntyExecuteRestoreHandle( fromId, toId );
	if ( nRet == -1 ){
		ntylog(" ntyJsonRestoreAction --> DB Exception\n");
	}else if ( nRet == 0 ){
		//first send to deviceId,then send to fromId(appId,webId)
		ret = ntySendRecodeJsonPacketVersionB( fromId, toId, (U8*)pActionParam->jsonstring, strlen(pActionParam->jsonstring) );
		if ( ret < 0 ){
			ntyJsonCommonResult( fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE );
		}else{
			ntyJsonCommonResult( fromId, NATTY_RESULT_CODE_SUCCESS );
		}
	}else{} 

	return nRet;
}

int ntyAppIdToDeviceIdDeleteCb( void *self, void *arg ){
	C_DEVID appId = 0;
	memcpy( &appId, self, sizeof(C_DEVID) );
	
	void *heap = ntyBHeapInstance();
	NRecord *record = ntyBHeapSelect( heap, appId );
	if ( record != NULL ) {
		Client *aclient = (Client *)record->value;
		if( aclient != NULL ){
			//arg:deviceId
			ntylog( "***********ntyAppIdToDeviceIdDelete before appId:%lld,deviceId:%lld\n",appId, atoll(arg) );
			ntyVectorDelete( aclient->friends, arg );
		}
	}
	return 0;
}


void ntyJsonAddScheduleAction(ActionParam *pActionParam) {
	if (pActionParam == NULL) return ;
	
	AddScheduleReq *pAddScheduleReq = (AddScheduleReq*)malloc(sizeof(AddScheduleReq));
	if (pAddScheduleReq == NULL) {
		ntylog("ntyJsonAddScheduleAction --> malloc failed AddScheduleReq\n");
		return ;
	}
	memset(pAddScheduleReq, 0, sizeof(AddScheduleReq));

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
		if (pDeviceAddScheduleAck == NULL) {
			free(pAddScheduleReq);
			ntylog("ntyJsonAddScheduleAction --> malloc failed DeviceAddScheduleAck\n");
			return ;
		}
		memset(pDeviceAddScheduleAck, 0, sizeof(DeviceAddScheduleAck));
		pDeviceAddScheduleAck = (DeviceAddScheduleAck*)pAddScheduleReq;
		pDeviceAddScheduleAck->id = ids;
		char *jsondeviceresult = ntyJsonWriteDeviceAddSchedule(pDeviceAddScheduleAck);
		ret = ntySendRecodeJsonPacket(fromId, toId, jsondeviceresult, strlen(jsondeviceresult));
		if (ret<0) {
			ntyExecuteScheduleDeleteHandle(fromId,toId,scheduleId);
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
		} else {
			AddScheduleAck *pAddScheduleAck = (AddScheduleAck*)malloc(sizeof(AddScheduleAck));
			if (pAddScheduleAck == NULL) {
				ntylog("ntyJsonAddScheduleAction --> malloc failed AddScheduleAck\n");
				free(pDeviceAddScheduleAck);
				free(pAddScheduleReq);
				return ;
			}
			memset(pAddScheduleAck, 0, sizeof(AddScheduleAck));
			
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
	if (pActionParam == NULL) return ;
	
	DelScheduleReq *pDelScheduleReq = (DelScheduleReq*)malloc(sizeof(DelScheduleReq));
	if (pDelScheduleReq == NULL) {
		ntylog("ntyJsonDelScheduleAction --> malloc failed DelScheduleReq\n");
		return ;
	}
	memset(pDelScheduleReq, 0, sizeof(DelScheduleReq));
	
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
		if (pDelScheduleAck == NULL) {
			ntylog("ntyJsonDelScheduleAction --> malloc failed DelScheduleAck\n");
			goto exit;
		}
		memset(pDelScheduleAck, 0, sizeof(DelScheduleAck));

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
	if (pActionParam == NULL) return ;
	
	UpdateScheduleReq *pUpdateScheduleReq = (UpdateScheduleReq*)malloc(sizeof(UpdateScheduleReq));
	if (pUpdateScheduleReq == NULL) {
		ntylog("ntyJsonUpdateScheduleAction --> malloc failed UpdateScheduleReq\n");
		return ;
	}
	memset(pUpdateScheduleReq, 0, sizeof(UpdateScheduleReq));

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
		if (pUpdateScheduleAck == NULL) {
			ntylog("ntyJsonUpdateScheduleAction --> malloc failed UpdateScheduleAck\n");
			goto exit; 
		}
		memset(pUpdateScheduleAck, 0, sizeof(UpdateScheduleAck));
		
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


void ntyJsonTimeTablesAction(ActionParam *pActionParam) {
	if (pActionParam == NULL) return ;
	
	TimeTablesReq *pTimeTablesReq = (TimeTablesReq*)malloc(sizeof(TimeTablesReq));
	if(pTimeTablesReq == NULL) {
		ntylog("ntyJsonTimeTablesAction --> malloc failed TimeTablesReq\n");
		return ;
	}
	memset(pTimeTablesReq, 0, sizeof(TimeTablesReq));
	
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
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DB_EXCEPTION);
			goto exit;
		}
	}
	
	if (ret == -1) {
		ntylog(" ntyJsonTimeTables --> DB Exception\n");
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DB_EXCEPTION);
	} else if (ret == 0 && result == -2) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DB_NOEXIST);
	} else if (ret >= 0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);
		
		TimeTablesAck *pTimeTablesAck = malloc(sizeof(TimeTablesAck));
		if (pTimeTablesAck == NULL) {
			ntylog("ntyJsonTimeTablesAction --> malloc failed TimeTablesAck\n");
			free(pTimeTablesReq);
			return ;
		}
		memset(pTimeTablesAck, 0, sizeof(TimeTablesAck));
		
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
	if (pActionParam == NULL) return ;
	
	AddContactsReq *pAddContactsReq = (AddContactsReq*)malloc(sizeof(AddContactsReq));
	if (pAddContactsReq == NULL) {
		ntylog("ntyJsonAddContactsAction --> malloc failed AddContactsReq\n");
		return ;
	}
	memset(pAddContactsReq, 0, sizeof(AddContactsReq));

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
		if (pDeviceAddContactsAck == NULL) {
			ntylog("ntyJsonAddContactsAction --> malloc failed DeviceAddContactsAck\n");
			goto exit;
		}
		memset(pDeviceAddContactsAck, 0, sizeof(DeviceAddContactsAck));

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
			if (pAddContactsAck == NULL) {
				ntylog("ntyJsonAddContactsAction --> malloc failed AddContactsAck\n");

				free(jsondeviceresult);
				free(pDeviceAddContactsAck);

				goto exit;
			}
			memset(pAddContactsAck, 0, sizeof(AddContactsAck));
			
			pAddContactsAck->results = *(AddContactsResults*)pAddContactsReq;
			char *jsonresult = ntyJsonWriteAddContacts(pAddContactsAck);
			ret = ntySaveCommonMsgData(
				pActionParam->fromId,
				pActionParam->toId, 
				jsonresult,
				&pActionParam->index);
			ntylog(" ntyJsonAddContactsAction--->ret %d %s\n", ret, jsonresult);
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
	if (pActionParam == NULL) return ;
	
	UpdateContactsReq *pUpdateContactsReq = (UpdateContactsReq*)malloc(sizeof(UpdateContactsReq));
	if (pUpdateContactsReq == NULL){ 
		ntylog("ntyJsonUpdateContactsAction --> malloc failed ActionParam\n");
		return ;
	}
	memset(pUpdateContactsReq, 0, sizeof(UpdateContactsReq));
	
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
		
		UpdateContactsAck *pUpdateContactsAck = (UpdateContactsAck*)malloc(sizeof(UpdateContactsAck));
		if (pUpdateContactsAck == NULL) {
			ntylog("ntyJsonUpdateContactsAction --> malloc failed UpdateContactsAck\n");
			goto exit;
		}
		memset(pUpdateContactsAck, 0, sizeof(UpdateContactsAck));

		
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
	if (pDelContactsReq == NULL){ 
		ntylog("ntyJsonDelContactsAction --> malloc failed DelContactsReq\n");
		return ;
	}

	memset(pDelContactsReq, 0, sizeof(DelContactsReq));

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
	} else if (ret >= 0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);
		char msgs[20] = {0};
		sprintf(msgs, "%d", pActionParam->index);
		pDelContactsReq->msg = msgs;
			
		DelContactsAck *pDelContactsAck = malloc(sizeof(DelContactsAck));
		if (pDelContactsAck == NULL) { 
			goto exit;
		}
		memset(pDelContactsAck, 0, sizeof(DelContactsAck));
		
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

void ntyJsonSOSReportAction(ActionParam *pActionParam) {
	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;

	U32 msg = 0;
	int ret = ntyExecuteCommonMsgInsertHandle(fromId, toId, pActionParam->jsonstring, &msg);
	if (ret == -1) {
		ntylog(" ntyJsonSOSReportAction --> DB Exception\n");
	} else if (ret >= 0) {
		ret = ntyJsonBroadCastRecvResult(fromId, toId, pActionParam->jsonstring, msg);
		if (ret >= 0) {
			ntyJsonCommonResult(toId, NATTY_RESULT_CODE_SUCCESS);
		} else {
			ntyJsonCommonResult(toId, NATTY_RESULT_CODE_ERR_BROADCAST);
		}
	}
}

void ntyJsonLocationReportAction(ActionParam *pActionParam) {
	if ( pActionParam == NULL ) return;
	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;


	ntylog("fromId:%lld, toId:%lld\n", fromId, toId);
	
	LocationReport *pLocationReport = (LocationReport *)malloc( sizeof(LocationReport) );
	if ( pLocationReport == NULL ) {
		ntylog("ntyJsonLocationReportAction --> malloc failed LocationReport\n");
		return;
	}
	memset( pLocationReport, 0, sizeof(LocationReport) );
	ntyJsonLocationReport( pActionParam->json, pLocationReport );	
	
	U8 httpBuf[1024] = {0};
	sprintf( httpBuf, "%s/v3/geocode/regeo?key=%s&location=%s", 
		HTTP_GAODE_RESTAPI_URL, HTTP_GAODE_KEY, pLocationReport->results.locationReport.location );
	ntylog( "********ntyJsonLocationReportAction http request:%s\n", httpBuf );
	int length = strlen( httpBuf );

#if 1 //Push to MessageQueue
	MessageTag *pMessageTag = (MessageTag *)malloc(sizeof(MessageTag));
	if ( pMessageTag == NULL ) {
		ntylog("ntyJsonLocationReportAction --> malloc failed MessageTag\n");
		free( pLocationReport );	
		return;
	}
	memset( pMessageTag, 0, sizeof(MessageTag) );	
	
	if (strcmp(pLocationReport->results.locationReport.type, NATTY_USER_PROTOCOL_WIFI) == 0) {
		pMessageTag->Type = MSG_TYPE_LOCATION_WIFI_API;
	} else if (strcmp(pLocationReport->results.locationReport.type, NATTY_USER_PROTOCOL_GPS) == 0) {
		pMessageTag->Type = MSG_TYPE_LOCATION_GPS_API;
	} else if (strcmp(pLocationReport->results.locationReport.type, NATTY_USER_PROTOCOL_LAB) == 0) {
		pMessageTag->Type = MSG_TYPE_LOCATION_LAB_API;
	}else{}	
	
	pMessageTag->fromId = fromId;
	pMessageTag->toId = toId;
	pMessageTag->length = length;

#if ENABLE_DAVE_MSGQUEUE_MALLOC
	pMessageTag->Tag = malloc((length+1)*sizeof(U8));
	if ( pMessageTag->Tag == NULL ) {
		ntylog("ntyJsonLocationReportAction --> malloc failed pMessageTag->Tag\n");
		free( pMessageTag );
		free( pLocationReport );	
		return;
	}
	memset( pMessageTag->Tag, 0, length+1 );
	memcpy( pMessageTag->Tag, httpBuf, length );
#else
	memset( pMessageTag->Tag, 0, length+1 );
	memcpy( pMessageTag->Tag, httpBuf, length );
#endif

	pMessageTag->cb = ntyHttpQJKLocationGetAddress;

	int ret = ntyDaveMqPushMessage( pMessageTag );

#endif
	//int ret = ntyHttpQJKLocation(pMessageTag);
	if ( ret < 0 ) {
		ntylog(" ntyDaveMqPushMessage error\n");
	} else{
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);
	}
	
	if ( pLocationReport != NULL ){
		free( pLocationReport );	
		pLocationReport = NULL;
	}

#if 0
	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;

	LocationReport *pLocationReport = malloc(sizeof(LocationReport));
	if (pLocationReport == NULL) {
		ntylog("ntyJsonLocationReportAction --> malloc failed LocationReport\n");
		return;
	}
	memset(pLocationReport, 0, sizeof(LocationReport));
	ntyJsonLocationReport(pActionParam->json, pLocationReport);

	U8 type = 1;
	if (strcmp(pLocationReport->results.locationReport.type, NATTY_USER_PROTOCOL_WIFI) == 0) {
		type = 1;
	} else if (strcmp(pLocationReport->results.locationReport.type, NATTY_USER_PROTOCOL_GPS) == 0) {
		type = 2;
	} else if (strcmp(pLocationReport->results.locationReport.type, NATTY_USER_PROTOCOL_LAB) == 0) {
		type = 3;
	}

	char desc[32] = {0};
	strcat(desc, "Location not detected");
	const char *lnglat = pLocationReport->results.locationReport.location;
	const char *detatils = pLocationReport->results.locationReport.radius;
	
	U32 msg = 0;
	int ret = ntyExecuteLocationReportInsertHandle(toId, (U8)type, desc, lnglat, detatils, &msg);
	if (ret == -1) {
		ntylog(" ntyJsonLocationReportAction --> DB Exception\n");
	} else if (ret >= 0) {
		ret = ntyJsonBroadCastRecvResult(fromId, toId, pActionParam->jsonstring, msg);
		if (ret >= 0) {
			ntyJsonCommonResult(toId, NATTY_RESULT_CODE_SUCCESS);
		} else {
			ntyJsonCommonResult(toId, NATTY_RESULT_CODE_ERR_BROADCAST);
		}
	}
#endif
}


void ntyJsonStepsReportAction(ActionParam *pActionParam) {
	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;

	StepsReport *pStepsReport = malloc(sizeof(StepsReport));
	if (pStepsReport == NULL) {
		ntylog("ntyJsonStepsReportAction --> malloc failed StepsReport\n");
		return;
	}
	memset(pStepsReport, 0, sizeof(StepsReport));
	ntyJsonStepsReport(pActionParam->json, pStepsReport);
	int stepReport = 0;
	int stepReport_check = checkStringIsAllNumber(pStepsReport->results.stepsReport);
	if (stepReport_check == 1) {
		stepReport = atoi(pStepsReport->results.stepsReport);
	}

	U32 msg = 0;
	int ret = ntyExecuteStepsReportInsertHandle(toId, stepReport, &msg);
	if (ret == -1) {
		ntylog(" ntyJsonStepsReportAction --> DB Exception\n");
	} else if (ret >= 0) {
		ret = ntyJsonBroadCastRecvResult(fromId, toId, pActionParam->jsonstring, msg);
		if (ret >= 0) {
			ntyJsonCommonResult(toId, NATTY_RESULT_CODE_SUCCESS);
		} else {
			ntyJsonCommonResult(toId, NATTY_RESULT_CODE_ERR_BROADCAST);
		}
	}
}

void ntyJsonHeartReportAction(ActionParam *pActionParam) {
	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;

	HeartReport *pHeartReport = malloc(sizeof(HeartReport));
	if (pHeartReport == NULL) {
		ntylog("ntyJsonHeartReportAction --> malloc failed HeartReport\n");
		return;
	}
	memset(pHeartReport, 0, sizeof(HeartReport));
	ntyJsonHeartReport(pActionParam->json, pHeartReport);

	int heartReport = 0;
	int heartReport_check = checkStringIsAllNumber(pHeartReport->results.heartReport);
	if (heartReport_check == 1) {
		heartReport = atoi(pHeartReport->results.heartReport);
	}

	U32 msg = 0;
	int ret = ntyExecuteHeartReportInsertHandle(toId, heartReport, &msg);
	if (ret == -1) {
		ntylog(" ntyJsonHeartReportAction --> DB Exception\n");
	} else if (ret >= 0) {
		ret = ntyJsonBroadCastRecvResult(fromId, toId, pActionParam->jsonstring, msg);
		if (ret >= 0) {
			ntyJsonCommonResult(toId, NATTY_RESULT_CODE_SUCCESS);
		} else {
			ntyJsonCommonResult(toId, NATTY_RESULT_CODE_ERR_BROADCAST);
		}
	}
}


void ntyJsonEfenceReportAction(ActionParam *pActionParam) {
	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;

	U32 msg = 0;
	int ret = ntyExecuteCommonMsgInsertHandle(fromId, toId, pActionParam->jsonstring, &msg);
	if (ret == -1) {
		ntylog(" ntyJsonSOSReportAction --> DB Exception\n");
		ret = 4;
	} else if (ret >= 0) {
		ret = ntyJsonBroadCastRecvResult(fromId, toId, pActionParam->jsonstring, msg);
		if (ret >= 0) {
			ntyJsonCommonResult(toId, NATTY_RESULT_CODE_SUCCESS);
		} else {
			ntyJsonCommonResult(toId, NATTY_RESULT_CODE_ERR_BROADCAST);
		}
	}
}


/**
 *
 * http://app.quanjiakan.com/familycare/api?m=health&a=falldown&deviceid=352315052834187&lat=23.13245960031104&lng=113.24134987677577&type=1 
 *
 */
int ntyJsonFallDownReportAction(ActionParam *pActionParam) {	
	ntylog(" ntyJsonFallDownReportAction begin --> \n");
	if (pActionParam == NULL) {
		return NTY_RESULT_FAILED;
	}
	
	Falldown *pFalldown = (Falldown*)malloc(sizeof(Falldown));
	if (pFalldown== NULL) {
		return NTY_RESULT_ERROR;
	}
	memset(pFalldown, 0, sizeof(Falldown));
	
	ntyJsonFalldown(pActionParam->json, pFalldown);
	
	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;

	double longitude = 0.0;
	double latitude = 0.0;
	int fdtype = 1;
	int check_lng = checkStringIsDouble(pFalldown->results.falldownReport.longitude);
	if (check_lng == 1) {
		longitude = atof(pFalldown->results.falldownReport.longitude);
	}
	int check_lat = checkStringIsDouble(pFalldown->results.falldownReport.latitude);
	if (check_lat == 1) {
		latitude = atof(pFalldown->results.falldownReport.latitude);
	}
	int check_type = checkStringIsAllNumber(pFalldown->results.falldownReport.type);
	if (check_type == 1) {
		fdtype = atoi(pFalldown->results.falldownReport.type);
	}
	
	
	ntylog("-----------ntyJsonFallDownReportAction falldownbuf----------- \n");
	U8 falldownbuf[PACKET_BUFFER_SIZE] = {0};
	sprintf(falldownbuf, "%s/familycare/api?m=health&a=falldown&deviceid=%s&lat=%s&lng=%s&type=%s", 
		HTTP_QJK_BASE_URL, 
		pFalldown->results.IMEI, 
		pFalldown->results.falldownReport.latitude, 
		pFalldown->results.falldownReport.longitude,
		pFalldown->results.falldownReport.type);
	ntylog(" falldownbuf --> %s\n", falldownbuf);
	int length = strlen(falldownbuf);

	MessageTag *pMessageTag = malloc(sizeof(MessageTag));
	if (pMessageTag == NULL) {
		free(pFalldown);
		return NTY_RESULT_ERROR;
	}
	memset(pMessageTag, 0, sizeof(MessageTag));
	
	pMessageTag->Type = MSG_TYPE_FALLDOWN_API;
	pMessageTag->fromId = fromId;
	pMessageTag->toId = toId;

	pMessageTag->length = length;

#if ENABLE_DAVE_MSGQUEUE_MALLOC
	pMessageTag->Tag = malloc((length+1)*sizeof(U8));
	if (pMessageTag->Tag == NULL) {
		free(pFalldown);
		free(pMessageTag);
		return NTY_RESULT_ERROR;
	}
	memset(pMessageTag->Tag, 0, length+1);
	memcpy(pMessageTag->Tag, falldownbuf, length);
#else
	memset(pMessageTag->Tag, 0, length+1);
	memcpy(pMessageTag->Tag, falldownbuf, length);
#endif
	pMessageTag->cb = ntyHttpQJKFalldown;
	
	
	pMessageTag->u8LocationType = fdtype;

	int ret = ntyDaveMqPushMessage(pMessageTag);
	free(pFalldown);


	ntylog("-----------ntyJsonFallDownReportAction natty ----------- \n");
	
	int id = 0;
	ret = ntyExecuteFalldownInsertHandle(fromId, toId, longitude, latitude, fdtype,	&id);

	//广播跌倒数据到相应的用户
	U32 msg = 0;
	int ret = ntyExecuteCommonMsgInsertHandle(fromId, toId, pActionParam->jsonstring, &msg);
	if (ret == -1) {
		ntylog(" ntyJsonFallDownReportAction --> DB Exception\n");
	} else if (ret >= 0) {
		ret = ntyJsonBroadCastRecvResult(fromId, toId, pActionParam->jsonstring, msg);
		if (ret >= 0) {
			ntyJsonCommonResult(toId, NATTY_RESULT_CODE_SUCCESS);
		} else {
			ntyJsonCommonResult(toId, NATTY_RESULT_CODE_ERR_BROADCAST);
		}
	}
	
	ntydbg(" ntyJsonFallDownReportAction end --> \n");
	return NTY_RESULT_SUCCESS;
}

void ntyJsonWearStatusAction(ActionParam *pActionParam) {
	C_DEVID fromId = pActionParam->fromId;
	C_DEVID toId = pActionParam->toId;

	U32 msg = 0;
	//int ret = ntyExecuteCommonMsgInsertHandle(fromId, toId, pActionParam->jsonstring, &msg);
	//if (ret == -1) {
	//	ntylog(" ntyJsonSOSReportAction --> DB Exception\n");
	//	ret = 4;
	//} else if (ret >= 0) {
		int ret = ntyJsonBroadCastRecvResult(fromId, toId, pActionParam->jsonstring, msg);
		if (ret >= 0) {
			ntyJsonCommonResult(toId, NATTY_RESULT_CODE_SUCCESS);
		} else {
			ntyJsonCommonResult(toId, NATTY_RESULT_CODE_ERR_BROADCAST);
		}
	//}
}

//添加联系人消息发送到管理员
void ntyBindAgreeAction(char *imei, C_DEVID fromId, C_DEVID proposerId, C_DEVID toId, char *phonenum, U32 msgId) {
#if 0
	char bindAgreeAck[64] = {0};
	memcpy(bindAgreeAck, NATTY_USER_PROTOCOL_BINDAGREE, strlen(NATTY_USER_PROTOCOL_BINDAGREE));
	size_t len_bindAgreeAck = sizeof(BindAgreeAck);
	BindAgreeAck *pBindAgreeAck = malloc(len_bindAgreeAck);
	if (pBindAgreeAck == NULL) {
		ntylog("ntyBindAgreeAction --> malloc failed BindAgreeAck\n");
		
		return;
	}
	memset(pBindAgreeAck, 0, len_bindAgreeAck);
	
	char msgIds[64] = {0};
	sprintf(msgIds, "%d", msgId);
	pBindAgreeAck->IMEI = imei;
	pBindAgreeAck->category = bindAgreeAck;
	pBindAgreeAck->adminId = adminIds;
	pBindAgreeAck->msgId = msgIds;

	char *jsonagree = ntyJsonWriteBindAgree(pBindAgreeAck);
	int ret = ntySendRecodeJsonPacket(fromId, toId, jsonagree, (int)strlen(jsonagree));
	if (ret < 0) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
	}
#else

	int contactsTempId = 0;
	char *pname = NULL;
	char *pimage = NULL;
	ntylog("--execute ntyBindAgreeAction action---1----------------\n");
	int ret = ntyQueryPhonebookBindAgreeSelectHandle(toId, proposerId, phonenum, &contactsTempId, pname, pimage);
	ntylog("--execute ntyBindAgreeAction action---2----------------\n");
	if (ret == -1) {
		ntylog(" ntyJsonDelContactsAction --> DB Exception\n");
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
	} else if (ret >= 0) {
		AddContactsAck *pAddContactsAck = malloc(sizeof(AddContactsAck));
		if (pAddContactsAck == NULL) {
			ntylog("ntyBindAgreeAction --> malloc AddContactsAck failed\n");
			return;
		}
		memset(pAddContactsAck, 0, sizeof(AddContactsAck));

		char contactsId[16] = {0};
		sprintf(contactsId, "%d", contactsTempId);
		char add[16] = {0};
		char category[16] = {0};
		strcat(add, "Add");
		strcat(category, "Contacts");
		pAddContactsAck->results.id = contactsId;
		pAddContactsAck->results.IMEI = imei;
		pAddContactsAck->results.category = category;
		pAddContactsAck->results.action = add;
		pAddContactsAck->results.contacts.image = pimage;
		pAddContactsAck->results.contacts.name = pname;
		pAddContactsAck->results.contacts.telphone = phonenum;

		char *jsonagree = ntyJsonWriteAddContacts(pAddContactsAck);
		int ret = ntySendRecodeJsonPacket(fromId, toId, jsonagree, (int)strlen(jsonagree));
		if (ret < 0) {
			ntylog(" ntyBindAgreeAction --> SendCommonReq Exception\n");
			//ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
		}
		ntyJsonFree(jsonagree);
		free(pAddContactsAck);
	}

	if (pname != NULL) {
		free(pname);
	}
	if (pimage != NULL) {
		free(pimage);
	}
	
#endif
}

void ntyJsonOfflineMsgReqAction(ActionParam *pActionParam) {
	DelContactsReq *pDelContactsReq = (DelContactsReq*)malloc(sizeof(DelContactsReq));
	if (pDelContactsReq == NULL) {
		ntylog("ntyJsonOfflineMsgReqAction --> malloc failed DelContactsReq\n");
		return ;
	}
	memset(pDelContactsReq, 0, sizeof(DelContactsReq));

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
			if (pDelContactsAck == NULL) {
				free(pDelContactsReq);
				ntylog("ntyJsonOfflineMsgReqAction --> malloc DelContactsAck failed\n");
				
				return ;
			}
			memset(pDelContactsAck, 0, sizeof(DelContactsAck));
			
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
	if (tag == NULL) {
		ntylog("ntyVoiceDataReqAction --> malloc VALUE_TYPE failed\n");
		return NTY_RESULT_FAILED;
	}
	
	memset(tag, 0, sizeof(VALUE_TYPE));

	int sLen = strlen(filename);
	tag->Tag = malloc(sLen+1);
	if (tag->Tag == NULL) {
		ntylog("ntyVoiceDataReqAction --> malloc VALUE_TYPE failed\n");
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

		//delete msgId
		return ret;
	}
	int filename_len = (int)strlen(filename);
	if (filename_len==0) {
		ntydbg("filename length is %d\n", filename_len);
		return -1;
	}

	ntylog(" ntyVoiceReqAction --> senderId:%lld, gId:%lld, filename:%s\n", senderId, gId, filename);
	U8 *pData = (U8 *)malloc(NTY_VOICEREQ_COUNT_LENGTH*NTY_VOICEREQ_PACKET_LENGTH);
	if (pData == NULL) {
		ntylog("ntyVoiceReqAction --> malloc failed\n");
		return NTY_RESULT_ERROR;
	}
	memset(pData, 0, NTY_VOICEREQ_COUNT_LENGTH*NTY_VOICEREQ_PACKET_LENGTH);
	int size = ntyReadDat(filename, pData, NTY_VOICEREQ_COUNT_LENGTH*NTY_VOICEREQ_PACKET_LENGTH);
	
	ntylog(" ntyVoiceReqAction --> size:%d\n", size);
	if (size == -1) {
		free(pData);
		
		int ret = ntyExecuteVoiceOfflineMsgDeleteHandle(msgId, fromId);
		ntylog("ntyVoiceReqAction --> ntyExecuteVoiceOfflineMsgDeleteHandle ret : %d\n", ret);
		
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
	if (tag == NULL) {
		ntylog("ntyVoiceAckAction --> malloc failed\n");
		return NTY_RESULT_ERROR;
	}
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
		return ret;
	}

	ntylog(" ntyBindReqAction after ntyQueryDevAppGroupCheckSelectHandle\n");
	BindReq *pBindReq = malloc(sizeof(BindReq));
	if (pBindReq == NULL) { 
		ntylog("ntyBindReqAction --> malloc BindReq failed\n");
		return NTY_RESULT_ERROR;
	}
	memset(pBindReq, 0, sizeof(BindReq));
	ntyJsonBind(pActionParam->json, pBindReq);

	ntylog(" ntyBindReqAction before ntyQueryAdminSelectHandle\n");
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
		U32 msgId = 0;
		U8 phnum[32] = {0};

		ntylog("WatchName:%s, UserName:%s\n", pBindReq->bind.watchName, pBindReq->bind.userName);
	
		if (admin == 0 || admin == fromId) { //first bind device
			//将管理员添加到新创建的群组
			ret = ntyQueryAdminGroupInsertHandle(devId, name, proposer, call, wimage, uimage, phnum, &msgId);
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

		//int msgId = 0;
		//绑定确认添加
		ntyQueryBindConfirmInsertHandle(admin, devId, name, wimage, proposer, call, uimage, &msgId);

		ntydbg("-----------------------query phonebook select---------------------------------\n");
		char phonenum[30] = {0};
		ntyQueryPhoneBookSelectHandle(devId, proposer, phonenum);

		ntydbg("-----------------------encode JSON write here---------------------------------\n");
		//Encode JSON write here
		BindConfirmPush *pBindConfirmPush = malloc(sizeof(BindConfirmPush));
		if (pBindConfirmPush == NULL) {
			ntylog("ntyBindReqAction --> malloc BindConfirmPush failed\n");
			goto exit;
		}
		memset(pBindConfirmPush, 0, sizeof(BindConfirmPush));
		
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

exit:
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


int ntyReadDeviceOfflineCommonMsgIter(void *self, void *arg) {
	CommonOfflineMsg *pCommonOfflineMsg = (CommonOfflineMsg*)self;
	if (pCommonOfflineMsg == NULL) {
		return NTY_RESULT_ERROR;
	}
	
	C_DEVID fromId = *(C_DEVID*)arg;
	char *json = pCommonOfflineMsg->details;
	if (json == NULL) {
		return -1;
	}
#if 0	
	int ret = ntySendCommonBroadCastItem(
		pCommonOfflineMsg->senderId, 
		fromId, 
		pCommonOfflineMsg->groupId,
		json, 
		strlen(json), 
		pCommonOfflineMsg->msgId);
	if (ret<0) {
		ntyJsonCommonResult(pCommonOfflineMsg->senderId, NATTY_RESULT_CODE_ERR_DB_SEND_OFFLINE);
	}
#else

	int ret = ntySendCommonReq(pCommonOfflineMsg->senderId, pCommonOfflineMsg->groupId, json, strlen(json), pCommonOfflineMsg->msgId);
	if (ret < 0) {
		ntylog("ntyReadDeviceOfflineCommonMsgIter");
	}
	
#endif
	free(json);
	
	return 0;
}



int ntyReadDeviceOfflineCommonMsgAction(C_DEVID devId) {
	NVector *container = ntyVectorCreator();
	if (NULL == container) return NTY_RESULT_FAILED;

	ntylog("ntyReadDeviceOfflineCommonMsgAction --> %lld\n", devId);
	int ret = ntyQueryCommonOfflineMsgSelectHandle(devId, container);
	if (ret == NTY_RESULT_SUCCESS) {
		ntyVectorIterator(container, ntyReadDeviceOfflineCommonMsgIter, &devId);
	} 
	ntylog("ntyReadDeviceOfflineCommonMsgAction --> ret : %d\n", ret);

	if (ret != NTY_RESULT_BUSY) {
		if (container->num == 0) { //count == 0
			ret = NTY_RESULT_NOEXIST;
		} else {
			ret = NTY_RESULT_SUCCESS;
		}
	} 
	
	ntyVectorDestory(container);

	return ret;
}


int ntyReadOfflineCommonMsgIter(void *self, void *arg) {
	CommonOfflineMsg *pCommonOfflineMsg = (CommonOfflineMsg*)self;
	if (pCommonOfflineMsg == NULL) return NTY_RESULT_ERROR;
	
	C_DEVID fromId = *(C_DEVID*)arg;
	char *json = pCommonOfflineMsg->details;
	if (json == NULL) {
		return -1;
	}
	
	int ret = ntySendCommonBroadCastItem(
		pCommonOfflineMsg->senderId, 
		fromId, 
		pCommonOfflineMsg->groupId,
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
		ntyVectorIterator(container, ntyReadOfflineCommonMsgIter, &devId);
	} 
	ntylog("ntyReadOfflineCommonMsgAction --> ret : %d\n", ret);

	if (ret != NTY_RESULT_BUSY) {
		if (container->num == 0) { //count == 0
			ret = NTY_RESULT_NOEXIST;
		} else {
			ret = NTY_RESULT_SUCCESS;
		}
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

	ntySendVoiceBroadCastItem(senderId, toId, gId, NULL, 0, msgId);
	return 0;
}


int ntyReadOfflineVoiceMsgAction(C_DEVID devId) {
	NVector *container = ntyVectorCreator();
	if (NULL == container) return NTY_RESULT_FAILED;

	int ret = ntyQueryVoiceOfflineMsgSelectHandle(devId, container);
	if (ret == NTY_RESULT_SUCCESS) {
		ntyVectorIterator(container, ntyReadOfflineVoiceMsgIter, &devId);
	} 

	ntylog("ntyReadOfflineVoiceMsgAction --> ret : %d\n", ret);
	
	if (ret != NTY_RESULT_BUSY) {
		if (container->num == 0) { //count == 0
			ret = NTY_RESULT_NOEXIST;
		} else {
			ret = NTY_RESULT_SUCCESS;
		}
	} 
	
	ntyVectorDestory(container);

	return ret;
}


/**
 *	当申请者发送请求是,而管理员不在线的情况下,管理员接收离线申请数据.
 *	When the request is sent and the administrator is not online, the administrator receives the offline application data.
 */
int ntyReadOfflineBindMsgToAdminIter(void *self, void *arg) {
	BindOfflineMsgToAdmin *pMsgToAdmin = (BindOfflineMsgToAdmin*)self;
	if (pMsgToAdmin == NULL) {
		return -1;
	}
	char phonenum[30] = {0};
	ntyQueryPhoneBookSelectHandle(pMsgToAdmin->IMEI, pMsgToAdmin->proposer, phonenum);
	
	char *adminMsg = ntyJsonWriteBindOfflineMsgToAdmin(pMsgToAdmin, phonenum);
	if (adminMsg==NULL) {
		ntylog("-- ntyReadOfflineBindMsgToAdminIter iter null----\n");
		return -1;
	}

	free(pMsgToAdmin->watchName);
	free(pMsgToAdmin->watchImage);
	free(pMsgToAdmin->userName);
	free(pMsgToAdmin->userImage);

	return ntySendBindConfirmPushResult(pMsgToAdmin->proposer, pMsgToAdmin->admin, adminMsg, strlen(adminMsg));
}

int ntyReadOfflineBindMsgToAdminAction(C_DEVID devId) {
	NVector *container = ntyVectorCreator();
	if (NULL == container) return NTY_RESULT_FAILED;
	
	int ret = ntyQueryBindOfflineMsgToAdminSelectHandle(devId, container);
	if (ret == NTY_RESULT_SUCCESS) {
		ntyVectorIter(container, ntyReadOfflineBindMsgToAdminIter, &devId);
	} 

	ntylog(" ntyReadOfflineBindMsgToAdminAction --> ret : %d\n", ret);
	
	if (ret != NTY_RESULT_BUSY) {
		if (container->num == 0) { //count == 0
			ret = NTY_RESULT_NOEXIST;
		} else {
			ret = NTY_RESULT_SUCCESS;
		}
	} 
	
	ntyVectorDestory(container);
	return ret;
}




//add by luoyb.add begin
/*************************************************************************************************
** Function: ntyClientSelectScheduleReqAction 
** Description: client select schedule request,use client id to search from the DB,
**               and then compose the results to a json object,send to the client that is sender.
** Input: ClientActionParam *clientActionParamVal,ClientSelectReq *ptrclientSelectReq
** Output: 
** Return: int 0:succes -1 -2 -3:error
** Author:luoyb
** Date: 2017-04-28
** Others:
*************************************************************************************************/
int ntyClientSelectScheduleReqIter(void *self, void *arg) {
	ScheduleItem *ptrScheduleItem = (ScheduleItem*)self;
	ScheduleAck *ptrScheduleAck = (ScheduleAck*)arg;

	if (ptrScheduleAck->results.index < ptrScheduleAck->results.size) {
		ptrScheduleAck->results.pSchedule[ptrScheduleAck->results.index] = *ptrScheduleItem;
		ptrScheduleAck->results.index = ptrScheduleAck->results.index + 1;
	}
	
	return 0;
}


int ntyClientSelectScheduleReqAction( ClientActionParam *clientActionParamVal,ClientSelectReq *ptrclientSelectReq ){
	ntydbg( "ntyClientSelectScheduleReqAction fromId:%lld toId:%lld json:%s\n", clientActionParamVal->fromId, clientActionParamVal->toId,clientActionParamVal->jsonString );

	if ( clientActionParamVal == NULL ) return -1;

	C_DEVID fromId = clientActionParamVal->fromId;
	C_DEVID toId = clientActionParamVal->toId;
	
	
	NVector *container = ntyVectorCreator();
	if (NULL == container) return NTY_RESULT_FAILED;
	
	int ret = ntyExecuteClientSelectScheduleHandle(fromId, toId, container);
	if ( ret == -1 || ret == -2 ) {
		ntylog(" ntyClientSelectScheduleReqAction --> DB Exception\n");
	} else if (ret == NTY_RESULT_SUCCESS) {
		ScheduleAck *ptrScheduleAck = (ScheduleAck*)malloc( sizeof(ScheduleAck));
		if ( ptrScheduleAck == NULL ) {
			ntylog("ntyClientSelectScheduleReqAction --> malloc failed ScheduleAck\n");
			ret = -3;
			goto exit_ack;
		}
		memset(ptrScheduleAck, 0, sizeof(ScheduleAck));

		//copy json data struct ptrclientSelectReq to stuct ptrScheduleAck.
		ptrScheduleAck->results.IMEI = ptrclientSelectReq->IMEI;
		ptrScheduleAck->results.category = ptrclientSelectReq->Category;
		char num[32] = {0};
		sprintf(num, "%d", container->num);
		ptrScheduleAck->results.num = num;
		ptrScheduleAck->results.size = container->num;
		ptrScheduleAck->results.index = 0;

		ScheduleItem *ptrScheduleItem = (ScheduleItem*)malloc(sizeof(ScheduleItem)*container->num);
		if ( ptrScheduleItem == NULL ){
			ntylog("ntyClientSelectScheduleReqAction --> malloc failed ScheduleItem\n");
			ret = -4;
			goto exit_item;
		}
		memset( ptrScheduleItem, 0, sizeof(ScheduleItem)*container->num);	
		ptrScheduleAck->results.pSchedule = ptrScheduleItem; //copy pointer

		ntyVectorIterator(container, ntyClientSelectScheduleReqIter, ptrScheduleAck);
#if 0
		char *jsonResult = ntyJsonWriteSchedule(ptrScheduleAck);
		int nRet = ntySendUserDataAck(fromId, (U8*)jsonResult, strlen(jsonResult));
		ntylog( "-----send to client %lld, %d, ntyJsonWriteSchedule json:%s\n", fromId, nRet, jsonResult);
		ntyJsonFree(jsonResult);
#else
		U8 buffer[NTY_PACKET_BUFFER_SIZE] = {0};
		ret = ntyJsonWriteSchedule(ptrScheduleAck, buffer);
		if (ret < NTY_RESULT_SUCCESS) {
			free(ptrScheduleItem);
			ptrScheduleItem = NULL;

			goto exit_item;
		}

		int nRet = ntySendUserDataAck(fromId, buffer, ret);
		ntylog( "-----send to client %lld, %d\n", fromId, nRet);
#endif
		free(ptrScheduleItem);
		ptrScheduleItem = NULL;
exit_item:
		free(ptrScheduleAck);
		ptrScheduleAck = NULL;
	}
	
exit_ack:
	ntyVectorDestory(container);
	
	return ret;
}





/*************************************************************************************************
** Function: ntyClientSeleteContactsReqAction 
** Description: client select contacts request,use client id to search from the DB,
**               and then compose the results to a json object,send to the client that is sender.
** Input: ClientActionParam *clientActionParamVal,ClientSelectReq *ptrclientSelectReq
** Output: 
** Return: int 0:succes -1 -2 -3 -4:error
** Author:luoyb
** Date: 2017-04-28
** Others:
*************************************************************************************************/

int ntyClientSelectContactsReqIter(void *self, void *arg) {
	ClientContactsAckItem *pClientContactsAckItem = (ClientContactsAckItem*)self;
	ClientContactsAck *pClientContactsAck = (ClientContactsAck*)arg;

	if (pClientContactsAck->index < pClientContactsAck->size) {
		pClientContactsAck->objClientContactsAckItem[pClientContactsAck->index] = *pClientContactsAckItem;
		pClientContactsAck->index = pClientContactsAck->index + 1;
	}

	return 0;
}

int ntyClientSelectContactsReqAction( ClientActionParam *pClientActionParam,ClientSelectReq *pClientSelectReq )
{
	ntylog( "ntyClientSeleteContactsReqAction fromId:%lld toId:%lld json:%s\n", pClientActionParam->fromId, pClientActionParam->toId,pClientActionParam->jsonString );

	if ( pClientActionParam == NULL ) return -1;

	C_DEVID fromId = pClientActionParam->fromId;
	C_DEVID toId = pClientActionParam->toId;
	
	NVector *container = ntyVectorCreator();
	if (NULL == container) return NTY_RESULT_FAILED;
	
	int ret = ntyExecuteClientSelectContactsHandle(fromId, toId, container);
	if ( ret == -1 || ret == -2 ) {
		ntylog(" ntyExecuteClientSelectContactsHandle --> DB Exception\n");
	} else if (ret == NTY_RESULT_SUCCESS) {
		ClientContactsAck *pClientContactsAck = (ClientContactsAck*)malloc(sizeof(ClientContactsAck));
		if ( pClientContactsAck == NULL ) {
			ntylog("ntyClientSeleteContactsReqAction --> malloc failed ClientContactsAck\n");
			ret = -2;
			goto exit_ack;
		}
		memset(pClientContactsAck, 0, sizeof(ClientContactsAck));

		ntylog("ntyClientSelectContactsReqAction --> ClientContactsAck\n");
		
		pClientContactsAck->IMEI = pClientSelectReq->IMEI;
		pClientContactsAck->Category = pClientSelectReq->Category;
		char num[32] = {0};
		sprintf(num, "%d", container->num);
		pClientContactsAck->Num = num;
		pClientContactsAck->size = container->num;
		pClientContactsAck->index = 0;
		
		ClientContactsAckItem *pClientContactsAckItem = (ClientContactsAckItem*)malloc(sizeof(ClientContactsAckItem)*container->num);
		if (pClientContactsAckItem == NULL){
			ntylog("ntyClientSelectContactsReqAction --> malloc failed ClientContactsAckItem\n");
			ret = -3;
			goto exit_item;
		}
		memset(pClientContactsAckItem, 0, sizeof(ClientContactsAckItem)*container->num);	
		pClientContactsAck->objClientContactsAckItem = pClientContactsAckItem; //copy pointer

		ntylog("ntyClientSelectContactsReqAction --> pClientContactsAckItem : %d\n", container->num);

		ntyVectorIterator(container, ntyClientSelectContactsReqIter, pClientContactsAck);

		ntylog("ntyClientSelectContactsReqAction --> ntyClientContactsAckJsonCompose\n");
		
#if 0	//Update By WangBoJing	
		char *jsonResult = ntyClientContactsAckJsonCompose(pClientContactsAck);
		ntylog("-----send before.send to client %lld, ntyClientContactsAckJsonCompose json: %d, %s\n", fromId, (int)strlen(jsonResult), jsonResult);
		int nRet = ntySendUserDataAck(fromId, (U8*)jsonResult, strlen(jsonResult));
		ntylog("-----send after. the result:%d\n", nRet);
		ntyJsonFree(jsonResult);
#else
		U8 buffer[NTY_PACKET_BUFFER_SIZE] = {0};
		ret = ntyClientContactsAckJsonCompose(pClientContactsAck, buffer);
		if (ret < NTY_RESULT_SUCCESS) {
			free(pClientContactsAckItem);
			pClientContactsAckItem = NULL;

			goto exit_item;
		}

		int nRet = ntySendUserDataAck(fromId, buffer, ret);
		ntylog("-----send after. the result:%d\n", nRet);
#endif

		free(pClientContactsAckItem);
		pClientContactsAckItem = NULL;
		
exit_item:
		free(pClientContactsAck);
		pClientContactsAck = NULL;
	}

exit_ack:
	ntyVectorDestory(container);

	return ret;		
}





/*************************************************************************************************
** Function: ntyClientSelectTurnReqAction 
** Description: client select contacts request,use client id to search from the DB,
**               and then compose the results to a json object,send to the client that is sender.
** Input: ClientActionParam *clientActionParamVal,ClientSelectReq *ptrclientSelectReq
** Output: 
** Return: int 0:succes -1 -2 -3 -4:error
** Author:luoyb
** Date: 2017-05-2
** Others:
*************************************************************************************************/

int ntyClientSelectTurnReqIter(void *self, void *arg) {
	ClientTurnAckItem *pClientTurnAckItem = (ClientTurnAckItem*)self;
	ClientTurnAck *pClientTurnAck = (ClientTurnAck*)arg;
	memcpy(pClientTurnAck->objClientTurnAckItem, pClientTurnAckItem, sizeof(ClientTurnAckItem));

	return 0;
}

int ntyClientSelectTurnReqAction( ClientActionParam *pClientActionParam,ClientSelectReq *pClientSelectReq )
{
	ntydbg( "ntyClientSelectTurnReqAction fromId:%lld toId:%lld json:%s\n", pClientActionParam->fromId, pClientActionParam->toId,pClientActionParam->jsonString );

	if ( pClientActionParam == NULL ) return -1;
	C_DEVID fromId = pClientActionParam->fromId;
	C_DEVID toId = pClientActionParam->toId;
	
	NVector *container = ntyVectorCreator();
	if (NULL == container) return NTY_RESULT_FAILED;
	
	int ret = ntyExecuteClientSelectTurnHandle(fromId, toId, container);
	if ( ret == -1 || ret == -2 ) {
		ntylog(" ntyClientSelectTurnReqAction --> DB Exception\n");
	} else if (ret == NTY_RESULT_SUCCESS) {
		ClientTurnAck *pClientTurnAck = (ClientTurnAck*)malloc( sizeof(ClientTurnAck) );
		if ( pClientTurnAck == NULL ) {
			ntylog("ntyClientSelectTurnReqAction --> malloc failed ClientTurnAck\n");
			ret = -2;
			goto exit_ack;
		}	
		memset(pClientTurnAck, 0, sizeof(ClientTurnAck));
		pClientTurnAck->IMEI = pClientSelectReq->IMEI;
		pClientTurnAck->Category = pClientSelectReq->Category;

		ClientTurnAckItem *pClientTurnAckItem = (ClientTurnAckItem*)malloc(sizeof(ClientTurnAckItem));
		if ( pClientTurnAckItem == NULL ){
			ntylog("ntyClientSelectTurnReqAction --> malloc failed ClientTurnAckItem\n");
			ret = -3;
			goto exit_item;
		}
		memset( pClientTurnAckItem, 0, sizeof(ClientTurnAckItem));
		pClientTurnAck->objClientTurnAckItem = pClientTurnAckItem; //copy pointer

		ntyVectorIterator(container, ntyClientSelectTurnReqIter, pClientTurnAck);
		
#if 0
		char *jsonResult = ntyClientTurnAckJsonCompose(pClientTurnAck);
		ntylog( "-----send to client %lld, ntyClientTurnAckJsonCompose json:%s\n", fromId,jsonResult );
		int nRet = ntySendUserDataAck( fromId, (U8*)jsonResult, strlen(jsonResult) );
		ntylog( "-----send after. the result:%d\n", nRet );
		ntyJsonFree( jsonResult );
#else

		U8 buffer[NTY_PACKET_BUFFER_SIZE] = {0};

		ret = ntyClientTurnAckJsonCompose(pClientTurnAck, buffer);
		if (ret < NTY_RESULT_SUCCESS) {
			free( pClientTurnAckItem );
			pClientTurnAckItem = NULL;

			goto exit_item;
		}

		int nRet = ntySendUserDataAck( fromId, buffer, ret);
		ntylog( "-----send after. the result:%d\n", nRet );

#endif
		free( pClientTurnAckItem );
		pClientTurnAckItem = NULL;
		
exit_item:
		free( pClientTurnAck );
		pClientTurnAck = NULL;
	}
	
exit_ack:
	ntyVectorDestory(container);

	return ret;	
}




/*************************************************************************************************
** Function: ntyClientSelectRunTimeReqAction 
** Description: client select contacts request,use client id to search from the DB,
**               and then compose the results to a json object,send to the client that is sender.
** Input: ClientActionParam *clientActionParamVal,ClientSelectReq *ptrclientSelectReq
** Output: 
** Return: int 0:succes -1 -2 -3 -4:error
** Author:luoyb
** Date: 2017-05-2
** Others:
*************************************************************************************************/

int ntyClientSelectRunTimeReqIter(void *self, void *arg) {
	ClientRunTimeAckItem *pClientRunTimeAckItem = (ClientRunTimeAckItem*)self;
	ClientRunTimeAck *pClientRunTimeAck = (ClientRunTimeAck*)arg;
	memcpy(pClientRunTimeAck->objClientRunTimeAckItem, pClientRunTimeAckItem, sizeof(ClientRunTimeAckItem));
	
	return 0;
}

int ntyClientSelectRunTimeReqAction( ClientActionParam *pClientActionParam,ClientSelectReq *pClientSelectReq )
{
	ntydbg( "ntyClientSelectRunTimeReqAction fromId:%lld toId:%lld json:%s\n", pClientActionParam->fromId, pClientActionParam->toId,pClientActionParam->jsonString );
	
	if ( pClientActionParam == NULL ) return -1;
	C_DEVID fromId = pClientActionParam->fromId;
	C_DEVID toId = pClientActionParam->toId;
	
	NVector *container = ntyVectorCreator();
	if (NULL == container) return NTY_RESULT_FAILED;
	
	int ret = ntyExecuteClientSelectRunTimeHandle(fromId, toId, container);
	if ( ret == -1 || ret == -2 ) {
		ntylog(" ntyClientSelectTurnReqAction --> DB Exception\n");
	} else if (ret == NTY_RESULT_SUCCESS) {
		ClientRunTimeAck *pClientRunTimeAck = (ClientRunTimeAck*)malloc( sizeof(ClientRunTimeAck) );
		if ( pClientRunTimeAck == NULL ) {
			ntylog("ntyClientSelectRunTimeReqAction --> malloc failed ClientRunTimeAck\n");
			ret = -2;
			goto exit_ack;
		}
		memset( pClientRunTimeAck, 0, sizeof(ClientRunTimeAck) );
		pClientRunTimeAck->IMEI = pClientSelectReq->IMEI;
		pClientRunTimeAck->Category = pClientSelectReq->Category;

		ClientRunTimeAckItem *pClientRunTimeAckItem = (ClientRunTimeAckItem*)malloc(sizeof(ClientRunTimeAckItem));
		if ( pClientRunTimeAckItem == NULL ){
			ntylog("ntyClientSelectRunTimeReqAction --> malloc failed ClientRunTimeAckItem\n");
			ret = -3;
			goto exit_item;
		}
		memset(pClientRunTimeAckItem, 0, sizeof(ClientRunTimeAckItem));	
		pClientRunTimeAck->objClientRunTimeAckItem = pClientRunTimeAckItem; //copy pointer

		ntyVectorIterator(container, ntyClientSelectRunTimeReqIter, pClientRunTimeAck);
#if 0
		char *jsonResult = ntyClientRunTimeAckJsonCompose( pClientRunTimeAck );
		ntylog( "-----send to client %lld, ntyClientRunTimeAckJsonCompose json:%s\n", fromId,jsonResult );
		int nRet = ntySendUserDataAck( fromId, (U8*)jsonResult, strlen(jsonResult) );
		ntylog( "-----send after. the result:%d\n", nRet );
		ntyJsonFree( jsonResult );
#else
		U8 buffer[NTY_PACKET_BUFFER_SIZE] = {0};

		ret = ntyClientRunTimeAckJsonCompose(pClientRunTimeAck, buffer);
		if (ret < NTY_RESULT_SUCCESS) {
			free( pClientRunTimeAckItem );
			pClientRunTimeAckItem = NULL;

			goto exit_item;
		}

		int nRet = ntySendUserDataAck( fromId, buffer, ret);
		ntylog( "-----send after. the result:%d\n", nRet );
#endif
		free( pClientRunTimeAckItem );
		pClientRunTimeAckItem = NULL;
		
exit_item:
		free( pClientRunTimeAck );
		pClientRunTimeAck = NULL;
	}

exit_ack:
	ntyVectorDestory(container);

	return ret;	
}




/*************************************************************************************************
** Function: ntyClientSelectTimeTablesReqAction 
** Description: client select contacts request,use client id to search from the DB,
**               and then compose the results to a json object,send to the client that is sender.
** Input: ClientActionParam *clientActionParamVal,ClientSelectReq *ptrclientSelectReq
** Output: 
** Return: int 0:succes -1 -2 -3 -4:error
** Author:luoyb
** Date: 2017-05-2
** Others:
*************************************************************************************************/

int ntyClientSelectTimeTablesReqIter(void *self, void *arg) {
	ClientTimeTablesAckItem *pClientTimeTablesAckItem = (ClientTimeTablesAckItem*)self;
	ClientTimeTablesAck *pClientTimeTablesAck = (ClientTimeTablesAck*)arg;

	if (pClientTimeTablesAck->index < pClientTimeTablesAck->size) {
		pClientTimeTablesAck->objClientTimeTablesAckItem[pClientTimeTablesAck->index] = *pClientTimeTablesAckItem;
		pClientTimeTablesAck->index = pClientTimeTablesAck->index + 1;
	}

	return 0;
}

int ntyClientSelectTimeTablesReqAction( ClientActionParam *pClientActionParam,ClientSelectReq *pClientSelectReq )
{
	ntydbg( "ntyClientSelectTimeTablesReqAction fromId:%lld toId:%lld json:%s\n", pClientActionParam->fromId, pClientActionParam->toId,pClientActionParam->jsonString );

	if ( pClientActionParam == NULL ) return -1;
	C_DEVID fromId = pClientActionParam->fromId;
	C_DEVID toId = pClientActionParam->toId;
	
	NVector *container = ntyVectorCreator();
	if (NULL == container) return NTY_RESULT_FAILED;
	
	int ret = ntyExecuteClientSelectTimeTablesHandle(fromId, toId, container);
	if ( ret == -1 || ret == -2 ) {
		ntylog(" ntyClientSelectTurnReqAction --> DB Exception\n");
	} else if (ret == NTY_RESULT_SUCCESS) {
		ClientTimeTablesAck *pClientTimeTablesAck = (ClientTimeTablesAck*)malloc( sizeof(ClientTimeTablesAck) );
		if ( pClientTimeTablesAck == NULL ) {
			ntylog("ntyClientSelectTimeTablesReqAction --> malloc failed ClientTimeTablesAck\n");
			goto exit_ack;
		}	
		memset( pClientTimeTablesAck, 0, sizeof(ClientTimeTablesAck) );	
		pClientTimeTablesAck->IMEI = pClientSelectReq->IMEI;
		pClientTimeTablesAck->Category = pClientSelectReq->Category;
		pClientTimeTablesAck->size = container->num;
		pClientTimeTablesAck->index = 0;

		ClientTimeTablesAckItem *pClientTimeTablesAckItem = (ClientTimeTablesAckItem*)malloc(sizeof(ClientTimeTablesAckItem)*container->num);
		if ( pClientTimeTablesAckItem == NULL ){
			ntylog("ntyClientSelectTimeTablesReqAction --> malloc failed ClientTimeTablesAckItem\n");
			ret = -3;
			goto exit_item;
		}
		memset(pClientTimeTablesAckItem, 0, sizeof(ClientTimeTablesAckItem)*container->num);	
		pClientTimeTablesAck->objClientTimeTablesAckItem = pClientTimeTablesAckItem; //copy pointer

		ntyVectorIter(container, ntyClientSelectTimeTablesReqIter, pClientTimeTablesAck);
#if 0
		char *jsonResult = ntyClientTimeTablesAckJsonCompose( pClientTimeTablesAck );
		ntylog( "-----send to client %lld, ntyClientTimeTablesAckJsonCompose json:%s\n", fromId,jsonResult );
		int nRet = ntySendUserDataAck( fromId, (U8*)jsonResult, strlen(jsonResult) );
		ntylog( "-----send after. the result:%d\n", nRet );
		ntyJsonFree( jsonResult );
#else
		U8 buffer[NTY_PACKET_BUFFER_SIZE] = {0};
		ret = ntyClientTimeTablesAckJsonCompose(pClientTimeTablesAck, buffer);
		if (ret < NTY_RESULT_SUCCESS) {
			free( pClientTimeTablesAckItem );
			pClientTimeTablesAckItem = NULL;

			goto exit_item;
		}

		int nRet = ntySendUserDataAck( fromId, buffer, ret );

		ntylog( "-----send after. the result:%d\n", nRet );
#endif
		free( pClientTimeTablesAckItem );
		pClientTimeTablesAckItem = NULL;
		
exit_item:
		free( pClientTimeTablesAck );
		pClientTimeTablesAck = NULL;
	}

exit_ack:
	ntyVectorDestory(container);

	return ret;	
}



/*************************************************************************************************
** Function: ntyClientSelectLocationReqAction 
** Description: client select contacts request,use client id to search from the DB,
**               and then compose the results to a json object,send to the client that is sender.
** Input: ClientActionParam *clientActionParamVal,ClientSelectReq *ptrclientSelectReq
** Output: 
** Return: int 0:succes -1 -2 -3 -4:error
** Author:luoyb
** Date: 2017-05-2
** Others:
*************************************************************************************************/

int ntyClientSelectLocationReqIter(void *self, void *arg) {
	ClientLocationAckResults *pClientLocationAckResults = (ClientLocationAckResults*)self;
	ClientLocationAck *pClientLocationAck = (ClientLocationAck*)arg;
	memcpy(pClientLocationAck->results, pClientLocationAckResults, sizeof(ClientLocationAckResults));

	return 0;
}

int ntyClientSelectLocationReqAction( ClientActionParam *pClientActionParam,ClientSelectReq *pClientSelectReq )
{
	ntydbg( "ntyClientSelectLocationReqAction fromId:%lld toId:%lld json:%s\n", pClientActionParam->fromId, pClientActionParam->toId,pClientActionParam->jsonString );

	if ( pClientActionParam == NULL ) return -1;
	C_DEVID fromId = pClientActionParam->fromId;
	C_DEVID toId = pClientActionParam->toId;
	
	NVector *container = ntyVectorCreator();
	if (NULL == container) return NTY_RESULT_FAILED;
	
	int ret = ntyExecuteClientSelectLocationHandle(fromId, toId, container);
	if ( ret == -1 || ret == -2 ) {
		ntylog(" ntyClientSelectLocationReqAction --> DB Exception\n");
	} else if (ret == NTY_RESULT_SUCCESS) {
		ClientLocationAck *pClientLocationAck = (ClientLocationAck*)malloc( sizeof(ClientLocationAck) );
		if ( pClientLocationAck == NULL ) {
			ntylog("ntyClientSelectLocationReqAction --> malloc failed ClientLocationAck\n");
			ret = -2;
			goto exit_ack;
		}
		memset( pClientLocationAck, 0, sizeof(ClientLocationAck) );	
		pClientLocationAck->IMEI = pClientSelectReq->IMEI;
		pClientLocationAck->Category = pClientSelectReq->Category;
		
		ClientLocationAckResults *pClientLocationAckResults = (ClientLocationAckResults*)malloc(sizeof(ClientLocationAckResults));
		if ( pClientLocationAckResults == NULL ){
			ntylog("ntyClientSelectLocationReqAction --> malloc failed ClientRunTimeAckItem\n");
			ret = -3;
			goto exit_item;
		}
		memset(pClientLocationAckResults, 0, sizeof(ClientLocationAckResults));	
		pClientLocationAck->results = pClientLocationAckResults; //copy pointer

		ntyVectorIterator(container, ntyClientSelectLocationReqIter, pClientLocationAck);
#if 0
		char *jsonResult = ntyClientLocationAckJsonCompose( pClientLocationAck );
		ntylog( "-----send to client %lld, ntyClientLocationAckJsonCompose json:%s\n", fromId,jsonResult );
		int nRet = ntySendUserDataAck( fromId, (U8*)jsonResult, strlen(jsonResult) );
		ntylog( "-----send after. the result:%d\n", nRet );
		ntyJsonFree( jsonResult );
#else
		U8 buffer[NTY_PACKET_BUFFER_SIZE] = {0};
		ret = ntyClientLocationAckJsonCompose(pClientLocationAck, buffer);
		if (ret < NTY_RESULT_SUCCESS) {
			free( pClientLocationAckResults );
			pClientLocationAckResults = NULL;

			goto exit_item;
		}

		int nRet = ntySendUserDataAck( fromId, buffer, ret );

		ntylog( "-----send after. the result:%d\n", nRet );
#endif
		free( pClientLocationAckResults );
		pClientLocationAckResults = NULL;
		
exit_item:
		free( pClientLocationAck );
		pClientLocationAck = NULL;
	}

exit_ack:
	ntyVectorDestory(container);

	return ret;
}

/*************************************************************************************************
** Function: ntyClientSelectEfenceReqAction 
** Description: client select contacts request,use client id to search from the DB,
**               and then compose the results to a json object,send to the client that is sender.
** Input: ClientActionParam *clientActionParamVal,ClientSelectReq *ptrclientSelectReq
** Output: 
** Return: int 0:succes -1 -2 -3 -4:error
** Author:luoyb
** Date: 2017-05-2
** Others:
*************************************************************************************************/

int ntyClientSelectEfenceReqIter(void *self, void *arg) {
	ClientEfenceListItem *pClientEfenceListItem = (ClientEfenceListItem*)self;
	ClientEfenceAck *pClientEfenceAck = (ClientEfenceAck*)arg;

	if (pClientEfenceAck->results.index < pClientEfenceAck->results.efencelist_size) {
		pClientEfenceAck->results.pClientEfenceListItem[pClientEfenceAck->results.index] = *pClientEfenceListItem;
		pClientEfenceAck->results.index = pClientEfenceAck->results.index + 1;
	}

	return 0;
}

int ntyClientSelectEfenceReqAction( ClientActionParam *pClientActionParam,ClientSelectReq *pClientSelectReq )
{
	ntylog( "ntyClientSelectEfenceReqAction fromId:%lld toId:%lld json:%s\n", pClientActionParam->fromId, pClientActionParam->toId,pClientActionParam->jsonString );
	if ( pClientActionParam == NULL ) return -1;

	C_DEVID fromId = pClientActionParam->fromId;
	C_DEVID toId = pClientActionParam->toId;
	
	NVector *container = ntyVectorCreator();
	if (NULL == container) return NTY_RESULT_FAILED;
	
	int ret = ntyExecuteClientSelectEfenceHandle(fromId, toId, container);
	if ( ret == -1 || ret == -2 ) {
		ntylog(" ntyExecuteClientSelectContactsHandle --> DB Exception\n");
	} else if (ret == NTY_RESULT_SUCCESS) {
		ClientEfenceAck *pClientEfenceAck = (ClientEfenceAck*)malloc( sizeof(ClientEfenceAck) );
		if ( pClientEfenceAck == NULL ) {
			ntylog("ntyClientSelectEfenceReqAction --> malloc failed ClientEfenceAck\n");
			ret = -2;
			goto exit_ack;
		}	
		memset( pClientEfenceAck, 0, sizeof(ClientEfenceAck) );	

		pClientEfenceAck->results.IMEI = pClientSelectReq->IMEI;
		pClientEfenceAck->results.category = pClientSelectReq->Category;
		pClientEfenceAck->results.efencelist_size = container->num;
		pClientEfenceAck->results.index = 0;

		ClientEfenceListItem *pClientEfenceListItem = (ClientEfenceListItem*)malloc(sizeof(ClientEfenceListItem)*container->num);
		if ( pClientEfenceListItem == NULL ){
			ntylog("ntyClientSelectEfenceReqAction --> malloc failed ClientContactsAckItem\n");
			ret = -3;
			goto exit_item;
		}
		memset( pClientEfenceListItem, 0, sizeof(ClientEfenceListItem)*container->num);	
		pClientEfenceAck->results.pClientEfenceListItem = pClientEfenceListItem; //copy pointer

		ntyVectorIter(container, ntyClientSelectEfenceReqIter, pClientEfenceAck);

		char *jsonResult = ntyClientEfenceAckJsonCompose( pClientEfenceAck );
		ntylog( "-----send to client %lld, ntyClientSelectEfenceReqAction json:%s\n", fromId,jsonResult );
		int nRet = ntySendUserDataAck( fromId, (U8*)jsonResult, strlen(jsonResult) );
		ntylog( "-----send after. the result:%d\n", nRet );
		ntyJsonFree( jsonResult );

		free(pClientEfenceListItem);
		pClientEfenceListItem = NULL;
exit_item:
		free( pClientEfenceAck);
		pClientEfenceAck = NULL;
	}

exit_ack:
	ntyVectorDestory(container);

	return ret;
}



/*************************************************************************************************
** Function: ntyClientSelectURLReqAction 
** Description: client select contacts request,use client id to search from the DB,
**               and then compose the results to a json object,send to the client that is sender.
** Input: ClientActionParam *clientActionParamVal,ClientSelectReq *ptrclientSelectReq
** Output: 
** Return: int 0:succes -1 -2 -3 -4:error
** Author:luop
** Date: 2017-05-10
** Others:
*************************************************************************************************/
int ntyClientSelectURLReqAction( ClientActionParam *pClientActionParam,ClientSelectReq *pClientSelectReq )
{
	ntydbg( "ntyClientSelectURLReqAction fromId:%lld toId:%lld json:%s\n", pClientActionParam->fromId, pClientActionParam->toId,pClientActionParam->jsonString );

	if ( pClientActionParam == NULL ) return -1;
	C_DEVID fromId = pClientActionParam->fromId;
	C_DEVID toId = pClientActionParam->toId;
	
	ClientURLAck *pClientURLAck = (ClientURLAck*)malloc( sizeof(ClientURLAck) );
	if ( pClientURLAck == NULL ) {
		ntylog("ntyClientSelectURLReqAction --> malloc failed ClientURLAck\n");
		return -2;
	}
	memset( pClientURLAck, 0, sizeof(ClientURLAck) );	
	pClientURLAck->IMEI = pClientSelectReq->IMEI;
	pClientURLAck->Category = pClientSelectReq->Category;

	char url_qrcode[128] = {0};
	strcat(url_qrcode, HTTP_QRCODE_URL);
	pClientURLAck->objClientURLAckItem.QRCode = url_qrcode;
#if 0
	char *jsonResult = ntyClientURLAckJsonCompose( pClientURLAck );
	ntylog( "-----send to client %lld, ntyClientLocationAckJsonCompose json:%s\n", fromId, jsonResult);
	int nRet = ntySendUserDataAck( fromId, (U8*)jsonResult, strlen(jsonResult) );
	ntylog( "-----send after. the result:%d\n", nRet );
	ntyJsonFree( jsonResult );
#else
	U8 buffer[NTY_PACKET_BUFFER_SIZE] = {0};
	int ret = ntyClientURLAckJsonCompose(pClientURLAck, buffer);
	if (ret < NTY_RESULT_SUCCESS) {
		free( pClientURLAck );
		pClientURLAck = NULL;

		return NTY_RESULT_ERROR;
	}

	int nRet = ntySendUserDataAck( fromId, buffer, ret);
	ntylog( "-----send after. the result:%d\n", nRet );
	
#endif
	free( pClientURLAck );
	pClientURLAck = NULL;

	return 0;
}


int ntyLocatorBindReqAction( ClientActionParam *pClientActionParam, LocatorBindReq *pLocatorBindReq ){
	
	if ( (pClientActionParam==NULL) || (pLocatorBindReq==NULL) ){ 
		ntylog( "ntyLocatorBindReqAction pClientActionParam==NULL||pLocatorBindReq==NULL\n" );
		return -1;
	}
	C_DEVID appId = pClientActionParam->fromId;
	C_DEVID devId = pClientActionParam->toId;
	int nRet = 0;
	ntylog( "ntyLocatorBindReqAction appId:%lld,devId:%lld,json:%s\n", appId, devId, pClientActionParam->jsonString );
	nRet = ntyExecuteLocatorBindInsertHandle( appId, devId, pLocatorBindReq );

	return nRet;
}

int ntyLocatorUnBindReqAction( C_DEVID appId, C_DEVID devId ){
	int nRet = 0;
	nRet = ntyExecuteLocatorUnBindDeleteHandle( appId, devId );
	return nRet;
}

//end



