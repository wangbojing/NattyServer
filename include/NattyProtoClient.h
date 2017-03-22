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
typedef unsigned long long DEVID;

typedef void (*PROXY_CALLBACK_CLINET)(int len);
typedef void (*PROXY_HANDLE_CLIENT)(DEVID id, int len);

typedef void (*NTY_STATUS_CALLBACK)(int status);
typedef void (*NTY_PARAM_CALLBACK)(U8 *arg, int length);
typedef void (*NTY_RETURN_CALLBACK)(DEVID fromId, U8 *arg, int length);
typedef void (*NTY_PACKET_CALLBACK)(DEVID fromId, DEVID gId, int length);


#define CLIENT_BUFFER_SIZE		1024
#define NTY_BIGBUFFER_SIZE		30*1024
#define NTY_TIMER_SIZE			32

#ifndef ntydbg
#define ntydbg(format, ...) 		fprintf(stdout, format, ##__VA_ARGS__)
#endif

int ntySendDataPacket(DEVID toId, U8 *data, int length);
int ntySendMassDataPacket(U8 *data, int length);

void* ntyStartupClient(int *status);
void ntyShutdownClient(void);


void ntySetSendSuccessCallback(PROXY_CALLBACK_CLINET cb);
void ntySetSendFailedCallback(PROXY_CALLBACK_CLINET cb);
void ntySetProxyCallback(PROXY_CALLBACK_CLINET cb);
U8* ntyGetRecvBuffer(void);
void ntySetDevId(DEVID id);
int ntyGetRecvBufferSize(void);
void ntyReleaseNetwork(void);
void ntySetBindResult(PROXY_CALLBACK_CLINET cb);
void ntySetUnBindResult(PROXY_CALLBACK_CLINET cb);
void ntySetProxyDisconnect(PROXY_CALLBACK_CLINET cb);
void ntySetProxyReconnect(PROXY_CALLBACK_CLINET cb);

void ntySetPacketRecv(NTY_PACKET_CALLBACK cb);
void ntySetPacketSuccess(PROXY_CALLBACK_CLINET cb);


void ntySetCommonBroadCastResult(NTY_RETURN_CALLBACK cb);
void ntySetLocationBroadCastResult(NTY_RETURN_CALLBACK cb);
void ntySetVoiceBroadCastResult(NTY_RETURN_CALLBACK cb);

void ntySetDataResult(NTY_PARAM_CALLBACK cb);
void ntySetDataRoute(NTY_RETURN_CALLBACK cb);

void ntySetWeatherPushResult(NTY_PARAM_CALLBACK cb);
void ntySetLocationPushResult(NTY_PARAM_CALLBACK cb);

void ntySetOfflineMsgAckResult(NTY_PARAM_CALLBACK cb);
void ntySetVoiceDataAckResult(NTY_STATUS_CALLBACK cb);
void ntySetCommonReqResult(NTY_RETURN_CALLBACK cb);

void ntySetICCIDAckResult(NTY_PARAM_CALLBACK cb);
void ntySetTimeAckResult(NTY_PARAM_CALLBACK cb);

void ntySetLogoutAckResult(NTY_STATUS_CALLBACK cb);
void ntySetHeartBeatAckResult(NTY_STATUS_CALLBACK cb);
void ntySetLoginAckResult(NTY_PARAM_CALLBACK cb);


int ntyBindClient(DEVID did);
int ntyUnBindClient(DEVID did);

int ntyVoiceReqClient(U32 msgId, U8 *json, U16 length);
int ntyVoiceAckClient(U32 msgId, U8 *json, U16 length);
int ntyVoiceDataReqClient(DEVID gId, U8 *data, int length);

int ntyCommonReqClient(DEVID gId, U8 *json, U16 length);
int ntyCommonAckClient(U32 msgId, U8 *json, U16 length);
int ntyDataRouteClient(DEVID toId, U8 *json, U16 length);

void ntyProtoClientSetToken(void *_self, U8 *tokens, int length);

U8 *ntyGetRecvBigBuffer(void);
U8 *ntyGetSendBigBuffer(void);
int ntyGetRecvBigLength(void);

char* ntyProtoClientGetVersion(void);

#endif

