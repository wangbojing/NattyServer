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




#ifndef __NATTY_UDP_SERVER_H__
#define __NATTY_UDP_SERVER_H__

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <ev.h>
#include <pthread.h>

#include "NattyAbstractClass.h"
#include "NattyConfig.h"
#include "NattyTimer.h"


#define NATTY_UDP_SERVER		8888

#define RECV_BUFFER_SIZE 		1024
#define PACKET_BUFFER_SIZE		1024
#define NATTY_UDP_SERVER_PORT	NATTY_UDP_SERVER

#if 0
typedef struct _UdpClient {
	int sockfd;
	struct sockaddr_in addr;
	C_DEVID devId; //client id use for rb-tree key
	U32 ackNum;
	void *friends; //client id list for this key
} UdpClient;

typedef struct _UdpServer {
	const void *_;
	int sockfd;
	struct sockaddr_in addr;
} UdpServer;

#else

typedef enum _CLIENT_TYPE {
	PROTO_TYPE_START = 0x0,
	PROTO_TYPE_TCP = PROTO_TYPE_START,
	PROTO_TYPE_UDP,
	PROTO_TYPE_MULTICAST,
	PROTO_TYPE_END = PROTO_TYPE_MULTICAST,
	PROTO_TYPE_COUNT
} CLIENT_TYPE;

typedef enum _DEVICE_TYPE {
	DEVICE_TYPE_START = 0x0,
	DEVICE_TYPE_APP = DEVICE_TYPE_START,
	DEVICE_TYPE_WATCH,
	DEVICE_TYPE_END = DEVICE_TYPE_WATCH,
	DEVICE_TYPE_COUNT
} DEVICE_TYPE;
#if 0
typedef struct _Client {
	int sockfd;
	struct sockaddr_in addr;
	C_DEVID devId; //client id use for rb-tree key
	U32 ackNum;
	void *friends; //client id list for this key
	TIMESTAMP stamp;
	struct ev_io *watcher;
	U8 clientType; //UDP / TCP
	U8 deviceType; 
	U16 rLength; //recv length
	U8 *recvBuffer; //recvBuffer
	pthread_mutex_t buffer_mutex;
} Client;
#else

typedef struct _NValue {
	int sockfd;
	struct sockaddr_in addr;
	U8 connectType; //UDP / TCP
} NValue;

typedef NValue ClientSocket;

typedef struct _Client {
	C_DEVID devId; //client id use for rb-tree key
	U8 deviceType; //device type , android ,ios, web, java, device
	U16 rLength; //recv length
	U8 *recvBuffer; //recvBuffer
	void *friends; //client id list for this key
	void *group; //group id list 
	pthread_mutex_t bMutex; //big buffer write lock
#if 1//Wheel Timer for Timer List
	void *hbdTimer;
	TIMESTAMP stamp;
#endif
} Client;

#endif

typedef Client UdpClient;

typedef struct _Server {
	const void *_;
	int sockfd;
	struct sockaddr_in addr;
} Server;

typedef Server UdpServer;

typedef struct _ServerHandle {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	int (*process)(const void *_self);
} ServerHandle;

typedef ServerHandle UdpServerOpera;

#endif

typedef struct _MessagePacket {
	Client *client;
	U8 *buffer; //current buffer
	U16 length; //current buffer length
	U8 connectType; //UDP / TCP
	
	union {//watcher socket use for network communicate
		int sockfd;
		struct ev_io *watcher; 
	};	
} MessagePacket;

typedef MessagePacket RequestPacket;


int ntyUdpServerRun(const void *arg);
void* ntyUdpServerInstance(void);
int ntyClientCompare(const UdpClient *clientA, const UdpClient *clientB);
int ntySendBuffer(ClientSocket *client, unsigned char *buffer, int length);
void* allocRequestPacket(void);
void freeRequestPacket(void *pReq);



#endif



