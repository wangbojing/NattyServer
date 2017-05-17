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




#define APPLE_HOST_NAME					"gateway.sandbox.push.apple.com"
#define APPLE_HOST_PORT					2195

#define APPLE_CLIENT_PEM_NAME			"/home/crt/ck_development.pem"
#define APPLE_CLIENT_PEM_NAME_PUBLISH	"/home/crt/ck_production.pem"

#define APPLE_CLIENT_PEM_KEY			"123456"
#define APPLE_CLIENT_PEM_PWD			"123456"

#define APPLE_SERVER_PEM_NAME			"/home/crt/entrust_2048_ca.pem"


#define MAX_MESSAGE_PACKET		1024

#define MAX_PAYLOAD_SIZE		256
#define TOKEN_SIZE				32

#define NTY_PUSH_MSG_CONTEXT			"全家康 \n 您收到了一条新消息"


typedef struct _Push_Context {
	const void *_;
	SSL_CTX *ctx;
	SSL_CTX *pctx;
	struct sockaddr_in addr;

	int d_sockfd;
	int p_sockfd;

	SSL *d_ssl;
	SSL *p_ssl;

} nPushContext;

typedef struct _Push_Handle {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	int (*push)(void *_self, U8 *msg, const U8 *token, U8 mode);
} nPushHandle;

void *ntyPushHandleInstance(void);
int ntyPushNotifyHandle(void *self, U8 *msg, const U8 *token, U8 mode);



#endif








