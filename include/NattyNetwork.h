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

#if 0
typedef unsigned long long U64;
typedef unsigned int U32;
typedef unsigned short U16;
typedef unsigned char U8;
typedef long long C_DEVID;
//typedef long long C_DEVID;
#endif

#define SERVER_NAME		"112.93.116.189"   //"112.93.116.188" //"112.93.116.189" //"127.0.0.1" 
//#define SERVER_HOSTNAME	"debug.quanjiakan.com"
#define SERVER_HOSTNAME	"localhost"
//#define SERVER_HOSTNAME	"watch.quanjiakan.com"

#define SERVER_PORT		8888
#define RECV_BUFFER_SIZE	1024
#define NORMAL_BUFFER_SIZE	64
#define SENT_TIMEOUT	3


#define DEBUG_WARNNING		0
#define DEBUG_TRACE			1
#define DEBUG_ERROR			2

#define DEBUG_LEVEL			DEBUG_TRACE



#if (DEBUG_LEVEL == DEBUG_WARNNING)
#define warn(format, ...)			fprintf(stdout, format, ##__VA_ARGS__)
#elif (DEBUG_LEVEL == DEBUG_TRACE)
#define LOG(format, ...) 			fprintf(stdout, format, ##__VA_ARGS__)
#define warn(format, ...)			fprintf(stdout, format, ##__VA_ARGS__)
#define trace(format, ...)			fprintf(stdout, format, ##__VA_ARGS__)
#elif (DEBUG_LEVEL == DEBUG_ERROR)
#define error(format, ...)			fprintf(stdout, format, ##__VA_ARGS__)
#else
#define LOG(format, ...) 			fprintf(stdout, format, ##__VA_ARGS__)
#endif


#if 0
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
#endif

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

typedef enum  {
	NTY_TIMER_ID_START = 0,
	NTY_TIMER_ID_HEARTBEAT = NTY_TIMER_ID_START,
	NTY_TIMER_ID_RECONNECT = 1,
	NTY_TIMER_ID_SENDBIGBUFFER = 2,
	NTY_TIMER_ID_RECVBIGBUFFER = 3,
	NTY_TIMER_ID_SENDPACKET = 4,
	NTY_TIMER_ID_END = NTY_TIMER_ID_SENDPACKET,
	NTY_TIMER_ID_COUNT,
} NTY_TIMER_ID;


#define CACHE_BUFFER_SIZE	1048

#define PACKET_SEND_TIME_TICK	10
#define HEARTBEAT_TIME_TICK		300
#define RECONNECT_TIME_TICK		60
#define P2P_HEARTBEAT_TIMEOUT	60
#define P2P_HEARTBEAT_TIMEOUT_COUNTR	5

typedef void (*PROXY_CALLBACK)(int len);
typedef void (*PROXY_HANDLE_CB)(C_DEVID id, int len);

typedef void (*NTY_STATUS_CALLBACK)(int status);
typedef void (*NTY_PARAM_CALLBACK)(U8 *arg, int length);
typedef void (*NTY_RETURN_CALLBACK)(C_DEVID fromId, U8 *arg, int length);


typedef struct _NETWORK {
	const void *_;
	int sockfd;
	struct addrinfo addr;
} Network;

typedef Network ClientSocket;

typedef struct _NETWORKOPERA {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	int (*send)(void *_self, U8 *buffer, int len);
	int (*recv)(void *_self, U8 *buffer, int len);
	int (*connect)(void *_self);
	int (*reconnect)(void *_self);
} NetworkOpera;

typedef NetworkOpera ClientSocketHandle;

void* ntyNetworkInstance(void);
void* ntyNetworkRelease(void);
int ntySendFrame(void *self, U8 *buffer, int len);
int ntyRecvFrame(void *self, U8 *buffer, int len);
int ntyReconnect(void *self);


int ntyGetSocket(void *self);
U8 ntyGetReqType(void *self);
C_DEVID ntyGetDestDevId(void *self);
void ntySetDevId(C_DEVID id);

void ntyGenCrcTable(void);


#endif



