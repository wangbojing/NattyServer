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

#ifndef __NATTY_TCP_SERVER_H__
#define __NATTY_TCP_SERVER_H__



#include "NattyUdpServer.h"

#define NATTY_TCP_SERVER_PORT			NATTY_UDP_SERVER_PORT
#define NATTY_CONNECTION_BACKLOG 		8
#define NATTY_SOCKET_READ_TIMEOUT		10
#define NATTY_SOCKET_WRITE_TIMEOUT		10


typedef Client TcpClient;
typedef Server TcpServer;
typedef ServerHandle TcpServerHandle; 
#if 0
typedef struct _TcpClient {
	int fd;
	struct event_base *evbase;
	struct bufferevent *buf_ev;
	struct evbuffer *output_buffer;
} TcpClient;
#endif



void* ntyTcpServerInstance(void);
int ntyTcpServerRun(const void *arg);
void *ntyTcpServerGetMainloop(void);



#endif





