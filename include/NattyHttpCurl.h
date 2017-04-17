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


#ifndef __NATTY_HTTP_CURL_H__
#define __NATTY_HTTP_CURL_H__


#include "NattyAbstractClass.h"

#include <curl/curl.h>

#define MAX_HTTTP_REQUEST_COUNT				30
#define NTY_CURL_TIMEOUT					5

typedef struct NTY_HTTP_REQUEST {
	CURL *curl;	
	void *arg;
	U8 enable;
	U8 index;
	long req_lock;
} nHttpRequest;


typedef struct NTY_HTTP_REQUEST_MOTOR {
	nHttpRequest request[MAX_HTTTP_REQUEST_COUNT];
} nRequestMotor;

int ntyHttpMtkQuickLocation(void *arg);
int ntyHttpGaodeWifiCellAPI(void *arg);
int ntyHttpQJKFallen(void *arg);

int ntyHttpQJKLocation(void *arg);
int ntyHttpQJKWeatherLocation(void *arg);
int ntyHttpQJKWeather(void *arg);

int ntyHttpCurlGlobalInit(void);


#endif






