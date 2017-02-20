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


#include <signal.h>
#include <sys/time.h>
#include <string.h>

#include <netdb.h>
#include <time.h>
#include <sys/queue.h>

#include <pthread.h>

#include "NattyAbstractClass.h"


#define NTY_MAX_TIMER_NUM			(1<<20)
#define NTY_CURRENT_TIMER_NUM		20
#define NTY_TIMER_START				1
#define NTY_TIMER_TICK				1
#define NTY_INVALID_TIMER_ID		(-1)

typedef unsigned int NITIMER_ID;
typedef int NFTIMER_EXPIRY_FUNC(NITIMER_ID id, void *user_data, int len);


/**
 * The type of the timer
 */
 struct ntimer {
	LIST_ENTRY(ntimer) entries;	/**< list entry		*/	
	
	NITIMER_ID id;			/**< timer id		*/

	int interval;			/**< timer interval(second)*/
	int elapse; 			/**< 0 -> interval 	*/

	NFTIMER_EXPIRY_FUNC *cb;		/**< call if expiry 	*/
#if 1
	//user_data use for storing Client Addr
	//
	void *user_data;		/**< callback arg	*/
	int len;			/**< user_data length	*/
#else
	unsigned long user_data;
#endif

	int enable;
};

//user_data	BPLUESNode *

/**
 * The timer list
 */
struct timer_list {
	LIST_HEAD(listheader, ntimer) header;	/**< list header 	*/
	int num;				/**< timer entry number */
	int max_num;				/**< max entry number	*/

	void (*old_sigfunc)(int);		/**< save previous signal handler */
	void (*new_sigfunc)(int);		/**< our signal handler	*/

	struct itimerval ovalue;		/**< old timer value */
	struct itimerval value;			/**< our internal timer value */
	
	pthread_mutex_t timer_mutex;
};


typedef struct ntimer NSTimer;
typedef struct timer_list NSTimerList;

typedef struct _NWTIMER {
	const void *_;
	NSTimerList *nContainer;
} NWTimer;

typedef struct _NWTIMERHANDLE {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	void* (*add)(void *_self, int interval, NFTIMER_EXPIRY_FUNC *cb, void *user_data, int len);
	int (*del)(void *_self, void *timer);
} NWTimerHandle;


void* ntyTimerInstance(void);
void ntyTimerRelease(void);
void* ntyTimerAdd(void *self, int interval, NFTIMER_EXPIRY_FUNC *cb, void *user_data, int len);
int ntyTimerDel(void *self, void *timer);





#endif


