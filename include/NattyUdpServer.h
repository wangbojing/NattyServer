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


#include "NattyAbstractClass.h"


#define NATTY_UDP_SERVER		8888
#define RECV_BUFFER_SIZE 		1048
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
	PROTO_TYPE_END = PROTO_TYPE_UDP,
	PROTO_TYPE_COUNT
} CLIENT_TYPE;

typedef struct _Client {
	int sockfd;
	struct sockaddr_in addr;
	C_DEVID devId; //client id use for rb-tree key
	U32 ackNum;
	void *friends; //client id list for this key
	U8 clientType; //UDP / TCP
} Client;

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




typedef struct _RequestPacket {
	Client *client;
	U8 *buffer;
	U16 length;
} RequestPacket;

int ntyUdpServerRun(const void *arg);
void* ntyUdpServerInstance(void);
int ntyClientCompare(const UdpClient *clientA, const UdpClient *clientB);
int ntySendBuffer(const UdpClient *client, unsigned char *buffer, int length);
void* allocRequestPacket(void);
void freeRequestPacket(void *pReq);



#endif



