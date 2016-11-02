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



#ifndef __NATTY_FILTER_H__
#define __NATTY_FILTER_H__

#include "NattyUdpServer.h"
#include "NattyProtocol.h"
#include "NattyAbstractClass.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <ev.h>



typedef struct {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	void (*setSuccessor)(void *_self, void *succ);
	void* (*getSuccessor)(const void *_self);
	void (*handleRequest)(const void *_self, unsigned char *buffer, int length,const void* obj);
} ProtocolFilter;

typedef struct {
	const void *_;
	void *succ;
} Packet;

void* ntyProtocolFilterInit(void);
void* ntyProtocolFilterInstance(void);

void ntyProtocolFilterProcess(void *_filter, unsigned char *buffer, U32 length,const void *obj);
void ntyProtocolFilterRelease(void *_filter);

void ntyGenCrcTable(void);
U32 ntyGenCrcValue(U8 *buf, int length);

void ntyProtoHttpProxyTransform(C_DEVID fromId, C_DEVID toId, U8 *buf, int length);
int ntyReleaseClientNodeByAddr(struct ev_loop *loop, struct sockaddr_in *addr, struct ev_io *watcher);
int ntyReleaseClientNodeSocket(struct ev_loop *loop, struct ev_io *watcher, int sockfd);
int ntyReleaseClientNodeHashTable(struct sockaddr_in *addr);
int ntyReleaseClientNodeByDevID(struct ev_loop *loop, struct ev_io *watcher, C_DEVID devid);

int ntyReleaseClientNodeByNode(struct ev_loop *loop, void *node);



typedef struct _Node {
	C_DEVID clientId;
	struct _Node *next;
} Node;

typedef struct {
	const void *_;
	Node *head;
	int count;
} SingleList;


typedef struct {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	void (*insert)(void *_self, int id);
	int (*remove)(void *_self, int id);
	C_DEVID* (*iterator)(const void *_self);
	void (*print)(const void *_self);
} List;


#endif


