/*
 *  Author : WangBoJing , email : 1989wangbojing@gmail.com
 * 
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of NALEX Inc. (C) 2016
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



#include <signal.h>
#include <sys/time.h>
#include <string.h>

#include "NattyTimer.h"


static void* ntyTimerCtor(void *_self, va_list *params) {
	NetworkTimer *timer = _self;
	pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;

	timer->sigNum = SIGALRM;
	//timer->timer_mutex = PTHREAD_MUTEX_INITIALIZER;
	memcpy(&timer->timer_mutex, &blank_mutex, sizeof(timer->timer_mutex));
	

	return timer;
}

static void* ntyTimerDtor(void *_self) {
	return _self;
}

static int ntyStartTimerOpera(void *_self, HANDLE_TIMER fun) {
	NetworkTimer *timer = _self;
	struct itimerval tick;
	timer->timerFunc = fun;

	signal(timer->sigNum, timer->timerFunc);
	memset(&tick, 0, sizeof(tick));

	tick.it_value.tv_sec = 0;
	tick.it_value.tv_usec = MS(TIMER_TICK);

	tick.it_interval.tv_sec = 0;
	tick.it_interval.tv_usec = MS(TIMER_TICK);

	pthread_mutex_lock(&timer->timer_mutex);
	if (setitimer(ITIMER_REAL, &tick, NULL) < 0) {
		printf("Set timer failed!\n");
		pthread_mutex_unlock(&timer->timer_mutex);
		return -1;
	}
	return 0;
}


static int ntyStopTimerOpera(void *_self) {
	NetworkTimer *timer = _self;

	struct itimerval tick;

	signal(timer->sigNum, timer->timerFunc);
	memset(&tick, 0, sizeof(tick));

	tick.it_value.tv_sec = 0;
	tick.it_value.tv_usec = 0;//MS(TIMER_TICK);

	tick.it_interval.tv_sec = 0;
	tick.it_interval.tv_usec = 0;//MS(TIMER_TICK);

	pthread_mutex_unlock(&timer->timer_mutex);
	if (setitimer(ITIMER_REAL, &tick, NULL) < 0) {
		printf("Set timer failed!\n");		
		return -1;
	}
	
	return 0;
}




static const TimerOpera ntyTimerOpera = {
	sizeof(NetworkTimer),
	ntyTimerCtor,
	ntyTimerDtor,
	ntyStartTimerOpera,
	ntyStopTimerOpera,
};

const void *pNtyTimerOpera = &ntyTimerOpera;

static void* pNetworkTimer = NULL;
void *ntyNetworkTimerInstance(void) {
	if (pNetworkTimer == NULL) {
		pNetworkTimer = New(pNtyTimerOpera);
	}
	return pNetworkTimer;
}


int ntyStartTimer(void *self,  HANDLE_TIMER func) {
	const TimerOpera* const *handle = self;
	if (self && (*handle) && (*handle)->start) {
		return (*handle)->start(self, func);
	}
	return -2;
}

int ntyStopTimer(void *self) {
	const TimerOpera* const *handle = self;
	if (self && (*handle) && (*handle)->stop) {
		return (*handle)->stop(self);
	}
	return -2;
}

void ntyNetworkTimerRelease(void *self) {
	
	return Delete(self);
}


