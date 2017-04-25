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


#include "NattyDaveMQ.h"
#include "NattyThreadPool.h"
#include "NattyUtils.h"
#include "NattyHttpCurl.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define JEMALLOC_NO_DEMANGLE 1
#define JEMALLOC_NO_RENAME	 1
#include <jemalloc/jemalloc.h>


static void ntyDaveMqPullMessage(void *arg);
extern int ntyThreadPoolPush(void *self, void *task);

DaveQueue* ntyDaveQueueInitialize(DaveQueue *Queue) {
	//Queue = (DaveQueue *)malloc(sizeof(DaveQueue));

	Queue->nil = (DaveNode*)malloc(sizeof(DaveNode));
	if (Queue->nil == NULL) return NULL;
	
	memset(Queue->nil, 0, sizeof(DaveNode));

	Queue->head = Queue->nil;
	Queue->tail = Queue->nil;

	return Queue;
}

void ntyDaveQueueDestroy(DaveQueue *Queue) {
	free(Queue->nil);
	free(Queue);
}

void ntyDaveEnQueue(DaveQueue *Queue, VALUE_TYPE *val) {
	DaveNode *tail = Queue->nil;
	DaveNode *node = (DaveNode*)malloc(sizeof(DaveNode));
	if (node == NULL) return ;
	
	memset(node, 0, sizeof(DaveNode));
	node->value = val;
	node->next = Queue->nil;
	node->prev = Queue->nil;

	
	while (1) {
		//if (Queue->head == Queue->nil) Queue->head = node;
		cmpxchg((void*)(&Queue->head), (unsigned long)Queue->nil, (unsigned long)node, WORD_WIDTH);

		tail = Queue->tail;
		node->next = Queue->tail;
		Queue->tail->prev = node;
		
        if ((unsigned long)tail == cmpxchg((void*)(&Queue->tail), (unsigned long)tail, (unsigned long)node, WORD_WIDTH)) {
			break;
        } else {
			ntylog(" enqueue cmpxchg failed");
		}
	}
	
}

DaveNode* ntyDaveDeQueue(DaveQueue *Queue) {
    DaveNode *head = Queue->nil, *node = Queue->nil;

	while(1) {
		if (Queue->head == Queue->nil) break;
		head = Queue->head;
		node = Queue->head->prev;

		if ((unsigned long)head == cmpxchg((void*)(&Queue->head), (unsigned long)head, (unsigned long)node, 4)) {
			break;
		} else {
			ntylog(" dequeue cmpxchg failed\n");
		}
	} 

	return head;
}


void *ntyDaveQueueCtor(void *self) {
	return ntyDaveQueueInitialize(self);
}

void *ntyDaveQueueDtor(void *self) {
	ntyDaveQueueDestroy(self);
	return self;
}

void ntyDaveQueueEnqueue(void *self, VALUE_TYPE *value) {
	ntyDaveEnQueue(self, value);
}

void *ntyDaveQueueDequeue(void *self) {
	return ntyDaveDeQueue(self);
}

static const DaveQueueHandle ntyDaveQueueHandle = {
	sizeof(DaveQueue),
	ntyDaveQueueCtor,
	ntyDaveQueueDtor,
	ntyDaveQueueEnqueue,
	ntyDaveQueueDequeue,
};

const void *pNtyDaveQueueHandle = &ntyDaveQueueHandle;
static void *pDaveQueue = NULL;


void *ntyDaveQueueInstance(void) {
	if (pDaveQueue == NULL) {
		void *pDQHandle = New(pNtyDaveQueueHandle);
		if ((unsigned long)NULL != cmpxchg((void*)(&pDaveQueue), (unsigned long)NULL, (unsigned long)pDQHandle, WORD_WIDTH)) {
			Delete(pDQHandle);
		}
	}
	
	return pDaveQueue;
}

void ntyDaveQueueRelease(void *self) {	
	Delete(self);
	pDaveQueue = NULL;
}

static void* ntyEnQueueThread(void *arg) {
	DaveQueue *Queue = (DaveQueue*)arg;
	int i = 0;
	//VALUE_TYPE Array[10] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

	//usleep(10);
	while (1) {
		for (i = 0;i < 10;i ++) {
			//ntylog("%d ", Array[i]);
			//ntyDaveEnQueue(Queue, &Array[i]);
			//if (i == 5) usleep(10);
		}
		ntylog("\n");
	}
}

static void* ntyDeQueueThread(void *arg) {
	DaveQueue *Queue = (DaveQueue*)arg;
	DaveNode *node = Queue->nil;

	ntylog(" ntyDeQueueThread --> Start\n");
	//usleep(10);
	while(1) {
		//ntylog("ntyDaveDeQueue <--- ");
		node = ntyDaveDeQueue(Queue);
		if (node != Queue->nil &&  node != NULL) {
			ntylog(" ntyDeQueueThread --> ntyDaveDeQueue Node\n");
			ntyDaveMqPullMessage(node->value);
			free(node);
		}
		usleep(10);
	}

	ntylog("\n");
}

const ThreadPoolOpera ntyThreadPoolHandle = {
	sizeof(ThreadPool),
	ntyThreadPoolCtor,
	ntyThreadPoolDtor,
	ntyThreadPoolAddJob,
};


//extern const ThreadPoolOpera ntyThreadPoolOpera;
//const void *pNtyDaveMqWorker = &ntyThreadPoolOpera;
const void *pNtyDaveMqWorker = &ntyThreadPoolHandle;

static void *pDaveMqWorker = NULL;
void *ntyDaveMqWorkerInstance(void) {

	if (pDaveMqWorker == NULL) {
		int param = 0;
		void *pWorker = New(pNtyDaveMqWorker, param);
		if ((unsigned long)NULL != cmpxchg((void*)(&pDaveMqWorker), (unsigned long)NULL, (unsigned long)pWorker, WORD_WIDTH)) {
			Delete(pWorker);
		}
	}
	return pDaveMqWorker;

}

void ntyDaveMqWorkerRelease(void) {
#if 1
	Delete(pDaveMqWorker);
	pDaveMqWorker = NULL;
#endif
}

void ntyDaveMqStart(void) {
	pthread_t thread;
	int rc = -1;

	void *worker =ntyDaveMqWorkerInstance();
	
	ntylog("\n ... Dave Message Queue Start ...\n");
	void *Queue= ntyDaveQueueInstance();

	rc = pthread_create(&thread, NULL, ntyDeQueueThread, (void*)Queue);
	if (rc) {
		ntylog("ERROR; return code is %d\n", rc);
	}
	usleep(10);
}

void ntyDaveMqEnd(void) {
	void *Queue = ntyDaveQueueInstance();

	ntyDaveQueueRelease(Queue);
}

static void ntyDaveMqHandleQJKFallen(Job *job) {
	VALUE_TYPE *tag = (VALUE_TYPE*)job->user_data;

	ntyHttpQJKFallen(tag);
	
	ntyFree(job);
}

static void ntyDaveMqHandleGaoWifiLocation(Job *job) {
	VALUE_TYPE *tag = (VALUE_TYPE*)(job->user_data);

	ntyHttpGaodeWifiCellAPI(tag);
	ntyFree(job);
}

static void ntyDaveMqHandleMTKQuickLocation(Job *job) {
	VALUE_TYPE *tag = (VALUE_TYPE*)job->user_data;
	
	ntyHttpMtkQuickLocation(tag);
	ntyFree(job);
}

/* add */

static void ntyDaveMqHandleCallback(Job *job) {
	VALUE_TYPE *tag = (VALUE_TYPE*)job->user_data;
	
	tag->cb(tag); //callback
	
	ntyFree(job);
} 




int ntyDaveMqEnQueue(void *Queue, void *arg) {
	DaveQueueHandle * const * handle = Queue;

	if (Queue && (*handle) && (*handle)->enqueue) {
		(*handle)->enqueue(Queue, arg);
		return 0;
	}
	ntylog("enqueue is not exist\n");
	return -1;
}
#if 0
int ntyDaveMqPushMessage(MESSAGE_TYPE type, C_DEVID fromId, C_DEVID toId, U8 *data, int length, DAVE_MESSAGE_CALLBACK cb) {
	void *Queue = ntyDaveQueueInstance();
	if (Queue == NULL) return -2;

	return ntyDaveMqEnQueue(Queue, fromId, toId, type, data, length);
}
#endif

static void ntyDaveMqPullMessage(void *arg) {
	VALUE_TYPE *tag = arg;
	ntylog(" ntyDaveMqPullMessage --> start \n");
	void *worker = ntyDaveMqWorkerInstance();
	Job *job = (Job*)malloc(sizeof(Job));
	if (job == NULL) {
		ntylog(" job == NULL");
		return ;
	}
	memset(job, 0, sizeof(Job));
	ntylog(" ntyDaveMqPullMessage --> type:%d\n", tag->Type);

	if (MSG_TYPE_START <= tag->Type && MSG_TYPE_END >= tag->Type) {
		job->job_function = ntyDaveMqHandleCallback;
		job->user_data = tag;
		ntyThreadPoolPush(worker, job);
	} else {
		ntylog("Message Type is not Support\n");
		free(tag);
		free(job);
	}

}



int ntyDaveMqPushMessage(VALUE_TYPE *tag) {
	void *Queue = ntyDaveQueueInstance();
	if (Queue == NULL) return -2;

	return ntyDaveMqEnQueue(Queue, tag);
}



#if 0

void *PrintHello(void *args)
{
    int thread_arg = 4;
    sleep(1);
    //thread_arg = (int)(*((int*)args));
    ntylog("Hello from thread %d\n", thread_arg);
    return NULL;
}


int ticks = 2;

#define NUM_THREADS 20

int main () {
#if 0
	int old = ticks-1;
	int next = ticks+1;
	//int ret = cmpxchg(&ticks, next);
	int ret = cmpxchg(&ticks, old, next, 4);
	if (old == ret) {
		ntylog("old is not changed, Success, ticks:%d\n", ticks);
	} else {
		ntylog("old is changed , failed, ticks:%d\n", ticks);
	}
	
	ntylog(" ret : %d\n", ret);
#elif 0
	DaveQueue *Queue;
	DaveNode *node;

	int i = 0;
	VALUE_TYPE Array[10] = {4, 7, 23, 56, 2, 76, 65, 36, 20, 91};

	Queue = ntyDaveQueueInitialize(Queue);
	
	for (i = 0;i < 10;i ++) {
		ntylog("%d,", Array[i]);
		ntyDaveEnQueue(Queue, Array[i]);
	}

	ntylog("\n");
	node = Queue->head;
	for (i = 0;i < 15;i ++) {
		if (node != Queue->nil &&  node != NULL)
			ntylog("%d ", node->value);
		else
			break;
		node = node->prev;
	}

	ntylog("\n");
	node = Queue->tail;
	for (i = 0;i < 15;i ++) {
		if (node != Queue->nil &&  node != NULL)
			ntylog("%d ", node->value);
		else
			break;
		node = node->next;
	}

	ntylog("\n");
	node = Queue->head;
	for (i = 0;i < 15;i ++) {
		node = ntyDaveDeQueue(Queue);
		if (node != Queue->nil &&  node != NULL)
			ntylog("%d ", node->value);
		else 
			break;

		free(node);
	}

	ntylog("\n end \n");
	ntyDaveQueueRelease(Queue);

#else
	int i = 0, rc;
	pthread_t thread[NUM_THREADS];

	DaveQueue *Queue;
	Queue = ntyDaveQueueInitialize(Queue);

	ntylog("ntyDaveQueueInitialize\n");
	
	for (i = 0;i < NUM_THREADS;i ++) {
		if (i % 3) {
			rc = pthread_create(&thread[i], NULL, ntyEnQueueThread, (void*)Queue);
			if (rc) {
				ntylog("ERROR; return code is %d\n", rc);
			}
		} else {
			rc = pthread_create(&thread[i], NULL, ntyDeQueueThread, (void*)Queue);
			if (rc) {
				ntylog("ERROR; return code is %d\n", rc);
			}
		}
		//usleep(1);
	}

	//sleep(5);
    for( i = 0; i < NUM_THREADS; i++)
        pthread_join(thread[i], NULL);
#endif
}
#endif

