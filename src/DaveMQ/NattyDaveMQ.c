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

extern int ntyThreadPoolPush(void *self, void *task);

DaveQueue* ntyDaveQueueInitialize(DaveQueue *Queue) {
	//Queue = (DaveQueue *)malloc(sizeof(DaveQueue));

	Queue->nil = (DaveNode*)malloc(sizeof(DaveNode));
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
	
	//usleep(10);
	while(1) {
		//ntylog("ntyDaveDeQueue <--- ");
		node = ntyDaveDeQueue(Queue);
		if (node != Queue->nil &&  node != NULL) {
			//ntylog("%d ", node->value);
			ntyDaveMqPushWorker(node->value);
			free(node);
		}
		usleep(10);
	}

	ntylog("\n");
}

extern const ThreadPoolOpera ntyThreadPoolOpera;
const void *pNtyDaveMqWorker = &ntyThreadPoolOpera;


static void *pDaveMqWorker = NULL;
void *ntyDaveMqWorkerInstance(void) {
	if (pDaveMqWorker == NULL) {
		void *pWorker = New(pNtyDaveMqWorker);
		if ((unsigned long)NULL != cmpxchg((void*)(&pDaveMqWorker), (unsigned long)NULL, (unsigned long)pWorker, WORD_WIDTH)) {
			Delete(pWorker);
		}
	}
	return pDaveMqWorker;
}

void ntyDaveMqWorkerRelease(void) {
	Delete(pDaveMqWorker);
}

void ntyDaveMqStart(void) {
	pthread_t thread;
	int rc = -1;
	ntydbg("\n ... Dave Message Queue Start ...\n");
	void *Queue= ntyDaveQueueInstance();

	rc = pthread_create(&thread, NULL, ntyDeQueueThread, (void*)Queue);
	if (rc) {
		ntylog("ERROR; return code is %d\n", rc);
	}
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


void ntyDaveMqPushWorker(void *arg) {
	VALUE_TYPE *tag = arg;
	void *worker = ntyDaveMqWorkerInstance();
	Job *job = (Job*)malloc(sizeof(Job));

	if (MSG_TYPE_QJK_FALLEN == tag->Type) {
		job->job_function = ntyDaveMqHandleQJKFallen;
		job->user_data = tag;
		ntyThreadPoolPush(worker, job);
	} else if (MSG_TYPE_GAODE_WIFI_CELL_API == tag->Type) {
		job->job_function = ntyDaveMqHandleGaoWifiLocation;
		job->user_data = tag;
		ntyThreadPoolPush(worker, job);
	} else if (MSG_TYPE_MTK_QUICKLOCATION == tag->Type) {
		job->job_function = ntyDaveMqHandleMTKQuickLocation;
		job->user_data = tag;
		ntyThreadPoolPush(worker, job);
	} else {
		ntylog("Message Type is not Support\n");
		free(tag);
		free(job);
	}
	
}

int ntyDaveMqEnQueue(void *Queue, C_DEVID fromId, C_DEVID toId, MESSAGE_TYPE type, U8 *data, int length) {
	DaveQueueHandle * const * handle = Queue;

	if (Queue && (*handle) && (*handle)->enqueue) {
		VALUE_TYPE *tag = (VALUE_TYPE*)malloc(sizeof(VALUE_TYPE));
		memcpy(tag->Tag, data, length);
		tag->length = length;
		tag->Type = type;
		tag->fromId = fromId;
		tag->toId = toId;

		(*handle)->enqueue(Queue, tag);
#if 0
		ntyDeQueueThread(Queue);
#endif
		return 0;
	}
	ntylog("enqueue is not exist\n");
	return -1;
}

int ntyPushDaveMessageQueue(MESSAGE_TYPE type, C_DEVID fromId, C_DEVID toId, U8 *data, int length) {
	void *Queue = ntyDaveQueueInstance();
	if (Queue == NULL) return -2;

	return ntyDaveMqEnQueue(Queue, fromId, toId, type, data, length);
}


int ntyClassifyMessageType(C_DEVID fromId, C_DEVID toId, U8 *data, int length) {
	int i = 0;
	
	if (length <= 4) return -5;
	
#if 1
	for (i = 5;i < length;i ++) {
		if (data[i] == ' ') {
			data[i] = '\0';
			break;
		}
	}
	
#endif

	if (0 == strncmp(NTY_HTTP_GET_HANDLE_STRING, data, 3)) {
		if (0 == strncmp(data+4, HTTP_QJK_BASE_URL, strlen(HTTP_QJK_BASE_URL))) {
			return ntyPushDaveMessageQueue(MSG_TYPE_QJK_FALLEN, fromId, toId, data+4, length);
		} else if (0 == strncmp(data+4, HTTP_GAODE_BASE_URL, strlen(HTTP_GAODE_BASE_URL))) {
			return ntyPushDaveMessageQueue(MSG_TYPE_GAODE_WIFI_CELL_API, fromId, toId, data+4, length);
		}
	} else if (0 == strncmp(NTY_HTTP_POST_HANDLE_STRING, data, 4)) {

	} else if (0 == strncmp(NTY_HTTP_RET_HANDLE_STRING, data, 3)) {
		if (0 == strncmp(data+4, HTTP_GAODE_BASE_URL, strlen(HTTP_GAODE_BASE_URL))) {
			return ntyPushDaveMessageQueue(MSG_TYPE_MTK_QUICKLOCATION, fromId, toId, data, length);
		}
	} else {
		return -4;
	}
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

