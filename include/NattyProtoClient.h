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


#ifndef __NATTY_PROTO_CLIENT_H__
#define __NATTY_PROTO_CLIENT_H__


typedef enum {
	STATUS_TIMEOUT = 0x0,
	STATUS_NOEXIST,
} StatusSendResult;

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
typedef long long DEVID;

typedef void (*PROXY_CALLBACK_CLINET)(int len);
typedef void (*PROXY_HANDLE_CLIENT)(DEVID id, int len);

#define CLIENT_BUFFER_SIZE		1024

#ifndef ntydbg
#define ntydbg(format, ...) 		fprintf(stdout, format, ##__VA_ARGS__)
#endif


int ntySendDataPacket(DEVID toId, U8 *data, int length);
int ntySendMassDataPacket(U8 *data, int length);

int ntyStartupClient(void);
void ntyShutdownClient(void);


void ntySetSendSuccessCallback(PROXY_CALLBACK_CLINET cb);
void ntySetSendFailedCallback(PROXY_CALLBACK_CLINET cb);
void ntySetProxyCallback(PROXY_HANDLE_CLIENT cb);
U8* ntyGetRecvBuffer(void);
void ntySetDevId(DEVID id);
int ntyGetRecvBufferSize(void);
void ntyReleaseNetwork(void);
int ntyGetNetworkStatus(void);
void ntySetBindResult(PROXY_CALLBACK_CLINET cb);
void ntySetUnBindResult(PROXY_CALLBACK_CLINET cb);
void ntySetProxyDisconnect(PROXY_CALLBACK_CLINET cb);
void ntySetProxyReconnect(PROXY_CALLBACK_CLINET cb);



void ntyBindClient(DEVID did);
void ntyUnBindClient(DEVID did);

DEVID* ntyGetFriendsList(int *Count);
void ntyReleaseFriendsList(DEVID **list);

#endif

