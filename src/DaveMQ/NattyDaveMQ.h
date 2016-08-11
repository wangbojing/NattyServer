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



#define DAVE_MESSAGE_LENGTH			256


typedef enum {
	MSG_TYPE_START = 0,
	MSG_TYPE_QJK = MSG_TYPE_START,
	MSG_TYPE_GAODE,
	MSG_TYPE_END = MSG_TYPE_GAODE,
} MESSAGE_TYPE;

typedef struct _MESSAGETAG {
	int msgType;
	//unsigned char 
} MessageTag;

typedef int VALUE_TYPE;

typedef struct _DaveNode {
	VALUE_TYPE value;
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
	void (*enqueue)(void *_self, VALUE_TYPE value);
	void* (*dequeue)(void *_self);
} DaveQueueHandle;

void *ntyDaveMqWorkerInstance(void);


#endif


