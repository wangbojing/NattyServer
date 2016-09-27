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


#ifndef __NATTY_MULTICAST_H__
#define __NATTY_MULTICAST_H__

#include "NattyAbstractClass.h"
#include "NattyUdpServer.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <poll.h>


#define NTY_MULTICAST_ADDRESS		"224.0.243.12"
#define NYT_MULTICAST_PORT			8882


typedef struct _MulticastServer {
	const void *_;
	int sockfd;
	struct sockaddr_in addr;
	struct ip_mreq mreq;
} MulticastServer;


typedef struct _MulticastServerHandle {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	int (*process)(const void *_self);
	int (*send)(const void *_self, U8 *data, int length);
} MulticastServerHandle;


void* ntyMulticastServerInstance(void);
int ntyMulticastSend(U8 *data, int length);
int ntyMulticastServerRun(const void *arg);



#endif







