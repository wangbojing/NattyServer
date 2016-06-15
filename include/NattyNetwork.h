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



#ifndef __NATTY_UDP_CLIENT_H__
#define __NATTY_UDP_CLIENT_H__

#include "NattyTimer.h"
#include "NattyProtocol.h"
#include "NattyRBTree.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


typedef long long U64;
typedef unsigned int U32;
typedef unsigned short U16;
typedef unsigned char U8;
typedef long long C_DEVID;

#define SERVER_NAME		"112.93.116.188" //"127.0.0.1"
#define SERVER_PORT		8888
#define RECV_BUFFER_SIZE	(1024+16)
#define SENT_TIMEOUT	3


typedef struct _ThreadArg {
	int sockfd;
	C_DEVID devid;
} ThreadArg;

typedef struct _FRIENDSINFO {
	//C_DEVID devid;
	int sockfd;
	U32 addr;
	U16 port;
	U8 isP2P;
	U8 counter;
} FriendsInfo;


#define SIGNAL_LOGIN_REQ			0x00000001
#define SIGNAL_LOGIN_ACK			0x00000003

#define SIGNAL_HEARTBEAT_REQ		0x00000004
#define SIGNAL_HEARTBEAT_ACK		0x0000000C

#define SIGNAL_LOGOUT_REQ			0x00000010
#define SIGNAL_LOGOUT_ACK			0x00000030

#define SIGNAL_P2PADDR_REQ			0x00000040
#define SIGNAL_P2PADDR_ACK			0x000000C0

#define SIGNAL_P2PCONNECT_REQ		0x00000100
#define SIGNAL_P2PCONNECT_ACK		0x00000300

#define SIGNAL_DATAPACKET_REQ		0x00000400
#define SIGNAL_DATAPACKET_ACK		0x00000C00

#define SIGNAL_P2PDATAPACKET_REQ	0x00001000
#define SIGNAL_P2PDATAPACKET_ACK	0x00003000


enum {
	LEVEL_LOGIN = 0x00,
	LEVEL_HEARTBEART = 0x01,
	LEVEL_P2PCONNECT = 0x02,
	LEVEL_P2PCONNECT_ACK = 0x03,
	LEVEL_LOGOUT = 0x04,
	LEVEL_P2PADDR = 0x05,
	LEVEL_P2PDATAPACKET = 0x06,
	LEVEL_DATAPACKET = 0x07,
	LEVEL_P2PCONNECTFRIEND = 0x08,
	LEVEL_P2PCONNECT_NOTIFY = 0x09,
	LEVEL_DEFAULT			= 0xFF,
} LEVEL;


#define CACHE_BUFFER_SIZE	1048

#define HEARTBEAT_TIMEOUT		25
#define P2P_HEARTBEAT_TIMEOUT	60
#define P2P_HEARTBEAT_TIMEOUT_COUNTR	5

typedef void (*PROXY_CALLBACK)(int len);


typedef struct _NETWORK {
	const void *_;
	int sockfd;
	struct sockaddr_in addr;
	int length;	
	HANDLE_TIMER onAck;
	PROXY_CALLBACK onDataLost;
	U32 ackNum;
	U8 buffer[CACHE_BUFFER_SIZE];
	//void *timer;
} Network;

typedef struct _NETWORKOPERA {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	int (*send)(void *_self, struct sockaddr_in *to, U8 *buf, int len);
	int (*recv)(void *_self, U8 *buf, int len, struct sockaddr_in *from);
	int (*resend)(void *_self);
} NetworkOpera;


void *ntyNetworkInstance(void);
void ntyNetworkRelease(void *self);
int ntySendFrame(void *self, struct sockaddr_in *to, U8 *buf, int len);
int ntyRecvFrame(void *self, U8 *buf, int len, struct sockaddr_in *from);


int ntyGetSocket(void *self);
U8 ntyGetReqType(void *self);
C_DEVID ntyGetDestDevId(void *self);
void ntySetDevId(C_DEVID id);



#endif



