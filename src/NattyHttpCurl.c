/*
 *  Author : WangBoJing , email : 1989wangbojing@gmail.com
 * 
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of Author. (C) 2016
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


#include "NattyHttpCurl.h"
#include "NattyDaveMQ.h"
#include "NattyFilter.h"
#include "NattyRBTree.h"
#include "NattyDBOperator.h"
#include "NattySession.h"
#include "NattyUtils.h"
#include "NattyUserProtocol.h"
#include "NattyDaveMQ.h"
#include "NattyJson.h"

#include <curl/curl.h>
#include <string.h>
#include <stdio.h>
#include <wchar.h>

#define JEMALLOC_NO_DEMANGLE 1
#define JEMALLOC_NO_RENAME	 1
#include <jemalloc/jemalloc.h>



static size_t ntyHttpQJKFallenHandleResult(void* buffer, size_t size, size_t nmemb, void *stream) {
	VALUE_TYPE *tag = stream;
	U8 u8ResultBuffer[256] = {0};

	ntylog("ntyHttpQJKFallenHandleResult --> length:%ld\n", size*nmemb);
	//ntylog("buffer:%s, %ld\n", (char*)buffer, size*nmemb);
	sprintf(u8ResultBuffer, "Set Fall %d", 1);
	if (tag == NULL) goto exit; 
	//strcpy(u8ResultBuffer, "Set Fall 1", 10);
#if 0
	Client *cv = ntyRBTreeInterfaceSearch(tree, tag->fromId);
	if (cv == NULL) { 
		ntylog(" http qjk fallen fromId not exist\n");
		goto exit;
	}
	
	ntySendDeviceRouterInfo(cv, ack, strlen(ack));
#else
#if 0
	ntyProtoHttpProxyTransform(tag->fromId, tag->toId, u8ResultBuffer, strlen(u8ResultBuffer+NTY_PROTO_DATAPACKET_CONTENT_IDX));
#else
	ntyBoardcastAllFriendsById(tag->fromId, u8ResultBuffer, strlen(u8ResultBuffer));
#endif
	ntyProtoHttpRetProxyTransform(tag->toId, u8ResultBuffer, strlen(u8ResultBuffer));
#endif

exit:	


	return size*nmemb;
}

int ntyHttpQJKFallen(void *arg) {
	CURL *curl;	
	CURLcode res;	
	VALUE_TYPE *tag = arg;
#if 0
	CURLcode return_code;
	return_code = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != return_code) {
		ntylog("init libcurl failed.\n");		
		return -1;
	}
#endif
	curl = curl_easy_init();	
	if (!curl)	{		
		ntylog("curl init failed\n");		
		return -2;	
	}

	ntylog("QJK url:%s\n", tag->Tag);

	curl_easy_setopt(curl, CURLOPT_URL, tag->Tag); 
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L); 
#if 1
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
#endif
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ntyHttpQJKFallenHandleResult); 
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, tag); 

	res = curl_easy_perform(curl);	
	if (res != CURLE_OK)	{		
		switch(res)		{			
			case CURLE_UNSUPPORTED_PROTOCOL:				
				ntylog("CURLE_UNSUPPORTED_PROTOCOL\n");			
			case CURLE_COULDNT_CONNECT:				
				ntylog("CURLE_COULDNT_CONNECT\n");			
			case CURLE_HTTP_RETURNED_ERROR:				
				ntylog("CURLE_HTTP_RETURNED_ERROR\n");			
			case CURLE_READ_ERROR:				
				ntylog("CURLE_READ_ERROR\n");			
			default:				
				ntylog("default %d\n",res);		
		}		
		return -3;	
	}	
	
	curl_easy_cleanup(curl);
	
#if 1
#if ENABLE_DAVE_MSGQUEUE_MALLOC
	if (tag->Tag != NULL) {
		free(tag->Tag);
		tag->Tag = NULL;
	}
#endif
	if (tag != NULL) {
		free(tag);
		tag = NULL;
	}
#endif

	return 0;
}

//MSG_TYPE_GAODE_WIFI_CELL_API

static int ntyHttpGaodeGetLocationInfo(U8 *buffer, int len, U8 *lng, U8 *lat) {
	const U8 *pattern_start = "<location>";
	const U8 *pattern_end = "</location>";

	int len_start = strlen(pattern_start);
	int len_end = strlen(pattern_end);

	int i, j, k = 0;
	
#define PATTERN_COUNT 			16
#define LOCATION_INFO_COUNT		64
	
	U32 start_matches[PATTERN_COUNT] = {0};
	U32 end_matches[PATTERN_COUNT] = {0};
	U8 location[LOCATION_INFO_COUNT] = {0};

	if (len < len_start && len < len_end) return -1;

	ntyKMP(buffer, len, pattern_start, len_start, start_matches);
	ntyKMP(buffer, len, pattern_end, len_end, end_matches);

	if (start_matches[0] == 0) {
		return -2;
	}

	for (i = 0;i < PATTERN_COUNT;i ++) {
		if (start_matches[i] != 0 && start_matches[i] < end_matches[i]) {
			for (j = start_matches[i]+len_start ; j < end_matches[i] ; j ++) {
				location[(k >= LOCATION_INFO_COUNT ? 0 : k++)] = buffer[j];
			}
			//ntylog("location:%s\n", location);

			k = 0;
			while (location[k++] != ',');
				
			memcpy(lng, location, k-1);
			memcpy(lat, location+k, strlen(location)-k);
			
			ntylog("lat:%s, lon:%s\n", lat, lng);
			
			break;
		}
	}

	return k;
}
static int ntyHttpGaodeGetDescInfo(U8 *buffer, int len, U8 *desc) {

	const U8 *pattern_start = "<desc>";
	const U8 *pattern_end = "</desc>";

	int len_start = strlen(pattern_start);
	int len_end = strlen(pattern_end);

	int i, j, k = 0;
	
#define PATTERN_COUNT 			16
#define DESC_INFO_COUNT			256
	
	U32 start_matches[PATTERN_COUNT] = {0};
	U32 end_matches[PATTERN_COUNT] = {0};
	//U8 u8Desc[DESC_INFO_COUNT] = {0};

	if (len < len_start && len < len_end) return -1;

	ntyKMP(buffer, len, pattern_start, len_start, start_matches);
	ntyKMP(buffer, len, pattern_end, len_end, end_matches);

	if (start_matches[0] == 0) {
		return -2;
	}

	for (i = 0;i < PATTERN_COUNT;i ++) {
		if (start_matches[i] != 0 && start_matches[i] < end_matches[i]) {
			for (j = start_matches[i]+len_start ; j < end_matches[i] ; j ++) {
				desc[(k >= DESC_INFO_COUNT ? 0 : k++)] = buffer[j];
			}
			
			ntylog("desc:%s\n", desc);
			//wprintf(desc, "%ls", u8Desc);
			
			break;
		}
	}

	return k;
}


static size_t ntyHttpGaodeWifiCellAPIHandleResult(void* data, size_t size, size_t nmemb, void *stream) {
	VALUE_TYPE *tag = stream;
	if (tag == NULL) return size*nmemb; 
#if 0 //add desc kmp
	const U8 *pattern_start = "<location>";
	const U8 *pattern_end = "</location>";

	int len_start = strlen(pattern_start);
	int len_end = strlen(pattern_end);

	int i, j, k = 0, len = size*nmemb;

#define PATTERN_COUNT 			16
#define LOCATION_INFO_COUNT		64

	U32 start_matches[PATTERN_COUNT] = {0};
	U32 end_matches[PATTERN_COUNT] = {0};
	U8 location[LOCATION_INFO_COUNT] = {0};

	U8 u8Lat[PATTERN_COUNT] = {0};
	U8 u8Lon[PATTERN_COUNT] = {0};
	U8 *buffer = data;
	U8 u8ResultBuffer[256] = {0};
	
	ntyKMP(buffer, len, pattern_start, len_start, start_matches);
	ntyKMP(buffer, len, pattern_end, len_end, end_matches);

	if (start_matches[0] == 0) {
		ntylog(" result failed: %s\n", buffer);

		sprintf(u8ResultBuffer, "Set Location LatLonFailed");
		ntyProtoHttpRetProxyTransform(tag->fromId, u8ResultBuffer, strlen(u8ResultBuffer));
#if 0
		free(tag->Tag);
		free(tag);
#endif		
		return size*nmemb;
	}

	for (i = 0;i < PATTERN_COUNT;i ++) {
		if (start_matches[i] != 0 && start_matches[i] < end_matches[i]) {
			for (j = start_matches[i]+len_start ; j < end_matches[i] ; j ++) {
				location[(k >= LOCATION_INFO_COUNT ? 0 : k++)] = buffer[j];
			}
			//ntylog("location:%s\n", location);

			k = 0;
			while (location[k++] != ',');
				
			memcpy(u8Lon, location, k-1);
			memcpy(u8Lat, location+k, strlen(location)-k);
			
			ntylog("lat:%s, lon:%s\n", u8Lat, u8Lon);
			
			sprintf(u8ResultBuffer, "Set Location %s:%s:%d", u8Lat, u8Lon, tag->u8LocationType);
#if 0
			if (tag->toId != 0x0) {
				ntyProtoHttpProxyTransform(tag->fromId, tag->toId, u8ResultBuffer, strlen(u8ResultBuffer+NTY_PROTO_DATAPACKET_CONTENT_IDX));
			}
#else
			ntylog("ntyHttpGaodeWifiCellAPIHandleResult --> Cmd : %s, length:%ld\n", u8ResultBuffer, strlen(u8ResultBuffer));
			ntyBoardcastAllFriendsById(tag->fromId, u8ResultBuffer, strlen(u8ResultBuffer));
#endif
			ntyProtoHttpRetProxyTransform(tag->fromId, u8ResultBuffer, strlen(u8ResultBuffer));
			//ntyProtoHttpProxyTransform(tag->toId, tag->fromId, u8ResultBuffer, strlen(u8ResultBuffer+NTY_PROTO_DATAPACKET_CONTENT_IDX));
			break;
		}
	}
#if 0
	free(tag->Tag);
	free(tag);
#endif
#else

#define PATTERN_COUNT 			16
#define DESC_INFO_COUNT			256

	U8 *buffer = data;
	U8 u8Lat[PATTERN_COUNT] = {0};
	U8 u8Lon[PATTERN_COUNT] = {0};

	U8 u8Desc[DESC_INFO_COUNT] = {0};
	wchar_t wDesc[DESC_INFO_COUNT] = {0};

	U8 u8ResultBuffer[DESC_INFO_COUNT] = {0};
	
	int len = size*nmemb;

	if (0 > ntyHttpGaodeGetLocationInfo(buffer, len, u8Lon, u8Lat)) { //no exist 
		ntylog(" result failed: %s\n", buffer);

		sprintf(u8ResultBuffer, "Set Location LatLonFailed");
		ntyProtoHttpRetProxyTransform(tag->fromId, u8ResultBuffer, strlen(u8ResultBuffer));

	} else {
		if (0 > ntyHttpGaodeGetDescInfo(buffer, len, u8Desc)) {
			ntylog(" result failed: %s\n", buffer);
		}
#if ENABLE_CONNECTION_POOL
		//ntylog(" u8Desc : %s\n", u8Desc);
		//int wLen = ntyCharToWchar(u8Desc, strlen(u8Desc), wDesc);
		//ntylog(" wLen:%d wDesc:%ls\n", wLen, wDesc);
		ntyExecuteLocationInsertHandle(tag->fromId, u8Lon, u8Lat, tag->u8LocationType, u8Desc);
#endif
		sprintf(u8ResultBuffer, "Set Location %s:%s:%d", u8Lat, u8Lon, tag->u8LocationType);
		ntyBoardcastAllFriendsById(tag->fromId, u8ResultBuffer, strlen(u8ResultBuffer));
		ntyProtoHttpRetProxyTransform(tag->fromId, u8ResultBuffer, strlen(u8ResultBuffer));
			
	}

#endif

	return size*nmemb;
}


int ntyHttpGaodeWifiCellAPI(void *arg) {
	CURL *curl;	
	CURLcode res;	
	VALUE_TYPE *tag = arg;
#if 0
	CURLcode return_code;
	return_code = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != return_code) {
		ntylog("init libcurl failed.\n");		
		return -1;
	}
#endif
	curl = curl_easy_init();	
	if (!curl)	{		
		ntylog("curl init failed\n");		
		return -2;	
	}

	ntylog("GAODE url:%s\n", tag->Tag);

	curl_easy_setopt(curl, CURLOPT_URL, tag->Tag); 
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L); 
#if 1
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
#endif
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ntyHttpGaodeWifiCellAPIHandleResult); 
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, tag); 

	res = curl_easy_perform(curl);	
	if (res != CURLE_OK)	{		
		switch(res)		{			
			case CURLE_UNSUPPORTED_PROTOCOL:				
				ntylog("CURLE_UNSUPPORTED_PROTOCOL\n");			
			case CURLE_COULDNT_CONNECT:				
				ntylog("CURLE_COULDNT_CONNECT\n");			
			case CURLE_HTTP_RETURNED_ERROR:				
				ntylog("CURLE_HTTP_RETURNED_ERROR\n");			
			case CURLE_READ_ERROR:				
				ntylog("CURLE_READ_ERROR\n");			
			default:				
				ntylog("default %d\n",res);		
		}		
		return -3;	
	}	

	ntylog(" ntyHttpGaodeWifiCellAPI --> res:%d\n", res);
	curl_easy_cleanup(curl);
	
#if 1
#if ENABLE_DAVE_MSGQUEUE_MALLOC
	if (tag->Tag != NULL) {
		free(tag->Tag);
		tag->Tag = NULL;
	}
#endif
	if (tag != NULL) {
		free(tag);
		tag = NULL;
	}
#endif
	
	return 0;
}

static size_t ntyHttpMtkQuickLocationHandleResult(void* data, size_t size, size_t nmemb, void *stream) {
	VALUE_TYPE *tag = stream;
	if (tag == NULL) return size*nmemb; 
#if 0
	const U8 *pattern_start = "<location>";
	const U8 *pattern_end = "</location>";

	int len_start = strlen(pattern_start);
	int len_end = strlen(pattern_end);

	int i, j, k = 0, len = size*nmemb;

#define PATTERN_COUNT 			16
#define LOCATION_INFO_COUNT		64

	U32 start_matches[PATTERN_COUNT] = {0};
	U32 end_matches[PATTERN_COUNT] = {0};
	U8 location[LOCATION_INFO_COUNT] = {0};

	U8 u8Lat[PATTERN_COUNT] = {0};
	U8 u8Lon[PATTERN_COUNT] = {0};
	U8 *buffer = data;
	U8 u8ResultBuffer[256] = {0};
	
	ntyKMP(buffer, len, pattern_start, len_start, start_matches);
	ntyKMP(buffer, len, pattern_end, len_end, end_matches);

	if (start_matches[0] == 0) {
		ntylog(" result failed: %s\n", buffer);
		sprintf(u8ResultBuffer, "Set Location LatLonFailed");
		ntyProtoHttpRetProxyTransform(tag->fromId, u8ResultBuffer, strlen(u8ResultBuffer));
#if 0
		free(tag->Tag);
		free(tag);
#endif
		return size*nmemb;
	}

	//ntylog(" result:%s\n", buffer);
	for (i = 0;i < PATTERN_COUNT;i ++) {
		if (start_matches[i] != 0 && start_matches[i] < end_matches[i]) {
			for (j = start_matches[i]+len_start;j < end_matches[i];j ++) {
				location[(k >= LOCATION_INFO_COUNT ? 0 : k++)] = buffer[j];
			}
			//ntylog("location:%s\n", location);

			k = 0;
			while (location[k++] != ',');
				
			memcpy(u8Lon, location, k-1);
			memcpy(u8Lat, location+k, strlen(location)-k);
			
			ntylog("lat:%s, lon:%s\n", u8Lat, u8Lon);
			sprintf(u8ResultBuffer, "Set Location %s:%s:3", u8Lat, u8Lon);
			//ntyProtoHttpProxyTransform(tag->fromId, tag->toId, u8ResultBuffer, strlen(u8ResultBuffer+NTY_PROTO_DATAPACKET_CONTENT_IDX));
			ntylog("ntyHttpMtkQuickLocationHandleResult --> Cmd : %s, length:%ld\n", u8ResultBuffer, strlen(u8ResultBuffer));
			ntyProtoHttpRetProxyTransform(tag->fromId, u8ResultBuffer, strlen(u8ResultBuffer));
			break;
		}
	}
#if 0
	ntySendDeviceRouterInfo(cv, ack, strlen(ack));
#endif
#if 0
	free(tag->Tag);
	free(tag);
#endif
#else
	
#define PATTERN_COUNT 			16
#define DESC_INFO_COUNT			256

	U8 *buffer = data;
	U8 u8Lat[PATTERN_COUNT] = {0};
	U8 u8Lon[PATTERN_COUNT] = {0};
	
	U8 u8Desc[DESC_INFO_COUNT] = {0};
	U8 u8ResultBuffer[DESC_INFO_COUNT] = {0};
	
	int len = size*nmemb;

	if (0 > ntyHttpGaodeGetLocationInfo(buffer, len, u8Lon, u8Lat)) { //no exist 
		ntylog(" result failed: %s\n", buffer);

		sprintf(u8ResultBuffer, "Set Location LatLonFailed");
		ntyProtoHttpRetProxyTransform(tag->fromId, u8ResultBuffer, strlen(u8ResultBuffer));

	} else {
		if (0 > ntyHttpGaodeGetDescInfo(buffer, len, u8Desc)) {
			ntylog(" result failed: %s\n", buffer);
		}
#if 0 //ENABLE_CONNECTION_POOL
		ntyExecuteLocationInsertHandle(tag->fromId, u8Lon, u8Lat, tag->u8LocationType, u8Desc);
#endif
		sprintf(u8ResultBuffer, "Set Location %s:%s:%d", u8Lat, u8Lon, tag->u8LocationType);
		//ntyBoardcastAllFriendsById(tag->fromId, u8ResultBuffer, strlen(u8ResultBuffer));
		ntylog(" fromId:%lld, %s\n", tag->fromId, u8ResultBuffer);
		ntyProtoHttpRetProxyTransform(tag->fromId, u8ResultBuffer, strlen(u8ResultBuffer));
			
	}

#endif



	return size*nmemb;
}


int ntyHttpMtkQuickLocation(void *arg) {
	CURL *curl;	
	CURLcode res;	
	VALUE_TYPE *tag = arg;

	curl = curl_easy_init();	
	if (!curl)	{		
		ntylog("curl init failed\n");		
		return -2;	
	}

	ntylog("GAODE url:%s\n", tag->Tag);

	curl_easy_setopt(curl, CURLOPT_URL, tag->Tag); 
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L); 
#if 1
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
#endif
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ntyHttpMtkQuickLocationHandleResult); 
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, tag); 

	res = curl_easy_perform(curl);	
	if (res != CURLE_OK)	{		
		switch(res)		{			
			case CURLE_UNSUPPORTED_PROTOCOL:				
				ntylog("CURLE_UNSUPPORTED_PROTOCOL\n");			
			case CURLE_COULDNT_CONNECT:				
				ntylog("CURLE_COULDNT_CONNECT\n");			
			case CURLE_HTTP_RETURNED_ERROR:				
				ntylog("CURLE_HTTP_RETURNED_ERROR\n");			
			case CURLE_READ_ERROR:				
				ntylog("CURLE_READ_ERROR\n");			
			default:				
				ntylog("default %d\n",res);		
		}		
		return -3;	
	}	
	
	curl_easy_cleanup(curl);
	
#if 1
#if ENABLE_DAVE_MSGQUEUE_MALLOC
	if (tag->Tag != NULL) {
		free(tag->Tag);
		tag->Tag = NULL;
	}
#endif
	if (tag != NULL) {
		free(tag);
		tag = NULL;
	}
#endif

	return 0;
}

int ntyHttpCurlGlobalInit(void) {
	CURLcode return_code;
	return_code = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != return_code) {
		ntylog("init libcurl failed.\n");		
		return -1;
	}
	return 0;
}


static size_t ntyHttpQJKLocationHandleResult(void* buffer, size_t size, size_t nmemb, void *stream) {
	ntydbg("ntyHttpQJKLocationHandleResult --> length:%ld\n", size*nmemb);

	MessageTag *pMessageTag = (MessageTag *)stream;
	U8 *jsonstring = buffer;

	ntydbg("ntyHttpQJKLocationHandleResult json --> %s\n", jsonstring);
	ntydbg("ntyHttpQJKLocationHandleResult url --> %s\n", pMessageTag->Tag);

	JSON_Value *json = ntyMallocJsonValue(jsonstring);
	AMap *pAMap = malloc(sizeof(AMap));
	ntyJsonAMap(json, pAMap);
	if (pAMap == NULL) {
		return -1;
	}

	ntydbg("==1=================================================================\n");
	
	LocationAck *pLocationAck = malloc(sizeof(LocationAck));
	pLocationAck->results.IMEI = ntyJsonDeviceIMEI(json);
	pLocationAck->results.category = ntyJsonAppCategory(json);
	if (pAMap->result.type != NULL) {
		int nLocationType = atoi(pAMap->result.type);
		char *type = NULL;
		ntyJsonGetLocationType(nLocationType, type);
		pLocationAck->results.type = type;
	}
	pLocationAck->results.location = pAMap->result.location;
	pLocationAck->results.radius = pAMap->result.radius;

	ntydbg("==2=================================================================\n");
	
	char *jsonresult = ntyJsonWriteLocation(pLocationAck);
	free(pAMap);
	free(pLocationAck);

	ntydbg("jsonresult --> %s\n", jsonresult);
	
	int ret = ntySendLocationPushResult(pMessageTag->fromId, jsonresult, strlen(jsonresult));
	if (ret > 0) {
		ntydbg("ntySendLocationPushResult ok\n");
		ret = ntySendLocationBroadCastResult(pMessageTag->fromId, pMessageTag->toId, jsonresult, strlen(jsonresult));
		if (ret < 0) {
			ntylog("ntyHttpQJKLocationHandleResult send error.\n");
		} else {
			ntydbg("ntySendLocationBroadCastResult ok\n");
		}
	}
	
	return 0;
}

int ntyHttpQJKLocation(void *arg) {
	CURL *curl;	
	CURLcode res;

	MessageTag *pMessageTag = (MessageTag *)arg;
	U8 *tag = pMessageTag->Tag;
#if 0
	CURLcode return_code;
	return_code = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != return_code) {
		ntylog("init libcurl failed.\n");		
		return -1;
	}
#endif
	curl = curl_easy_init();
	if (!curl)	{		
		ntylog("curl init failed\n");
		return -2;	
	}

	ntylog("QJK url:%s\n", tag);

	curl_easy_setopt(curl, CURLOPT_URL, tag);
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
#if 1
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
#endif
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ntyHttpQJKLocationHandleResult); 
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, arg); 

	res = curl_easy_perform(curl);	
	if (res != CURLE_OK)	{		
		switch(res)		{
			case CURLE_UNSUPPORTED_PROTOCOL:			
				ntylog("CURLE_UNSUPPORTED_PROTOCOL\n");	
			case CURLE_COULDNT_CONNECT:
				ntylog("CURLE_COULDNT_CONNECT\n");	
			case CURLE_HTTP_RETURNED_ERROR:				
				ntylog("CURLE_HTTP_RETURNED_ERROR\n");			
			case CURLE_READ_ERROR:				
				ntylog("CURLE_READ_ERROR\n");			
			default:				
				ntylog("default %d\n",res);		
		}		
		return -3;	
	}
	
	curl_easy_cleanup(curl);
	
	if (tag != NULL) {
		free(tag);
		tag = NULL;
	}

	return 0;
}



static size_t ntyHttpQJKWeatherLocationHandleResult(void* buffer, size_t size, size_t nmemb, void *stream) {
	ntylog("==================begin ntyHttpQJKWeatherLocationHandleResult ==========================\n");
	ntydbg("ntyHttpQJKWeatherLocationHandleResult --> length:%ld\n", size*nmemb);
	MessageTag *pMessageTag = (MessageTag *)stream;
	U8 *jsonstring = buffer;
	ntydbg("ntyHttpQJKWeatherLocationHandleResult json --> %s\n", jsonstring);
	ntydbg("ntyHttpQJKWeatherLocationHandleResult url --> %s\n", pMessageTag->Tag);
	
	JSON_Value *json = ntyMallocJsonValue(jsonstring);
	AMap *pAMap = (AMap*)malloc(sizeof(AMap));
	ntyJsonAMap(json, pAMap);
	if (strcmp(pAMap->status, "1") != 0) {
		return -1;
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
	sprintf(weatherbuf, "%s/v3/weather/daily.json?key=%s&location=%s&language=zh-Hans&unit=c&start=0&days=3", 
		HTTP_WEATHER_BASE_URL, HTTP_WEATHER_KEY, latlng);
	ntydbg(" weatherbuf --> %s\n", weatherbuf);

	MessageTag *pMessageSendTag = malloc(sizeof(MessageTag));
	pMessageSendTag->fromId = pMessageTag->fromId;
	pMessageSendTag->toId = pMessageTag->toId;
	pMessageSendTag->Tag = weatherbuf;
	pMessageSendTag->length = strlen(weatherbuf);

	int ret = ntyClassifyMessageType(pMessageSendTag->fromId, pMessageSendTag->toId, pMessageSendTag->Tag, pMessageSendTag->length);
	//int ret = ntyHttpQJKWeather(pMessageSendTag);
	ntylog("result : %d\n", ret);
	free(pAMap);
	ntylog("==================end ntyHttpQJKWeatherLocationHandleResult ============================\n");
	return ret;
}

int ntyHttpQJKWeatherLocation(void *arg) {
	CURL *curl;	
	CURLcode res;	
	MessageTag *pMessageTag = (MessageTag *)arg;
	U8 *tag = pMessageTag->Tag;
#if 0
	CURLcode return_code;
	return_code = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != return_code) {
		ntylog("init libcurl failed.\n");		
		return -1;
	}
#endif
	curl = curl_easy_init();
	if (!curl)	{		
		ntylog("curl init failed\n");		
		return -2;	
	}

	ntylog("QJK url:%s\n", tag);

	curl_easy_setopt(curl, CURLOPT_URL, tag); 
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L); 
#if 1
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
#endif
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ntyHttpQJKWeatherLocationHandleResult); 
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, arg); 

	res = curl_easy_perform(curl);	
	if (res != CURLE_OK)	{		
		switch(res)		{			
			case CURLE_UNSUPPORTED_PROTOCOL:				
				ntylog("CURLE_UNSUPPORTED_PROTOCOL\n");			
			case CURLE_COULDNT_CONNECT:				
				ntylog("CURLE_COULDNT_CONNECT\n");			
			case CURLE_HTTP_RETURNED_ERROR:				
				ntylog("CURLE_HTTP_RETURNED_ERROR\n");			
			case CURLE_READ_ERROR:				
				ntylog("CURLE_READ_ERROR\n");			
			default:				
				ntylog("default %d\n",res);		
		}		
		return -3;	
	}	
	
	curl_easy_cleanup(curl);
	
	if (tag != NULL) {
		free(tag);
		tag = NULL;
	}

	return 0;
}


static size_t ntyHttpQJKWeatherHandleResult(void* buffer, size_t size, size_t nmemb, void *stream) {
	ntylog("==================begin ntyHttpQJKWeatherHandleResult ==========================\n");
	ntydbg("ntyHttpQJKWeatherHandleResult --> length:%ld\n", size*nmemb);
	MessageTag *pMessageTag = (MessageTag *)stream;
	U8 *jsonstring = buffer;
	ntydbg("ntyHttpQJKWeatherHandleResult json --> %s\n", jsonstring);
	ntydbg("ntyHttpQJKWeatherHandleResult url --> %s\n", pMessageTag->Tag);
	

	//ntySendWeatherPushResult(C_DEVID fromId,U8 * json,int length);
	
	return 0;
}


int ntyHttpQJKWeather(void *arg) {
	CURL *curl;	
	CURLcode res;	
	MessageTag *pMessageTag = (MessageTag *)arg;
	U8 *tag = pMessageTag->Tag;
#if 0
	CURLcode return_code;
	return_code = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != return_code) {
		ntylog("init libcurl failed.\n");		
		return -1;
	}
#endif
	curl = curl_easy_init();
	if (!curl)	{		
		ntylog("curl init failed\n");		
		return -2;	
	}

	ntylog("QJK url:%s\n", tag);

	curl_easy_setopt(curl, CURLOPT_URL, tag); 
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L); 
#if 1
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
#endif
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ntyHttpQJKWeatherHandleResult); 
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, arg); 

	res = curl_easy_perform(curl);	
	if (res != CURLE_OK)	{		
		switch(res)		{			
			case CURLE_UNSUPPORTED_PROTOCOL:				
				ntylog("CURLE_UNSUPPORTED_PROTOCOL\n");			
			case CURLE_COULDNT_CONNECT:				
				ntylog("CURLE_COULDNT_CONNECT\n");			
			case CURLE_HTTP_RETURNED_ERROR:				
				ntylog("CURLE_HTTP_RETURNED_ERROR\n");			
			case CURLE_READ_ERROR:				
				ntylog("CURLE_READ_ERROR\n");			
			default:				
				ntylog("default %d\n",res);		
		}		
		return -3;	
	}	
	
	curl_easy_cleanup(curl);
	
	if (tag != NULL) {
		free(tag);
		tag = NULL;
	}

	return 0;
}


static size_t ntyHttpQJKCommonHandleResult(void* buffer, size_t size, size_t nmemb, void *stream) {
	VALUE_TYPE *tag = stream;
	U8 u8ResultBuffer[256] = {0};

	ntylog("ntyHttpQJKCommonHandleResult --> length:%ld\n", size*nmemb);
	ntylog("buffer:%s, %ld\n", (char*)buffer, size*nmemb);
	sprintf(u8ResultBuffer, "Set Fall %d", 1);

	if (tag == NULL) goto exit; 
	//strcpy(u8ResultBuffer, "Set Fall 1", 10);
#if 0
	Client *cv = ntyRBTreeInterfaceSearch(tree, tag->fromId);
	if (cv == NULL) { 
		ntylog(" http qjk fallen fromId not exist\n");
		goto exit;
	}
	
	ntySendDeviceRouterInfo(cv, ack, strlen(ack));
#else
#if 0
	ntyProtoHttpProxyTransform(tag->fromId, tag->toId, u8ResultBuffer, strlen(u8ResultBuffer+NTY_PROTO_DATAPACKET_CONTENT_IDX));
#else
	ntyBoardcastAllFriendsById(tag->fromId, u8ResultBuffer, strlen(u8ResultBuffer));
#endif
	ntyProtoHttpRetProxyTransform(tag->toId, u8ResultBuffer, strlen(u8ResultBuffer));
#endif

exit:	


	return size*nmemb;
}


int ntyHttpQJKCommon(void *arg) {
	CURL *curl;	
	CURLcode res;	
	U8 *tag = arg;
#if 0
	CURLcode return_code;
	return_code = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != return_code) {
		ntylog("init libcurl failed.\n");		
		return -1;
	}
#endif
	curl = curl_easy_init();
	if (!curl)	{		
		ntylog("curl init failed\n");		
		return -2;	
	}

	ntylog("QJK url:%s\n", tag);

	curl_easy_setopt(curl, CURLOPT_URL, tag); 
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L); 
#if 1
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
#endif
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ntyHttpQJKCommonHandleResult); 
	//curl_easy_setopt(curl, CURLOPT_WRITEDATA, tag); 

	res = curl_easy_perform(curl);	
	if (res != CURLE_OK)	{		
		switch(res)		{			
			case CURLE_UNSUPPORTED_PROTOCOL:				
				ntylog("CURLE_UNSUPPORTED_PROTOCOL\n");			
			case CURLE_COULDNT_CONNECT:				
				ntylog("CURLE_COULDNT_CONNECT\n");			
			case CURLE_HTTP_RETURNED_ERROR:				
				ntylog("CURLE_HTTP_RETURNED_ERROR\n");			
			case CURLE_READ_ERROR:				
				ntylog("CURLE_READ_ERROR\n");			
			default:				
				ntylog("default %d\n",res);		
		}		
		return -3;	
	}	
	
	curl_easy_cleanup(curl);
	
	if (tag != NULL) {
		free(tag);
		tag = NULL;
	}


	return 0;
}


#if 0
#define QJK_URL "GET http://shangshousoft.applinzi.com/api?m=health&a=falldown&deviceid=123456&platform=4&timestamp=20160809125623&token=d9066e2359acd0e41529482c44de7a39"

int main(void) {
	VALUE_TYPE *tag = (VALUE_TYPE*)malloc(sizeof(VALUE_TYPE));

	tag->Type = MSG_TYPE_QJK_FALLEN;
	tag->fromId = 1;
	tag->length = strlen(QJK_URL);
	memcpy(tag->Tag, QJK_URL, tag->length);

	ntyHttpQJKFallen(tag);

	while(1);
}

#endif


