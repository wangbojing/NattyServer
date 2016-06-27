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
		printf("%x", buffer[i]);
	}
	printf("\n");
}

void ntySendSuccess(int arg) {
	printf("Success\n");
}

void ntySendFailed(int arg) {
	if (arg == STATUS_TIMEOUT)
		printf("STATUS_TIMEOUT\n");
	else if (arg == STATUS_NOEXIST)
		printf("STATUS_NOEXIST\n");
}


int main() {
	C_DEVID AppId = 0x01;
	int n = 0, length;
	U8 tempBuf[RECV_BUFFER_SIZE] = {0};
	
	printf(" Press DevId <1 or 2>: ");   	
	n = scanf("%lld", &AppId);
	
	ntySetProxyCallback(ntyUserRecvCb);
	ntySetSendFailedCallback(ntySendFailed);
	ntySetSendSuccessCallback(ntySendSuccess);

	ntySetDevId(AppId);

	sleep(5);
	while(1) {
		printf("Proxy Please send msg:");
		char *ptr = fgets(tempBuf, RECV_BUFFER_SIZE, stdin);
		int len = strlen(tempBuf);
		ntySendMassDataPacket(tempBuf, len);
	}
}


