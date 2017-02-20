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


#include "NattyTimer.h"
#include "NattyResult.h"

static NSTimerList *nGlobalTimer;


static void ntySignalAlarmCb(int signo) {
	NSTimer *node = nGlobalTimer->header.lh_first;

	while (node != NULL) {
		NSTimer *cnode = node;
		node = node->entries.le_next;

		cnode->elapse++;
		if (cnode->enable == 0) {
			LIST_REMOVE(cnode, entries);
			nGlobalTimer->num--;
#if 1 //user_data store in memory addr
			if (cnode->user_data != NULL)
				free(cnode->user_data);
#endif			
			free(cnode);
			continue;
		}

		if(cnode->elapse >= cnode->interval) {
			cnode->elapse = 0;
			cnode->cb(cnode->id, (void*)cnode->user_data, cnode->len);
		}
	}

}


static int ntyWheelTimerInitialize(NSTimerList *nTimerList, int count)
{
	int ret = 0;
	//NSTimerList 
	
	if(count <= 0 || count > NTY_MAX_TIMER_NUM) {
		printf("the timer max number MUST less than %d.\n", NTY_MAX_TIMER_NUM);
		return -1;
	}
	
	memset(nTimerList, 0, sizeof(NSTimerList));
	LIST_INIT(&nTimerList->header);
	nTimerList->max_num = count;	

	/* Register our internal signal handler and store old signal handler */
	if ((nTimerList->old_sigfunc = signal(SIGALRM, ntySignalAlarmCb)) == SIG_ERR) {
		return -1;
	}
	nTimerList->new_sigfunc = ntySignalAlarmCb;
	//pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;
	//memcpy(&nTimerList->timer_mutex, &blank_mutex, sizeof(nTimerList->timer_mutex));

	/* Setting our interval timer for driver our mutil-timer and store old timer value */
	nTimerList->value.it_value.tv_sec = NTY_TIMER_START;
	nTimerList->value.it_value.tv_usec = 0;
	nTimerList->value.it_interval.tv_sec = NTY_TIMER_TICK;
	nTimerList->value.it_interval.tv_usec = 0;
	ret = setitimer(ITIMER_REAL, &nTimerList->value, &nTimerList->ovalue);

	return ret;
}

static int ntyWheelTimerDestroy(NSTimerList *nTimerList)
{
	NSTimer *node = NULL;
	
	if ((signal(SIGALRM, nTimerList->old_sigfunc)) == SIG_ERR) {
		return -1;
	}

	if((setitimer(ITIMER_REAL, &nTimerList->ovalue, &nTimerList->value)) < 0) {
		return -1;
	}
	
	while (!LIST_EMPTY(&nTimerList->header)) {/* Delete. */
		node = LIST_FIRST(&nTimerList->header);
		LIST_REMOVE(node, entries);
		/* Free node */
		printf("Remove id %d\n", node->id);
#if 1
		free(node->user_data);
#endif
		free(node);
	}
	
	memset(nTimerList, 0, sizeof(NSTimerList));

	return 0;
}


static NSTimer* ntyWheelTimerAdd(NSTimerList *nTimerList, int interval, NFTIMER_EXPIRY_FUNC *cb, void *user_data, int len)
{
	NSTimer *node = NULL;	
	pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;

	if (cb == NULL || interval <= 0) {
		return NULL;
	}

	if(nTimerList->num < nTimerList->max_num) {
		nTimerList->num++;
	} else {
		return NULL;
	} 
	
	if((node = malloc(sizeof(NSTimer))) == NULL) {
		return NULL;
	}
	if(user_data != NULL) {
#if 1
		node->user_data = malloc(len);
		memcpy(node->user_data, user_data, len);
		node->len = len;
#else
		node->user_data = (unsigned long)user_data;
		node->len = sizeof(void*);
#endif
	} else {
		node->user_data = NULL;
		node->len = 0;
	}

	node->cb = cb;
	node->interval = interval;
	node->elapse = 0;
	node->id = nTimerList->num;
	node->enable = 1;
	//memcpy(&timer_mutex[node->id], &blank_mutex, sizeof(timer_mutex[node->id]));
	
	LIST_INSERT_HEAD(&nTimerList->header, node, entries);
	
	return node;
}

static int ntyWheelTimerDel(NSTimerList *nTimerList, NSTimer *node) {
	ASSERT(node != NULL);
	if (node->id <0 || node->id > nTimerList->max_num) {
		return NTY_RESULT_FAILED;
	}
	node->enable = 0;
	
	return NTY_RESULT_SUCCESS;
}


static void* ntyTimerCtor(void *self, va_list *params) {
	NWTimer* nTimer = self;
	nTimer->nContainer = (NSTimerList*)malloc(sizeof(NSTimerList));
	
	ntyWheelTimerInitialize(nTimer->nContainer, NTY_MAX_TIMER_NUM);

	nGlobalTimer = nTimer->nContainer;

	return nTimer;
}

static void* ntyTimerDtor(void *self) {
	NWTimer* nTimer = self;
	ntyWheelTimerDestroy(nTimer->nContainer);

	free(nTimer->nContainer);
	nTimer->nContainer = NULL;

	return nTimer;
}

static void* ntyTimerAddHandle(void *self, int interval, NFTIMER_EXPIRY_FUNC *cb, void *user_data, int len) {
	NWTimer* nTimer = self;

	return ntyWheelTimerAdd(nTimer->nContainer, interval, cb, user_data, len);
}

static int ntyTimerDelHandle(void *self, void *timer) {
	NWTimer* nTimer = self;

	return ntyWheelTimerDel(nTimer->nContainer, timer);
}

static const NWTimerHandle ntyTimerHandle  = {
	sizeof(NWTimer),
	ntyTimerCtor,
	ntyTimerDtor,
	ntyTimerAddHandle,
	ntyTimerDelHandle,
};

const void *pNtyTimerHandle = &ntyTimerHandle;

static void *pWheelTimer = NULL; //Singleton

void* ntyTimerInstance(void) {
	if (pWheelTimer == NULL) {
		int arg = 1;
		void *pTimer = New(pNtyTimerHandle, arg);
		if ((unsigned long)NULL != cmpxchg((void*)(&pWheelTimer), (unsigned long)NULL, (unsigned long)pTimer, WORD_WIDTH)) {
			Delete(pTimer);
		} 
	}
	return pWheelTimer;
}

void ntyTimerRelease(void) {
	if (pWheelTimer != NULL) {
		Delete(pWheelTimer);
		pWheelTimer = NULL;
	}
}

void* ntyTimerAdd(void *self, int interval, NFTIMER_EXPIRY_FUNC *cb, void *user_data, int len) {
	NWTimerHandle * const * pTimerHandle = self;

	if (self && *pTimerHandle && (*pTimerHandle)->add) {
		return (*pTimerHandle)->add(self, interval, cb, user_data, len);
	}
	return NULL;
}

int ntyTimerDel(void *self, void *timer) {
	NWTimerHandle * const * pTimerHandle = self;

	if (self && *pTimerHandle && (*pTimerHandle)->del) {
		return (*pTimerHandle)->del(self, timer);
	}
	return NTY_RESULT_FAILED;
}



