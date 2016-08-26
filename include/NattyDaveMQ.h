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



#ifndef __DAVE_MQ_H__
#define __DAVE_MQ_H__

#include "NattyAbstractClass.h"



#define DAVE_MESSAGE_LENGTH			512

#define NTY_HTTP_GET_HANDLE_STRING			"GET"
#define NTY_HTTP_POST_HANDLE_STRING			"POST"
#define NTY_HTTP_RET_HANDLE_STRING			"RET"



typedef enum {
	MSG_TYPE_START = 0,
	MSG_TYPE_QJK_FALLEN = MSG_TYPE_START,
	MSG_TYPE_GAODE_WIFI_CELL_API,
	MSG_TYPE_MTK_QUICKLOCATION,
	MSG_TYPE_END = MSG_TYPE_MTK_QUICKLOCATION,
} MESSAGE_TYPE;

typedef struct _MESSAGETAG {
	MESSAGE_TYPE Type;
	U8 Tag[DAVE_MESSAGE_LENGTH];
	int length;
	C_DEVID fromId;
	C_DEVID toId;
} MessageTag;

//typedef int VALUE_TYPE;
typedef MessageTag VALUE_TYPE;


typedef struct _DaveNode {
	VALUE_TYPE *value;
	struct _DaveNode *next;
	struct _DaveNode *prev;
} DaveNode ;

typedef struct _DaveQueue {
	const void *_;
	DaveNode *head;
	DaveNode *tail;
	DaveNode *nil;
} DaveQueue;

typedef struct _DaveQueueHandle {
	int size;
	void* (*ctor)(void *_self);
	void* (*dtor)(void *_self);
	void (*enqueue)(void *_self, VALUE_TYPE *value);
	void* (*dequeue)(void *_self);
} DaveQueueHandle;

void *ntyDaveMqWorkerInstance(void);
void ntyDaveMqPushWorker(void *arg) ;
int ntyClassifyMessageType(C_DEVID fromId, C_DEVID toId, U8 *data, int length);

void ntyDaveMqStart(void);




#endif


