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

#include <curl/curl.h>
#include <string.h>
#include <stdio.h>

size_t ntyHttpQJKFallenHandleResult(void* buffer, size_t size, size_t nmemb, void *stream) {
	VALUE_TYPE *tag = stream;

	printf("buffer:%s, %ld\n", (char*)buffer, size*nmemb);
	free(tag);

	return size*nmemb;
}

int ntyHttpQJKFallen(void *arg) {
	CURL *curl;	
	CURLcode res;	
	VALUE_TYPE *tag = arg;

	CURLcode return_code;
	return_code = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != return_code) {
		ntylog("init libcurl failed.\n");		
		return -1;
	}

	curl = curl_easy_init();	
	if (!curl)	{		
		ntylog("curl init failed\n");		
		return -2;	
	}

	ntydbg("QJK url:%s\n", tag->Tag);

	curl_easy_setopt(curl, CURLOPT_URL, tag->Tag); 
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L); 
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

	return 0;
}

//MSG_TYPE_GAODE_WIFI_CELL_API

size_t ntyHttpGaodeWifiCellAPIHandleResult(void* data, size_t size, size_t nmemb, void *stream) {
	VALUE_TYPE *tag = stream;
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
			
			sprintf(u8ResultBuffer+NTY_PROTO_DATAPACKET_CONTENT_IDX, "Set Location %s:%s:3", u8Lat, u8Lon);
			ntyProtoHttpProxyTransform(tag->fromId, tag->toId, u8ResultBuffer, strlen(u8ResultBuffer+NTY_PROTO_DATAPACKET_CONTENT_IDX));

			break;
		}
	}

	
	free(tag);
	return size*nmemb;
}


int ntyHttpGaodeWifiCellAPI(void *arg) {
	CURL *curl;	
	CURLcode res;	
	VALUE_TYPE *tag = arg;

	CURLcode return_code;
	return_code = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != return_code) {
		ntylog("init libcurl failed.\n");		
		return -1;
	}

	curl = curl_easy_init();	
	if (!curl)	{		
		ntylog("curl init failed\n");		
		return -2;	
	}

	ntydbg("GAODE url:%s\n", tag->Tag);

	curl_easy_setopt(curl, CURLOPT_URL, tag->Tag); 
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L); 
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
	
	curl_easy_cleanup(curl);

	return 0;
}

size_t ntyHttpMtkQuickLocationHandleResult(void* data, size_t size, size_t nmemb, void *stream) {
	VALUE_TYPE *tag = stream;
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
			sprintf(u8ResultBuffer+NTY_PROTO_DATAPACKET_CONTENT_IDX, "Set Location %s:%s:3", u8Lat, u8Lon);
			ntyProtoHttpProxyTransform(tag->fromId, tag->toId, u8ResultBuffer, strlen(u8ResultBuffer+NTY_PROTO_DATAPACKET_CONTENT_IDX));

			break;
		}
	}

	
	free(tag);
	return size*nmemb;
}


int ntyHttpMtkQuickLocation(void *arg) {
	CURL *curl;	
	CURLcode res;	
	VALUE_TYPE *tag = arg;

	CURLcode return_code;
	return_code = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != return_code) {
		ntylog("init libcurl failed.\n");		
		return -1;
	}

	curl = curl_easy_init();	
	if (!curl)	{		
		ntylog("curl init failed\n");		
		return -2;	
	}

	ntydbg("GAODE url:%s\n", tag->Tag);

	curl_easy_setopt(curl, CURLOPT_URL, tag->Tag); 
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L); 
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


