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

#include <string.h>

#include "NattyProtoClient.h"



void ntyUserRecvCb(int len) {
	int i = 0;
	U8 *buffer = ntyGetRecvBuffer();

	for (i = 0;i < len;i ++) {
		ntylog("%x", buffer[i]);
	}
	ntylog("\n");
}

void ntySendSuccess(int arg) {
	ntylog("Success\n");
}

void ntySendFailed(int arg) {
	if (arg == STATUS_TIMEOUT)
		ntylog("STATUS_TIMEOUT\n");
	else if (arg == STATUS_NOEXIST)
		ntylog("STATUS_NOEXIST\n");
}


int main() {
	C_DEVID AppId = 0x01;
	int n = 0, length;
	U8 tempBuf[RECV_BUFFER_SIZE] = {0};
	const char *url =  "GET http://apilocate.amap.com/position?accesstype=1&imei=352315052834187&macs=30:FC:68:B9:E6:E6,-48,TP-LINK_E6E6_JUGUI|50:BD:5F:00:5F:62,-79,TP-LINK_5F62|28:10:7B:F6:E1:BA,-80,joshen?D|C0:61:18:90:1D:64,-86,ttt222????|D4:EE:07:3E:C8:8A,-87,HiWiFi_3EC88A|&output=xml&key=81040f256992a218a8a20ffb7f13ba9f HTTP/1.1";
	
	ntylog(" Press DevId <1 or 2>: ");   	
	n = scanf("%lld", &AppId);
	
	ntySetProxyCallback(ntyUserRecvCb);
	ntySetSendFailedCallback(ntySendFailed);
	ntySetSendSuccessCallback(ntySendSuccess);

	ntySetDevId(AppId);
	sleep(5);
	while(1) {
		ntylog("Proxy Please send msg:");
		char *ptr = fgets(tempBuf, RECV_BUFFER_SIZE, stdin);

		memset(tempBuf, 0, RECV_BUFFER_SIZE);
		memcpy(tempBuf, url, strlen(url));
		int len = strlen(tempBuf);
		ntySendMassDataPacket(tempBuf, len);
	}
}


