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

#define APPLE_CLIENT_PEM_NAME_JINGWU			"/home/crt/jingwu_ck_development.pem"
#define APPLE_CLIENT_PEM_NAME_PUBLISH_JINGWU	"/home/crt/jingwu_ck_production.pem"


#define APPLE_CLIENT_PEM_KEY			"123456"
#define APPLE_CLIENT_PEM_PWD			"123456"

#define APPLE_SERVER_PEM_NAME			"/home/crt/entrust_2048_ca.pem"


#define MAX_MESSAGE_PACKET		1024

#define MAX_PAYLOAD_SIZE		256
#define TOKEN_SIZE				32

#define NTY_PUSH_MSG_CONTEXT					"平安通 \n 您收到了新消息"
#define NTY_PUSH_VOICE_MSG_CONTEXT				"平安通 \n 您收到了语音消息"
#define NTY_PUSH_EFENCE_MSG_CONTEXT				"平安通 \n 您收到了电子围栏设置消息"
#define NTY_PUSH_SOSREPORT_MSG_CONTEXT			"平安通 \n 您收到了SOS告警消息"
#define NTY_PUSH_EFENCEREPORT_MSG_CONTEXT		"平安通 \n 您收到了电子围栏越界消息"
#define NTY_PUSH_BINDCONFIRM_MSG_CONTEXT		"平安通 \n 您收到了绑定请求确认消息"
#define NTY_PUSH_BINDCONFIRM_OK_MSG_CONTEXT		"平安通 \n 您收到了管理员通过绑定请求消息"
#define NTY_PUSH_WEARSTATUS_MSG_CONTEXT			"平安通 \n 您收到了手表穿戴状态消息"
#define NTY_PUSH_FALLDOWN_MSG_CONTEXT			"平安通 \n 您收到了跌倒告警消息"
#define NTY_PUSH_BLOOD_MSG_CONTEXT				"平安通 \n 您收到了血压告警消息"
#define NTY_PUSH_HEART_MSG_CONTEXT				"平安通 \n 您收到了心率告警消息"
#define NTY_PUSH_MSG_CONTEXT_MATTRESS			"平安通 \n 您收到了安全告警消息"

#define NTY_PUSH_MSG_CONTEXT_QJK				"全家康 \n 您收到了新消息"
#define NTY_PUSH_VOICE_MSG_CONTEXT_QJK			"全家康  \n 您收到了语音消息"
#define NTY_PUSH_EFENCE_MSG_CONTEXT_QJK			"全家康  \n 您收到了电子围栏设置消息"
#define NTY_PUSH_SOSREPORT_MSG_CONTEXT_QJK		"全家康  \n 您收到了SOS告警消息"
#define NTY_PUSH_EFENCEREPORT_MSG_CONTEXT_QJK	"全家康  \n 您收到了电子围栏越界消息"
#define NTY_PUSH_BINDCONFIRM_MSG_CONTEXT_QJK	"全家康  \n 您收到了绑定请求确认消息"
#define NTY_PUSH_BINDCONFIRM_OK_MSG_CONTEXT_QJK	"全家康 \n 您收到了管理员通过绑定请求消息"
#define NTY_PUSH_WEARSTATUS_MSG_CONTEXT_QJK		"全家康  \n 您收到了手表穿戴状态消息"
#define NTY_PUSH_FALLDOWN_MSG_CONTEXT_QJK		"全家康  \n 您收到了跌倒告警消息"
#define NTY_PUSH_BLOOD_MSG_CONTEXT_QJK			"全家康 \n 您收到了血压告警消息"
#define NTY_PUSH_HEART_MSG_CONTEXT_QJK			"全家康 \n 您收到了心率告警消息"
#define NTY_PUSH_MSG_CONTEXT_MATTRESS_QJK		"全家康  \n 您收到了安全告警消息"

#define NTY_PUSH_POLICE_MSG_CONTEXT				"警务通 \n 您收到了报警消息"



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

	//B stand for jingwu
	SSL_CTX *ctx_b;
	SSL_CTX *pctx_b;

	int d_sockfd_b; //B stand for jingwu
	int p_sockfd_b; //B stand for jingwu

	SSL *d_ssl_b; //B stand for jingwu
	SSL *p_ssl_b; //B stand for jingwu

} nPushContext;

typedef struct _Push_Handle {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	int (*push)(void *_self, C_DEVID gId, U32 type, U32 counter, U8 *msg, const U8 *token, U8 mode);
} nPushHandle;


typedef enum {
	NTY_PUSH_CLIENT_DEVELOPMENT = 0,
	NTY_PUSH_CLIENT_PRODUCTION = 1,
	NTY_PUSH_CLIENT_DEVELOPMENT_B = 2,
	NTY_PUSH_CLIENT_PRODUCTION_B = 3,
	NTY_PUSH_CLIENT_COUNT = 4,
} NTY_PUSH_TYPE;

void *ntyPushHandleInstance(void);
int ntyPushNotifyHandle(void *self, C_DEVID gId, U32 type, U32 counter, U8 *msg, const U8 *token, U8 mode);



#endif








