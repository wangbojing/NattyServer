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



#ifndef __NATTY_TIMER_H__
#define __NATTY_TIMER_H__

#include <pthread.h>
#include "NattyAbstractClass.h"

typedef void (*HANDLE_TIMER)(int sig);

typedef struct _NetworkTimer {
	const void *_;
	int sigNum;
	U32 timerProcess;
	HANDLE_TIMER timerFunc;
	pthread_mutex_t timer_mutex;
	pthread_cond_t timer_cond;
} NetworkTimer;

typedef struct _TIMEROPERA {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	int (*start)(void *_self, HANDLE_TIMER fun);
	int (*stop)(void *_self);
} TimerOpera;


#define TIMER_TICK		200
#define MS(x)		(x*1000)

#define RECONNECT_TICK		60
#define S(x)		(MS(x)*1000)

void *ntyNetworkTimerInstance(void);
void *ntyReconnectTimerInstance(void);

int ntyStartTimer(void *self,  HANDLE_TIMER func);
int ntyStopTimer(void *self);



#endif








