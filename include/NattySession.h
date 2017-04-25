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
 

#ifndef __NATTY_SESSION_H__
#define __NATTY_SESSION_H__

#include "NattyUdpServer.h"
#include "NattyProtocol.h"



typedef struct _INTER_MSG {
	void *self;
	void *group;
	U8 *buffer;
	int length;
	int arg;
} InterMsg;



int ntySendFriendsTreeIpAddr(const void *client, U8 reqType);
#if 0
int ntySendIpAddrFriendsList(void *client, C_DEVID *friends, U16 Count);
#endif

//int ntyRouteUserData(C_DEVID friendId, U8 *buffer);
int ntySendDeviceRouterInfo(const Client *pClient, U8 *buffer, int length);
int ntySendAppRouterInfo(const Client *pClient, C_DEVID fromId, U8 *buffer, int length);
int ntyBoardcastAllFriends(const Client *self, U8 *buffer, int length);
int ntyBoardcastAllFriendsById(C_DEVID fromId, U8 *buffer, int length);
int ntyBoardcastAllFriendsNotifyDisconnect(C_DEVID selfId);
void ntyProtoHttpProxyTransform(C_DEVID fromId, C_DEVID toId, U8 *buffer, int length);
void ntyProtoHttpRetProxyTransform(C_DEVID toId, U8 *buffer, int length);
int ntySendDeviceTimeCheckAck(C_DEVID fromId, U32 ackNum);

void ntyProtoUnBindAck(C_DEVID aid, C_DEVID did, int result);
void ntyProtoBindAck(C_DEVID aid, C_DEVID did, int result);

void ntySelfLogoutPacket(C_DEVID id, U8 *buffer);
int ntyIterFriendsMessage(void *self, void *arg);


/* ** **** ******** **************** Natty V3.6 **************** ******** **** ** */

int ntySendOfflineMsgReqResult(C_DEVID fromId, U8 *json, int length);
int ntySendOfflineMsgAckResult(C_DEVID toId, U8 *json, int length, U16 status);
int ntySendLoginAckResult(C_DEVID fromId, U8 *json, int length, U16 status);
int ntySendLocationBroadCastResult(C_DEVID selfId, C_DEVID gId, U8 *json, int length);
int ntySendVoiceBroadCastResult(C_DEVID fromId, C_DEVID gId, U8 *json, int length, int index);
int ntySendCommonBroadCastResult(C_DEVID selfId, C_DEVID gId, U8 *json, int length, int index);
int ntySendDataResult(C_DEVID toId, U8 *json, int length, U16 status);
int ntySendDataRoute(C_DEVID toId, U8 *buffer, int length);

int ntySendCommonReq(C_DEVID fromId, C_DEVID toId, U8 *json, int length, int msgId);


int ntySendLocationPushResult(C_DEVID fromId, U8 *json, int length);
int ntySendWeatherPushResult(C_DEVID fromId, U8 *json, int length);

int ntySendICCIDAckResult(C_DEVID fromId, U8 *json, int length, U16 status);
int ntySendRecodeJsonPacket(C_DEVID fromId, C_DEVID toId, U8 *json, int length);

int ntySendBinBufferBroadCastResult(U8 *u8Buffer, int length, C_DEVID fromId, C_DEVID gId, U32 index);


int ntySendBindConfirmPushResult(C_DEVID proposerId, C_DEVID adminId, U8 *json, int length);

int ntySendPushNotify(C_DEVID selfId, U8 *msg);


#endif


