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

#ifndef __NATTY_PUSH_H__
#define __NATTY_PUSH_H__


#include "NattyAbstractClass.h"


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <errno.h>

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>




#define APPLE_HOST_DEVELOPMENT_NAME		"gateway.sandbox.push.apple.com"
#define APPLE_HOST_PRODUCTION_NAME		"gateway.push.apple.com"
#define APPLE_HOST_PORT					2195

#define APPLE_CLIENT_PEM_NAME			"/home/crt/ck_development.pem"
#define APPLE_CLIENT_PEM_NAME_PUBLISH	"/home/crt/ck_production.pem"

#define APPLE_CLIENT_PEM_KEY			"123456"
#define APPLE_CLIENT_PEM_PWD			"123456"

#define APPLE_SERVER_PEM_NAME			"/home/crt/entrust_2048_ca.pem"


#define MAX_MESSAGE_PACKET		1024

#define MAX_PAYLOAD_SIZE		256
#define TOKEN_SIZE				32
#define PUSH_HANDLE_MAX_COUNT   20
#define PUSH_HANDLE_MAX_SOCKET (PUSH_HANDLE_MAX_COUNT*2)

#define NTY_PUSH_MSG_CONTEXT			"全家康 \n 您收到了一条新消息"
#define NTY_PUSH_VOICE_MSG_CONTEXT			"全家康 \n 您收到了一条语音消息"
#define NTY_PUSH_EFENCE_MSG_CONTEXT			"全家康 \n 您收到了一条电子围栏设置消息"
#define NTY_PUSH_SOSREPORT_MSG_CONTEXT			"全家康 \n 您收到了一条SOS消息"
#define NTY_PUSH_EFENCEREPORT_MSG_CONTEXT			"全家康 \n 您收到了电子围栏越界消息"
#define NTY_PUSH_BINDCONFIRM_MSG_CONTEXT			"全家康 \n 您收到了申请确认消息"


typedef struct _Push_Context {
	const void *_;
	SSL_CTX *ctx;
	SSL_CTX *pctx;
#if 0
	struct sockaddr_in addr;
#endif
	int d_sockfd;
	int p_sockfd;

	SSL *d_ssl;
	SSL *p_ssl;

} nPushContext;

typedef struct _Push_Handle {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	int (*push)(void *_self, C_DEVID gId, U32 type, U32 counter, U8 *msg, const U8 *token, U8 mode);
} nPushHandle;

typedef struct _stPushHandle {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	int (*push)(void *_self, C_DEVID gId, U32 type, U32 counter, U8 *msg, const U8 *token, U8 mode, U32 index);
} stPushHandle;


typedef struct PushHandleIndex{
	int lockFlag;	
}PushHandleIndex;

typedef struct _PushHandleObj{
	stPushHandle *pushHandleArray[PUSH_HANDLE_MAX_COUNT];
	PushHandleIndex pushHandleIndexArray[PUSH_HANDLE_MAX_COUNT]; //the flag index for pushHandleArray[PUSH_HANDLE_MAX_COUNT]
}PushHandleObj;


typedef enum {
	NTY_PUSH_CLIENT_DEVELOPMENT = 0,
	NTY_PUSH_CLIENT_PRODUCTION = 1,
	NTY_PUSH_CLIENT_COUNT = 2,
} NTY_PUSH_TYPE;

void *ntyPushHandleInstance(void);
int ntyPushConnectionPoolInstance(void);
//when use this function,and then must use the function ntySetPushHandle( int index )
stPushHandle *ntyGetPushHandle( int *outInt );
int ntySetPushHandle( U32 index );
int ntyPushNotifyToHandle(void *self, C_DEVID gId, U32 type, U32 counter, U8 *msg, const U8 *token, U8 mode, U32 index);



int ntyPushNotifyHandle(void *self, C_DEVID gId, U32 type, U32 counter, U8 *msg, const U8 *token, U8 mode);



#endif








