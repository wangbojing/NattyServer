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

int ntySendFriendIpAddr(void* fTree, C_DEVID id);
int ntyNotifyFriendConnect(void* fTree, C_DEVID id);
int ntySendFriendsTreeIpAddr(void *fTree, U8 reqType);
int ntyNotifyFriendMessage(C_DEVID fromId, C_DEVID toId);
int ntySendIpAddrFriendsList(void *client, C_DEVID *friends, U16 Count);

//int ntyRouteUserData(C_DEVID friendId, U8 *buffer);
int ntySendDeviceRouterInfo(const Client *pClient, U8 *buffer, int length);
int ntySendAppRouterInfo(const Client *pClient, C_DEVID fromId, U8 *buffer, int length);
int ntyBoardcastAllFriends(const Client *self, U8 *buffer, int length);
int ntyBoardcastAllFriendsById(C_DEVID fromId, U8 *buffer, int length);
int ntyBoardcastAllFriendsNotifyDisconnect(C_DEVID selfId);
void ntyProtoHttpProxyTransform(C_DEVID fromId, C_DEVID toId, U8 *buffer, int length);
void ntyProtoHttpRetProxyTransform(C_DEVID toId, U8 *buffer, int length);
int ntySendDeviceTimeCheckAck(const UdpClient *pClient, U32 ackNum);

void ntyProtoUnBindAck(C_DEVID aid, C_DEVID did, int result);
void ntyProtoBindAck(C_DEVID aid, C_DEVID did, int result);



#endif


