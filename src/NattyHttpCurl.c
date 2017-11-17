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
#include "NattySqlHandle.h"

#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <wchar.h>

#define JEMALLOC_NO_DEMANGLE 1
#define JEMALLOC_NO_RENAME	 1
#include <jemalloc/jemalloc.h>


nHttpRequest *ntyHttpRequestGetCURL(void);

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
#if 1
	if (tag->Tag != NULL) {
		free(tag->Tag);
		tag->Tag = NULL;
	}

	if (tag != NULL) {
		free(tag);
		tag = NULL;
	}
#endif

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
	
#if 0
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
			if ( lng != NULL ){	
				memcpy(lng, location, k-1);
			}
			if ( lat != NULL ){
				memcpy(lat, location+k, strlen(location)-k);
			}
			
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


	if (tag->Tag != NULL) {
		free(tag->Tag);
		tag->Tag = NULL;
	}

	if (tag != NULL) {
		free(tag);
		tag = NULL;
	}


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
	curl_version_info_data *info=curl_version_info(CURLVERSION_NOW);
	if (info->features & CURL_VERSION_ASYNCHDNS) {
		ntylog("ares enabled\n");
	} else {
		ntylog("ares NOT enabled\n");
	}

	curl = curl_easy_init();	
	if (!curl)	{		
		ntylog("curl init failed\n");		
		return -2;	
	}

	ntylog("GAODE url:%s\n", tag->Tag);

	curl_easy_setopt(curl, CURLOPT_URL, tag->Tag); 
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L); 
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, NTY_CURL_TIMEOUT);
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

	curl_version_info_data *info=curl_version_info(CURLVERSION_NOW);
	if (info->features & CURL_VERSION_ASYNCHDNS) {
		ntylog("ares enabled\n");
	} else {
		ntylog("ares NOT enabled\n");
	}

	curl = curl_easy_init();	
	if (!curl)	{		
		ntylog("curl init failed\n");		
		return -2;	
	}

	ntylog("GAODE url:%s\n", tag->Tag);

	curl_easy_setopt(curl, CURLOPT_URL, tag->Tag); 
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L); 
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, NTY_CURL_TIMEOUT);
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

static nRequestMotor *nReqMoter = NULL;

nHttpRequest *ntyHttpRequestGetCURL(void) {
	int i = 0;
	nHttpRequest *req = NULL;

	if (nReqMoter == NULL) return NULL;

	for (i = 0;i < MAX_HTTTP_REQUEST_COUNT;i ++) {
		if (nReqMoter->request[i].enable == 1) {
			if(0 == cmpxchg(&nReqMoter->request[i].req_lock, 0, 1, WORD_WIDTH)) {

				req = nReqMoter->request+i;

				nReqMoter->request[i].enable = 0;
				nReqMoter->request[i].req_lock = 0;
			}
		}
	}

	return req;
}

int ntyHttpRequestResetCURL(nHttpRequest *req) {
	int i = 0;

	if (req == NULL) return NTY_RESULT_ERROR;
	if (nReqMoter == NULL) return NTY_RESULT_ERROR;

	ntylog(" ntyHttpRequestResetCURL --> index:%d\n", req->index);

	if(0 == cmpxchg(&nReqMoter->request[req->index].req_lock, 0, 1, WORD_WIDTH)) {

		nReqMoter->request[req->index].enable = 1;
		nReqMoter->request[req->index].req_lock = 0;
	}

	return NTY_RESULT_SUCCESS;
}



int ntyHttpCurlGlobalInit(void) {
	int i = 0;
	CURLcode return_code;
	
	return_code = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != return_code) {
		ntylog("init libcurl failed.\n");		
		return -1;
	}

	nReqMoter = (nRequestMotor*)malloc(sizeof(nRequestMotor));
	if (nReqMoter == NULL) {
		return NTY_RESULT_ERROR;
	}
	for (i = 0;i < MAX_HTTTP_REQUEST_COUNT;i ++) {
		nReqMoter->request[i].curl = curl_easy_init();
		nReqMoter->request[i].arg = NULL;
		nReqMoter->request[i].index = i;
		nReqMoter->request[i].enable = 1; //1:enable,0:disable
		nReqMoter->request[i].req_lock = 0;
	}
	
	
	return NTY_RESULT_ERROR;
}

int ntyHttpCurlDeInit(void) {
	int i = 0;

	if (nReqMoter == NULL) return NTY_RESULT_ERROR;

	for (i = 0;i < MAX_HTTTP_REQUEST_COUNT;i ++) {
		
		curl_easy_cleanup(nReqMoter->request[i].curl);
	}

	return NTY_RESULT_SUCCESS;
}

static size_t ntyHttpQJKLocationGetAddressHandleResult(void* buffer, size_t size, size_t nmemb, void *stream) {

	U8 *data = (U8*)stream;
	int nSize = size*nmemb;
	ntylog("ntyHttpQJKLocationGetAddressHandleResult size*nmemb:%d, data:%s\n", nSize, data);
	
	strncat(data, buffer, nSize);
	
	
	return size * nmemb;
}


int ntyQJKLocationGetAddressHandle(U8 *jsonstring, MessageTag *pMessageTag) {
	if (jsonstring == NULL || pMessageTag == NULL) return NTY_RESULT_ERROR;
	
	JSON_Value *json = ntyMallocJsonValue( jsonstring );
	AMap *pAMap = (AMap *)malloc( sizeof(AMap) );
	if ( pAMap == NULL || json == NULL ) {
		ntylog("ntyQJKLocationGetAddressHandle --> %s:%d\n", __FILE__, __LINE__);
		return NTY_RESULT_FAILED;
	}
	memset( pAMap, 0, sizeof(AMap) );
	ntyJsonAMapGetAddress( json, pAMap );
	
	//the data of location should be match the rule.
	if ( pAMap->result.location==NULL || pAMap->result.desc==NULL ){
		free( pAMap );
		ntylog("ntyQJKLocationGetAddressHandle location is NULL\n");
		return NTY_RESULT_FAILED;
	}

	size_t len_LocationAck = sizeof(LocationAck);
	LocationAck *pLocationAck = malloc(len_LocationAck);
	if (pLocationAck == NULL) {
		
		free(pAMap);
		
		return NTY_RESULT_FAILED;
	}
	memset(pLocationAck, 0, len_LocationAck);
	
	char bufIMEI[50] = {0};
	sprintf(bufIMEI, "%llx", pMessageTag->fromId);
	ntylog("*****ntyQJKLocationGetAddressHandle IMEI:%s,fromId:%lld,toId:%lld\n", bufIMEI, pMessageTag->fromId, pMessageTag->toId);
	pLocationAck->results.IMEI = bufIMEI;

	U8 tb_location_type = 0; //the value of tb_location_type insert tb_location table
	if (pMessageTag->Type== MSG_TYPE_LOCATION_WIFI_API) {
		pLocationAck->results.category = NATTY_USER_PROTOCOL_WIFI;
		pLocationAck->results.type = NATTY_USER_PROTOCOL_WIFI;
		tb_location_type = 1;
	}else if (pMessageTag->Type == MSG_TYPE_LOCATION_GPS_API) {
		pLocationAck->results.category = NATTY_USER_PROTOCOL_GPS;
		pLocationAck->results.type = NATTY_USER_PROTOCOL_GPS;
		tb_location_type = 2;
	}else if (pMessageTag->Type == MSG_TYPE_LOCATION_LAB_API) {
		pLocationAck->results.category = NATTY_USER_PROTOCOL_LAB;
		pLocationAck->results.type = NATTY_USER_PROTOCOL_LAB;
		tb_location_type = 3;
	}
	
	pLocationAck->results.location = pAMap->result.location;
	pLocationAck->results.radius = pAMap->result.radius;
	U32 msgid = 0;
	
	U32 length = strlen(pAMap->result.desc);	
	U8 *urlCode = (U8 *)malloc( length*3+1 ); //becourse of the function ntyUrlEncode.
	if ( urlCode == NULL ){
		ntylog( "ntyQJKLocationGetAddressHandle jsonstring malloc failed\n");
		
		free(pAMap);
		
		return NTY_RESULT_FAILED;
	}
	int res = ntyUrlEncode(pAMap->result.desc, urlCode, length);  //strlen(urlCode) = length * 3 +1
	ntylog("ntyQJKLocationGetAddressHandle urlCode : %s\n", urlCode);
	int ret = ntyExecuteLocationReportInsertHandle( pMessageTag->toId, tb_location_type, urlCode, pAMap->result.location, pAMap->result.radius, &msgid );
	if (ret == 0) {
		char *jsonresult = ntyJsonWriteLocation(pLocationAck);
		ntylog("******ntyQJKLocationGetAddressHandle broadcast to app json:%s\n", jsonresult);
#if 0
			
			ntySendLocationBroadCastResult(pMessageTag->fromId, pMessageTag->toId, jsonresult, strlen(jsonresult));
#else
		size_t len_ValueType = sizeof(VALUE_TYPE);
		VALUE_TYPE *tag = (VALUE_TYPE*)malloc(len_ValueType);
		if (tag == NULL) {
			goto exit;
		}
		memset(tag, 0, len_ValueType);

		tag->fromId = pMessageTag->fromId;
		tag->gId = pMessageTag->toId;
		if ( jsonresult != NULL ){
			tag->length = strlen(jsonresult);
		}
		tag->Tag = malloc(tag->length+1);
		memset(tag->Tag, 0, tag->length+1);
		memcpy(tag->Tag, jsonresult, tag->length);

		tag->Type = MSG_TYPE_LOCATION_BROADCAST_HANDLE;
		tag->cb = ntyLocationBroadCastHandle;
		ntyDaveMqPushMessage(tag);

#endif
	}

exit:
	free(pLocationAck);
	free(pAMap);
	free( urlCode );

	return NTY_RESULT_SUCCESS;
}

int ntyHttpQJKLocationGetAddress(void *arg) {
	ntylog("**********ntyHttpQJKLocationGetAddress begin\n");
	CURL *curl;	
	CURLcode res;

	MessageTag *pMessageTag = (MessageTag *)arg;
	if (pMessageTag == NULL) return NTY_RESULT_ERROR;
	ntylog("ntyHttpQJKLocationGetAddress --> fromId:%llx, toId:%llx\n", pMessageTag->fromId, pMessageTag->toId);
	
	U8 *tag = pMessageTag->Tag;
#if 0
	CURLcode return_code;
	return_code = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != return_code) {
		ntylog("init libcurl failed.\n");		
		return -1;
	}
#endif
	curl_version_info_data *info=curl_version_info(CURLVERSION_NOW);
	if (info->features & CURL_VERSION_ASYNCHDNS) {
		ntylog("ares enabled\n");
	} else {
		ntylog("ares NOT enabled\n");
	}
#if 0
	curl = curl_easy_init();
#else
	nHttpRequest *req = ntyHttpRequestGetCURL();
	if (req == NULL) { 

		if (pMessageTag->Tag != NULL) {
			free(pMessageTag->Tag);
		}
		free(pMessageTag);
		
		return NTY_RESULT_ERROR;
	}
	curl = req->curl;
	//pMessageTag->param = req;
#endif
	if (!curl)	{		
		ntylog("curl init failed\n");

		if (pMessageTag->Tag != NULL) {
			free(pMessageTag->Tag);
		}
		free(pMessageTag);
		
		return NTY_RESULT_ERROR;	
	}

	ntylog("get address QJK url:%s\n", tag);

	U8 *data = (U8*)malloc(2*1024);
	memset(data, 0, 2*1024);

	curl_easy_setopt(curl, CURLOPT_URL, tag);
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, NTY_CURL_TIMEOUT);
#if 1
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
#endif
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ntyHttpQJKLocationGetAddressHandleResult); 
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, data); 

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
		//return -3;	
	} else {
		ntyQJKLocationGetAddressHandle(data, pMessageTag);
	}

#if 0	
	curl_easy_cleanup(curl);
#endif
	ntyHttpRequestResetCURL(req);
#if 1
	if (pMessageTag->Tag != NULL) {
		free(pMessageTag->Tag);
	}
	free(pMessageTag);
	
	free(data);
#endif
	return 0;
}



static size_t ntyHttpQJKLocationHandleResult(void* buffer, size_t size, size_t nmemb, void *stream) {
	int ret = 0;
	MessageTag *pMessageTag = (MessageTag *)stream;
	if (pMessageTag == NULL) return NTY_RESULT_PROCESS;

	U32 nSize = size*nmemb;
	ntylog("ntyHttpQJKLocationHandleResult size*nmemb:%d\n",nSize);
	U8 *jsonstring = (U8 *)malloc( nSize + 1 );
	if( jsonstring == NULL ){
		free( pMessageTag );
		ntylog("ntyHttpQJKLocationHandleResult jsonstring malloc failed.\n");
		return size*nmemb;
	}
	memset( jsonstring, 0, nSize+1 );
	memcpy( jsonstring, buffer, nSize );
		
	ntylog("ntyHttpQJKLocationHandleResult json --> %s\n", jsonstring);
	if (pMessageTag->Tag != NULL) {
		ntylog("ntyHttpQJKLocationHandleResult url --> %s\n", pMessageTag->Tag);
	}

	JSON_Value *json = ntyMallocJsonValue(jsonstring);
	size_t len_AMap = sizeof(AMap);
	AMap *pAMap = malloc(len_AMap);
	if (pAMap == NULL) {
		if (pMessageTag->Tag != NULL) {
			free(pMessageTag->Tag);
		}
		free(pMessageTag);
		return size * nmemb;
	}
	memset(pAMap, 0, len_AMap);
	ntyJsonAMap(json, pAMap);
	
	//the data of location should be match the rule.
	if ( pAMap->result.location==NULL || pAMap->result.desc==NULL ){
		free( pAMap );
		ntylog("ntyHttpQJKLocationHandleResult location is NULL\n");
		return size * nmemb;
	}
	
	//the data of location should be match the rule.
	if ( pAMap->result.location==NULL || pAMap->result.desc==NULL ){
		free( pMessageTag );
		free( pAMap );
		ntylog("ntyHttpQJKLocationHandleResult location is NULL\n");
		return size * nmemb;
	}	

	size_t len_LocationAck = sizeof(LocationAck);
	LocationAck *pLocationAck = malloc(len_LocationAck);
	if (pLocationAck == NULL) {
		if (pMessageTag->Tag != NULL) {
			free(pMessageTag->Tag);
		}
		free(pMessageTag);
		free(pAMap);
		return size * nmemb;
	}
	memset(pLocationAck, 0, len_LocationAck);
	
	char bufIMEI[50] = {0};
	sprintf(bufIMEI, "%llx", pMessageTag->fromId);
	ntylog("ntyHttpQJKLocationHandleResult bufIMEI:%s,fromId:%lld,toId:%lld\n", bufIMEI, pMessageTag->fromId, pMessageTag->toId);
	pLocationAck->results.IMEI = bufIMEI;

	U8 tb_location_type = 0;	
	if (pMessageTag->Type== MSG_TYPE_LOCATION_WIFI_API) {
		pLocationAck->results.category = NATTY_USER_PROTOCOL_LOCATION;
		pLocationAck->results.type = NATTY_USER_PROTOCOL_WIFI;
		tb_location_type = 1;
	} else if (pMessageTag->Type == MSG_TYPE_LOCATION_LAB_API) {
		pLocationAck->results.category = NATTY_USER_PROTOCOL_LOCATION;
		pLocationAck->results.type = NATTY_USER_PROTOCOL_LAB;
		tb_location_type = 3;
	}
	
	pLocationAck->results.location = pAMap->result.location;
	pLocationAck->results.radius = pAMap->result.radius;

	U32 nLength = strlen( pAMap->result.desc );
	U8 *urlCode = (U8 *)malloc( nLength*3+1 );  //becourse of the function ntyUrlEncode.
	if ( urlCode == NULL ){
		free( pMessageTag );
		free( pAMap );
		free( jsonstring );
		free( pLocationAck );
		return size*nmemb;
	}
	int res = ntyUrlEncode( pAMap->result.desc, urlCode, nLength );  //strlen(urlCode) = nLength * 3 +1
	ntylog("urlCode : %s\n", urlCode);

	//int ret = ntyExecuteLocationNewInsertHandle(pMessageTag->toId, tb_location_type, pAMap->result.location, urlCode, pAMap->result.radius);
	//if (ret == 0) {
		char *jsonresult = ntyJsonWriteLocation(pLocationAck);
		ntylog("ntyHttpQJKLocationHandleResult jsonresult --> %s\n", jsonresult);
		ntyJsonCommonContextResult( NATTY_USER_PROTOCOL_TOMCAT_CLIENTID, jsonresult );
		if ( jsonresult != NULL ){
			ret = ntySendLocationPushResult(pMessageTag->toId, jsonresult, strlen(jsonresult));
		}
		if (ret > 0) {
#if 0			
			ntySendLocationBroadCastResult(pMessageTag->fromId, pMessageTag->toId, jsonresult, strlen(jsonresult));
#else
			size_t len_ValueType = sizeof(VALUE_TYPE);
			VALUE_TYPE *tag = (VALUE_TYPE*)malloc(len_ValueType);
			if (tag == NULL) {
				goto exit;
			}
			memset(tag, 0, len_ValueType);

			tag->fromId = pMessageTag->fromId;
			tag->gId = pMessageTag->toId;
			if ( jsonresult != NULL ){
				tag->length = strlen(jsonresult);
			}
			tag->Tag = malloc(tag->length+1);
			memset(tag->Tag, 0, tag->length+1);
			memcpy(tag->Tag, jsonresult, tag->length);

			tag->Type = MSG_TYPE_LOCATION_BROADCAST_HANDLE;
			tag->cb = ntyLocationBroadCastHandle;
			ntyDaveMqPushMessage(tag);

#endif
		}
	//}

exit:
	free(pLocationAck);
	free(pAMap);
	free( jsonstring );
	free( urlCode );
#if 1 //Release Message
	//nHttpRequest *req = (nHttpRequest *)pMessageTag->param;
	

	if (pMessageTag->Tag != NULL) {
		free(pMessageTag->Tag);
	}
	free(pMessageTag);
#endif
	
	return size * nmemb;
}



int ntyHttpQJKLocation(void *arg) {
	CURL *curl;	
	CURLcode res;

	MessageTag *pMessageTag = (MessageTag *)arg;
	if (pMessageTag == NULL) return NTY_RESULT_ERROR;
	
	U8 *tag = pMessageTag->Tag;
#if 0
	CURLcode return_code;
	return_code = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != return_code) {
		ntylog("init libcurl failed.\n");		
		return -1;
	}
#endif
	curl_version_info_data *info=curl_version_info(CURLVERSION_NOW);
	if (info->features & CURL_VERSION_ASYNCHDNS) {
		ntylog("ares enabled\n");
	} else {
		ntylog("ares NOT enabled\n");
	}
#if 0
	curl = curl_easy_init();
#else
	nHttpRequest *req = ntyHttpRequestGetCURL();
	if (req == NULL) { 

		if (pMessageTag->Tag != NULL) {
			free(pMessageTag->Tag);
		}
		free(pMessageTag);
		
		return NTY_RESULT_ERROR;
	}
	curl = req->curl;
	//pMessageTag->param = req;
#endif
	if (!curl)	{		
		ntylog("curl init failed\n");

		if (pMessageTag->Tag != NULL) {
			free(pMessageTag->Tag);
		}
		free(pMessageTag);
		
		return NTY_RESULT_ERROR;	
	}

	ntylog("QJK url:%s\n", tag);

	curl_easy_setopt(curl, CURLOPT_URL, tag);
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, NTY_CURL_TIMEOUT);
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
		//return -3;	
	}
#if 0	
	curl_easy_cleanup(curl);
#endif
	ntyHttpRequestResetCURL(req);
#if 0	
	if (tag != NULL) {
		free(tag);
		tag = NULL;
	}
#endif
	return 0;
}







static size_t ntyHttpQJKFalldownHandleResult(void* buffer, size_t size, size_t nmemb, void *stream) {
	MessageTag *pMessageTag = (MessageTag *)stream;
	if (pMessageTag == NULL) return NTY_RESULT_PROCESS;

	U32 nSize = size*nmemb;
	ntylog("ntyHttpQJKFalldownHandleResult size*nmemb:%d\n",nSize);
	U8 *jsonstring = (U8 *)malloc( nSize + 1 );
	if( jsonstring == NULL ){
		free( pMessageTag );
		ntylog("ntyHttpQJKFalldownHandleResult jsonstring malloc failed.\n");
		return size*nmemb;
	}
	memset( jsonstring, 0, nSize+1 );
	memcpy( jsonstring, buffer, nSize );
		
	ntylog("ntyHttpQJKFalldownHandleResult json --> %s\n", jsonstring);
	if (pMessageTag->Tag != NULL) {
		ntylog("ntyHttpQJKFalldownHandleResult url --> %s\n", pMessageTag->Tag);
	}

	JSON_Value *json = ntyMallocJsonValue(jsonstring);
	size_t len_CommonResponse = sizeof(CommonResponse);
	CommonResponse *pCommonResponse = malloc(len_CommonResponse);
	if (pCommonResponse == NULL) {
		if (pMessageTag->Tag != NULL) {
			free(pMessageTag->Tag);
		}
		free(pMessageTag);
		return size * nmemb;
	}
	memset(pCommonResponse, 0, len_CommonResponse);
	ntyJsonCommonResponse(json, pCommonResponse);
	
	//the data of location should be match the rule.
	if ( pCommonResponse->code==NULL){
		free( pCommonResponse );
		ntylog("ntyHttpQJKFalldownHandleResult location is NULL\n");
		return size * nmemb;
	}

	if (strcmp(pCommonResponse->code, "200") == 0) {
		//success
	} else {
		ntylog("Falldown failed\n");
	}
	
	/*
	size_t len_FalldownAck = sizeof(FalldownAck);
	FalldownAck *pFalldownAck = malloc(len_FalldownAck);
	if (pFalldownAck == NULL) {
		if (pMessageTag->Tag != NULL) {
			free(pMessageTag->Tag);
		}
		free(pMessageTag);
		free(pCommonResponse);
		return size * nmemb;
	}
	memset(pFalldownAck, 0, len_FalldownAck);
	
	char bufIMEI[50] = {0};
	sprintf(bufIMEI, "%llx", pMessageTag->fromId);
	ntydbg("bufIMEI %s %lld %lld\n", bufIMEI, pMessageTag->fromId, pMessageTag->toId);
	pFalldownAck->results.IMEI = bufIMEI;

	pFalldownAck->results.category = NATTY_USER_PROTOCOL_FALLDOWNREPORT;
	pFalldownAck->results.falldownReport.radius = "500";
	pFalldownAck->results.falldownReport.longitude = "dd";
	pFalldownAck->results.falldownReport.latitude = "adasdf";
	pFalldownAck->results.falldownReport.type = "1";
	free(pFalldownAck);
	*/
	
exit:
	free(pCommonResponse);
	free(jsonstring);
#if 1 //Release Message
	//nHttpRequest *req = (nHttpRequest *)pMessageTag->param;
	
	if (pMessageTag->Tag != NULL) {
		free(pMessageTag->Tag);
	}
	free(pMessageTag);
#endif
	
	return size * nmemb;
}







int ntyHttpQJKFalldown(void *arg) {
	CURL *curl;	
	CURLcode res;

	MessageTag *pMessageTag = (MessageTag *)arg;
	if (pMessageTag == NULL) return NTY_RESULT_ERROR;
	
	U8 *tag = pMessageTag->Tag;
#if 0
	CURLcode return_code;
	return_code = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != return_code) {
		ntylog("init libcurl failed.\n");		
		return -1;
	}
#endif
	curl_version_info_data *info=curl_version_info(CURLVERSION_NOW);
	if (info->features & CURL_VERSION_ASYNCHDNS) {
		ntylog("ares enabled\n");
	} else {
		ntylog("ares NOT enabled\n");
	}
#if 0
	curl = curl_easy_init();
#else
	nHttpRequest *req = ntyHttpRequestGetCURL();
	if (req == NULL) { 

		if (pMessageTag->Tag != NULL) {
			free(pMessageTag->Tag);
		}
		free(pMessageTag);
		
		return NTY_RESULT_ERROR;
	}
	curl = req->curl;
	//pMessageTag->param = req;
#endif
	if (!curl)	{		
		ntylog("curl init failed\n");

		if (pMessageTag->Tag != NULL) {
			free(pMessageTag->Tag);
		}
		free(pMessageTag);
		
		return NTY_RESULT_ERROR;	
	}

	ntylog("QJK url:%s\n", tag);

	curl_easy_setopt(curl, CURLOPT_URL, tag);
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, NTY_CURL_TIMEOUT);
#if 1
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
#endif
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ntyHttpQJKFalldownHandleResult); 
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
		//return -3;	
	}
#if 0	
	curl_easy_cleanup(curl);
#endif
	ntyHttpRequestResetCURL(req);
#if 0	
	if (tag != NULL) {
		free(tag);
		tag = NULL;
	}
#endif
	return 0;
}





#if 0
int ntyStringReplace(char res[], char from[], char to[]) {
    int i,flag = 0;
    char *p,*q,*ts;
    for(i = 0; res[i]; ++i) {
        if(res[i] == from[0]) {
            p = res + i;
            q = from;
            while(*q && (*p++ == *q++));
            if(*q == '\0') {
                ts = (char *)malloc(strlen(res) + 1);
                strcpy(ts,p);
                res[i] = '\0';
                strcat(res,to);
                strcat(res,ts);
                free(ts);
                flag = 1;
            }
        }
    }
    return flag;
}
#endif

static size_t ntyHttpQJKWeatherLocationHandleResult(void* buffer, size_t size, size_t nmemb, void *stream) {
	int ret = 0;
	ntylog("==================begin ntyHttpQJKWeatherLocationHandleResult ==========================\n");
	MessageTag *pMessageTag = (MessageTag *)stream;
	if (pMessageTag == NULL) return NTY_RESULT_ERROR;
	
	U8 *jsonstring = buffer;
	ntylog("ntyHttpQJKWeatherLocationHandleResult json:%s, url:%s\n", jsonstring,pMessageTag->Tag);
	
	JSON_Value *json = ntyMallocJsonValue(jsonstring);
	size_t len_AMap = sizeof(AMap);
	AMap *pAMap = (AMap*)malloc(len_AMap);
	if (pAMap == NULL) {
		
		if (pMessageTag->Tag != NULL) {
			free(pMessageTag->Tag);
		}
		free(pMessageTag);
		
		return size * nmemb;
	}
	memset(pAMap, 0, len_AMap);
	
	ntyJsonAMap(json, pAMap);
	if (strcmp(pAMap->status, "1") != 0) {
		ntylog(" status is %s\n", pAMap->status);
		ret = -1;
		goto exit;
	}
	if (pAMap->result.location == NULL) {
		ntylog(" location is null\n");
		ret = -2;
		goto exit;
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
	} else {
		ntylog(" location data format error\n");
		ret = -3;
		goto exit;
	}
	char latlng[100] = {0};
	strncat(latlng, lat, (size_t)strlen(lat));
	strncat(latlng, colon, (size_t)strlen(colon));
	strncat(latlng, lng, (size_t)strlen(lng));

#if 0 //Update By WangBoJing
	LocationAck *pLocationAck = malloc(sizeof(LocationAck));
	if (pLocationAck == NULL) {
		
		if (pMessageTag->Tag != NULL) {
			free(pMessageTag->Tag);
		}
		free(pMessageTag);
	
		goto exit;
	}
	memset(pLocationAck, 0, sizeof(LocationAck));

	char bufIMEI[50] = {0};
	sprintf(bufIMEI, "%llx", pMessageTag->fromId);
	pLocationAck->results.IMEI = bufIMEI;

	if (pMessageTag->Type== MSG_TYPE_LOCATION_WIFI_API) {
		pLocationAck->results.category = NATTY_USER_PROTOCOL_WIFI;
		pLocationAck->results.type = NATTY_USER_PROTOCOL_WIFI;
	} else if (pMessageTag->Type == MSG_TYPE_LOCATION_LAB_API) {
		pLocationAck->results.category = NATTY_USER_PROTOCOL_LAB;
		pLocationAck->results.type = NATTY_USER_PROTOCOL_LAB;
	} else if (pMessageTag->Type == MSG_TYPE_WEATHER_API) {
		pLocationAck->results.category = NATTY_USER_PROTOCOL_LAB;
		pLocationAck->results.type = NATTY_USER_PROTOCOL_LAB;
	}
	
	pLocationAck->results.location = pAMap->result.location;
	pLocationAck->results.radius = pAMap->result.radius;


	ret = ntyExecuteLocationNewInsertHandle(pMessageTag->toId, (U8)pMessageTag->Type, pAMap->result.location, pAMap->info, pAMap->result.desc);
	if (ret == 0) {
		char *jsonresult = ntyJsonWriteLocation(pLocationAck);
		ntylog("ntyHttpQJKLocationHandleResult jsonresult --> %s\n", jsonresult);
#if 1 //cancel return to watchid
		ret = ntySendLocationPushResult(pMessageTag->toId, jsonresult, strlen(jsonresult));
		if (ret < 0) {
			//ntySendLocationBroadCastResult(pMessageTag->fromId, pMessageTag->toId, jsonresult, strlen(jsonresult));
			ntylog("ntySendLocationPushResult --> send failed n");
		}
#else
		ntySendLocationBroadCastResult(pMessageTag->fromId, pMessageTag->toId, jsonresult, strlen(jsonresult));
#endif
	}
	free(pLocationAck);
#endif

	U8 weatherbuf[500] = {0};
	sprintf(weatherbuf, "%s/v3/weather/daily.json?key=%s&location=%s&language=zh-Hans&unit=c&start=0&days=2", 
		HTTP_WEATHER_BASE_URL, HTTP_WEATHER_KEY, latlng);
	ntylog(" ntyHttpQJKWeatherLocationHandleResult weatherbuf:%s\n", weatherbuf);
	int length = strlen(weatherbuf);

	size_t len_MessageTag = sizeof(MessageTag);
	MessageTag *pMessageSendTag = malloc(len_MessageTag);
	if (pMessageSendTag == NULL) {

		if (pMessageTag->Tag != NULL) {
			free(pMessageTag->Tag);
		}
		free(pMessageTag);
		
		goto exit;
	}
	memset(pMessageSendTag, 0, len_MessageTag);
	
	pMessageSendTag->Type = MSG_TYPE_WEATHER_API;
	pMessageSendTag->fromId = pMessageTag->fromId;
	pMessageSendTag->toId = pMessageTag->toId;
	pMessageSendTag->length = length;

#if ENABLE_DAVE_MSGQUEUE_MALLOC
	pMessageSendTag->Tag = malloc((length+1)*sizeof(U8));
	if (pMessageSendTag->Tag == NULL) {
		free(pMessageSendTag);
		
		if (pMessageTag->Tag != NULL) {
			free(pMessageTag->Tag);
		}
		free(pMessageTag);

		goto exit;
	}

	memset(pMessageSendTag->Tag, 0, length+1);
	memcpy(pMessageSendTag->Tag, weatherbuf, length);
#else
	memset(pMessageSendTag->Tag, 0, length+1);
	memcpy(pMessageSendTag->Tag, weatherbuf, length);
#endif

	pMessageSendTag->cb = ntyHttpQJKWeather;
	ret = ntyDaveMqPushMessage(pMessageSendTag);
	ntylog("==================end ntyHttpQJKWeatherLocationHandleResult ============================\n");

#if 0 //Release Message

	if (pMessageTag->Tag != NULL) {
		free(pMessageTag->Tag);
	}
	free(pMessageTag);
#endif

exit:
	free(pAMap);
	
	return size * nmemb;
}

int ntyHttpQJKWeatherLocation(void *arg) {
	CURL *curl;	
	CURLcode res;	
	MessageTag *pMessageTag = (MessageTag *)arg;
	if (pMessageTag == NULL) return NTY_RESULT_ERROR;
	
	U8 *tag = pMessageTag->Tag;
#if 0
	CURLcode return_code;
	return_code = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != return_code) {
		ntylog("init libcurl failed.\n");		
		return -1;
	}
#endif
	curl_version_info_data *info=curl_version_info(CURLVERSION_NOW);
	if (info->features & CURL_VERSION_ASYNCHDNS) {
		ntylog("ares enabled\n");
	} else {
		ntylog("ares NOT enabled\n");
	}
#if 0
	curl = curl_easy_init();
#else
	nHttpRequest *req = ntyHttpRequestGetCURL();
	if (req == NULL) { 

		if (pMessageTag->Tag != NULL) {
			free(pMessageTag->Tag);
		}
		free(pMessageTag);
		
		return NTY_RESULT_ERROR;
	}

	curl = req->curl;
	//pMessageTag->param = req;
#endif
	if (!curl)	{
		ntylog("curl init failed\n");	
		if (pMessageTag->Tag != NULL) {
			free(pMessageTag->Tag);
		}
		free(pMessageTag);
		
		return NTY_RESULT_ERROR;
	}

	ntylog("QJK url:%s\n", tag);

	curl_easy_setopt(curl, CURLOPT_URL, tag);
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, NTY_CURL_TIMEOUT);
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
		//return -3;	
	}	
#if 0
	curl_easy_cleanup(curl);
#endif
	//nHttpRequest *req = (nHttpRequest *)pMessageTag->param;
	ntyHttpRequestResetCURL(req);
#if 0	
	if (tag != NULL) {
		free(tag);
		tag = NULL;
	}
#endif
	return 0;
}


static size_t ntyHttpQJKWeatherHandleResult(void* buffer, size_t size, size_t nmemb, void *stream) {
	int flag = 0;
	ntylog("==================begin ntyHttpQJKWeatherHandleResult ==========================\n");
	MessageTag *pMessageTag = (MessageTag *)stream;
	if (pMessageTag == NULL){
		ntylog( "ntyHttpQJKWeatherHandleResult pMessageTag==NULL return -2.............\n" );
		return NTY_RESULT_ERROR;
	}
	
	U8 *jsonstring = buffer;
	ntylog( "ntyHttpQJKWeatherHandleResult.............\n" );
	ntylog("ntyHttpQJKWeatherHandleResult json:%s, url:%s\n", jsonstring,pMessageTag->Tag );
	JSON_Value *json = ntyMallocJsonValue(jsonstring);
	size_t len_WeatherReq = sizeof(WeatherReq);
	ntylog( "ntyHttpQJKWeatherHandleResult len_WeatherReq:%d\n",len_WeatherReq );
	WeatherReq *pWeatherReq = malloc(len_WeatherReq);
	if (pWeatherReq == NULL) {
		flag = -1;
		goto exit;
	}
	memset(pWeatherReq, 0, len_WeatherReq);
	
	ntyJsonWeather(json, pWeatherReq);
	WeatherAck *pWeatherAck = (WeatherAck*)pWeatherReq;
	/*
	size_t i,j;
	for (i=0; i<pWeatherAck->size; i++) {
		WeatherResults *results = &(pWeatherAck->pResults[i]);
		for (j=0; j<results->size; j++) {
			//results.pDaily[j].date = NULL;
			//results.pDaily[j].text_day = NULL;
			//results.pDaily[j].code_day = NULL;
			//results.pDaily[j].text_night = NULL;
			//results.pDaily[j].code_night = NULL;
			//results.pDaily[j].high = NULL;
			//results.pDaily[j].low = NULL;
			results->pDaily[j].precip = NULL;
			results->pDaily[j].wind_direction = NULL;
			results->pDaily[j].wind_direction_degree = NULL;
			results->pDaily[j].wind_speed = NULL;
			results->pDaily[j].wind_scale = NULL;
		}
	}
	*/
	char *jsonresult = ntyJsonWriteWeather(pWeatherAck);
	if (jsonresult == NULL) {
		flag = -1;
		goto exit;
	}
	int ret = ntySendWeatherPushResult(pMessageTag->fromId, jsonresult, strlen(jsonresult));
	ntylog("ntyHttpQJKWeatherHandleResult --> ret:%d, json:%s\n", ret, jsonresult);
	if (ret < 0) {
		ntylog("ntySendWeatherBroadCastResult ntySendWeatherPushResult failed\n");
	}

exit:
	ntyJsonWeatherRelease(pWeatherReq);
	free(pWeatherReq);
	
#if 1 //Release Message
	
	free(pMessageTag->Tag);
	free(pMessageTag);
#endif
	
	return size * nmemb;
}

/*
	
//* curl 获取 https 请求 
//* @param String $url		请求的url 
//* @param Array	$data		要發送的數據 
//* @param Array	$header 	请求时发送的header 
//* @param int	$timeout	超时时间，默认30s 
int ntyCurlHttps(const char *url, const char *data, const char *$header, int timeout) {
	CURL *curl;	
	curl = curl_easy_init();
 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0); // 跳过证书检查  
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1);  // 从证书中检查SSL加密算法是否存在  
    curl_easy_setopt(curl, CURLOPT_URL, url);  
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, $header);  
    curl_easy_setopt(curl, CURLOPT_POST, 1);  
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, http_build_query(data));  
    curl_easy_setopt(curl, CURLOPT_RETURNTRANSFER, 1);   
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);  
  
    int response = curl_easy_exec(curl);  
  
    if($error=curl_error(curl)){  
        die($error);  
    }  
  
    curl_close(curl);  
  
    return response;  
}
 
int ntyCurlHttpsExec() {
	// 调用  
	char *url = "https://www.example.com/api/message.php"; 
	char *data = array('name'=>'fdipzone');  
	char *header = array();  
  
	int response = curl_https(url, data, header, 5);
	ntydbg("");
}
		
#endif
*/

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
	curl_version_info_data *info=curl_version_info(CURLVERSION_NOW);
	if (info->features & CURL_VERSION_ASYNCHDNS) {
		ntylog("ares enabled\n");
	} else {
		ntylog("ares NOT enabled\n");
	}
#if 0
	curl = curl_easy_init();
#else
	nHttpRequest *req = ntyHttpRequestGetCURL();
	if (req == NULL) { 

		if (pMessageTag->Tag != NULL) {
			free(pMessageTag->Tag);
		}
		free(pMessageTag);
		
		return NTY_RESULT_ERROR;
	}

	curl = req->curl;
	//pMessageTag->param = req;
#endif
	if (!curl)	{		
		ntylog("curl init failed\n");	

		if (pMessageTag->Tag != NULL) {
			free(pMessageTag->Tag);
		}
		free(pMessageTag);	
		return NTY_RESULT_ERROR;	
	}

	ntylog("ntyHttpQJKWeather QJK url:%s\n", tag);
	//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);	// 跳过证书检查  
	//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1);	// 从证书中检查SSL加密算法是否存在  

	curl_easy_setopt(curl, CURLOPT_URL, tag); 
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L); 
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, NTY_CURL_TIMEOUT);
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
		//return -3;	
	}
#if 0
	curl_easy_cleanup(curl);
#endif
	
	ntyHttpRequestResetCURL(req);
#if 0	
	if (tag != NULL) {
		free(tag);
		tag = NULL;
	}
#endif
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
	curl_version_info_data *info=curl_version_info(CURLVERSION_NOW);
	if (info->features & CURL_VERSION_ASYNCHDNS) {
		ntylog("ares enabled\n");
	} else {
		ntylog("ares NOT enabled\n");
	}
#if 1
	curl = curl_easy_init();
#else
	nHttpRequest req = ntyHttpRequestGetCURL();
	curl = req->curl;
	pMessageTag->param = req;
#endif
	if (!curl)	{		
		ntylog("curl init failed\n");		

		return NTY_RESULT_ERROR;	
	}

	ntylog("QJK url:%s\n", tag);

	curl_easy_setopt(curl, CURLOPT_URL, tag); 
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L); 
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, NTY_CURL_TIMEOUT);
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
#if 0
	curl_easy_cleanup(curl);
#endif
#if 0	
	if (tag != NULL) {
		free(tag);
		tag = NULL;
	}
#endif

	return 0;
}






#if 0
//#define QJK_URL "GET http://shangshousoft.applinzi.com/api?m=health&a=falldown&deviceid=123456&platform=4&timestamp=20160809125623&token=d9066e2359acd0e41529482c44de7a39"
//gcc -o NattyHttpCurl NattyHttpCurl.c  NattyTcpServer.c NattyHash.c NattyThreadPool.c NattyAbstractClass.c NattyHBD.c NattyUdpServer.c NattyBPlusTree.c NattyRBTree.c NattyUtils.c NattyLetter.c NattyDBOperator.c NattyConnectionPool.c NattyVector.c NattyFilter.c NattyJson.c NattyServAction.c NattySession.c NattyMulticast.c NattyPush.c NattySqlHandle.c DaveMQ/NattyDaveMQ.c NattyMessage.c  ../json/parson.c -lev -lcrypto -lssl -lmysqlclient -lpthread -lcurl  -I ../include/


#define QJK_URL		"http://restapi.amap.com/v3/geocode/regeo?key=fb44f91d1a1df4d4b6356f43183a329f&location=113.2409402,23.1326885"

int main(void) {

	ntyHttpCurlGlobalInit();

	VALUE_TYPE *tag = (VALUE_TYPE*)malloc(sizeof(VALUE_TYPE));

	tag->Type = MSG_TYPE_QJK_FALLEN;
	tag->fromId = 1;
	tag->length = strlen(QJK_URL);
	tag->Tag = malloc(256);
	memset(tag->Tag, 0, 256);
	memcpy(tag->Tag, QJK_URL, tag->length);

	ntyHttpQJKLocationGetAddress(tag);

	getchar();
}

#endif


