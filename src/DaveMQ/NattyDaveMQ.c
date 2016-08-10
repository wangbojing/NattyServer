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



//#include <arch/atomic.h>

#include "NattyDaveMQ.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>



unsigned long cmpxchg(void *addr, unsigned long _old, unsigned long _new, int size) {
	unsigned long prev;
	volatile unsigned int *_ptr = (volatile unsigned int *)(addr);

	switch (size) {
		case 1: {
			__asm__ volatile (
		        "lock; cmpxchgb %b1, %2"
		        : "=a" (prev)
		        : "r" (_new), "m" (*_ptr), "0" (_old)
		        : "memory");
			break;
		}
		case 2: {
			__asm__ volatile (
		        "lock; cmpxchgw %w1, %2"
		        : "=a" (prev)
		        : "r" (_new), "m" (*_ptr), "0" (_old)
		        : "memory");
			break;
		}
		case 4: {
			__asm__ volatile (
		        "lock; cmpxchgl %1, %2"
		        : "=a" (prev)
		        : "r" (_new), "m" (*_ptr), "0" (_old)
		        : "memory");
			break;
		}
	}

	return prev;
}




DaveQueue* ntyDaveQueueInitialize(DaveQueue *Queue) {
	Queue = (DaveQueue *)malloc(sizeof(DaveQueue));

	Queue->nil = (DaveNode*)malloc(sizeof(DaveNode));
	memset(Queue->nil, 0, sizeof(DaveNode));

	Queue->head = Queue->nil;
	Queue->tail = Queue->nil;

	return Queue;
}

void ntyDaveQueueRelease(DaveQueue *Queue) {
	free(Queue->nil);
	free(Queue);
}


void ntyDaveEnQueue(DaveQueue *Queue, VALUE_TYPE val) {
	DaveNode *tail = Queue->nil;
	DaveNode *node = (DaveNode*)malloc(sizeof(DaveNode));
	
	memset(node, 0, sizeof(DaveNode));
	node->value = val;
	node->next = Queue->nil;
	node->prev = Queue->nil;

	
	while (1) {
		if (Queue->head == Queue->nil) Queue->head = node;
		
		tail = Queue->tail;
		node->next = Queue->tail;
		Queue->tail->prev = node;
		
        if ((unsigned long)tail == cmpxchg((void*)(&Queue->tail), (unsigned long)tail, (unsigned long)node, 4)) {
            break;
        } else {
			printf(" enqueue cmpxchg failed");
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
			printf(" dequeue cmpxchg failed");
		}
	} 

	return head;
}

static void* ntyEnQueueThread(void *arg) {
	DaveQueue *Queue = (DaveQueue*)arg;
	int i = 0;
	VALUE_TYPE Array[10] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

	printf("ntyEnQueueThread: ");
	//usleep(10);
	while (1) {
		for (i = 0;i < 10;i ++) {
			printf("%d ", Array[i]);
			ntyDaveEnQueue(Queue, Array[i]);
			//if (i == 5) usleep(10);
		}
		printf("\n");
	}
}

static void* ntyDeQueueThread(void *arg) {
	DaveQueue *Queue = (DaveQueue*)arg;
	DaveNode *node;

	printf("ntyDeQueueThread --> ");
	//usleep(10);
	while(1) {
		node = ntyDaveDeQueue(Queue);
		if (node != Queue->nil &&  node != NULL)
			printf("%d ", node->value);
		else 
			break;

		free(node);
	}
	printf("\n");
}

void *PrintHello(void *args)
{
    int thread_arg = 4;
    sleep(1);
    //thread_arg = (int)(*((int*)args));
    printf("Hello from thread %d\n", thread_arg);
    return NULL;
}


#if 1
int ticks = 2;

#define NUM_THREADS 20

int main () {
#if 0
	int old = ticks-1;
	int next = ticks+1;
	//int ret = cmpxchg(&ticks, next);
	int ret = cmpxchg(&ticks, old, next, 4);
	if (old == ret) {
		printf("old is not changed, Success, ticks:%d\n", ticks);
	} else {
		printf("old is changed , failed, ticks:%d\n", ticks);
	}
	
	printf(" ret : %d\n", ret);
#elif 0
	DaveQueue *Queue;
	DaveNode *node;

	int i = 0;
	VALUE_TYPE Array[10] = {4, 7, 23, 56, 2, 76, 65, 36, 20, 91};

	Queue = ntyDaveQueueInitialize(Queue);
	
	for (i = 0;i < 10;i ++) {
		printf("%d,", Array[i]);
		ntyDaveEnQueue(Queue, Array[i]);
	}

	printf("\n");
	node = Queue->head;
	for (i = 0;i < 15;i ++) {
		if (node != Queue->nil &&  node != NULL)
			printf("%d ", node->value);
		else
			break;
		node = node->prev;
	}

	printf("\n");
	node = Queue->tail;
	for (i = 0;i < 15;i ++) {
		if (node != Queue->nil &&  node != NULL)
			printf("%d ", node->value);
		else
			break;
		node = node->next;
	}

	printf("\n");
	node = Queue->head;
	for (i = 0;i < 15;i ++) {
		node = ntyDaveDeQueue(Queue);
		if (node != Queue->nil &&  node != NULL)
			printf("%d ", node->value);
		else 
			break;

		free(node);
	}

	printf("\n end \n");
	ntyDaveQueueRelease(Queue);

#else
	int i = 0, rc;
	pthread_t thread[NUM_THREADS];

	DaveQueue *Queue;
	Queue = ntyDaveQueueInitialize(Queue);

	printf("ntyDaveQueueInitialize\n");
	
	for (i = 0;i < NUM_THREADS;i ++) {
		if (i % 3) {
			rc = pthread_create(&thread[i], NULL, ntyEnQueueThread, (void*)Queue);
			if (rc) {
				printf("ERROR; return code is %d\n", rc);
			}
		} else {
			rc = pthread_create(&thread[i], NULL, ntyDeQueueThread, (void*)Queue);
			if (rc) {
				printf("ERROR; return code is %d\n", rc);
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

