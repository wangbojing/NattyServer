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



#ifndef __NATTY_THREAD_POOL_H__ 
#define __NATTY_THREAD_POOL_H__

#include <pthread.h>
#include "NattyAbstractClass.h"

#define NTY_THREAD_POOL_NUM		80


typedef struct _Worker {
	pthread_t thread;
	int terminate;
	struct _WorkQueue *workqueue;
	struct _Worker *prev;
	struct _Worker *next;
} Worker;


typedef struct _Job {
	void (*job_function)(struct _Job *job);
	void *user_data;
	struct _Job *prev;
	struct _Job *next;
} Job;


typedef struct _WorkQueue {
	Worker *workers;
	Job *waiting_jobs;
	pthread_mutex_t jobs_mutex;
	pthread_cond_t jobs_cond;
} WorkQueue;

typedef struct _ThreadPool {
	const void *_;
	WorkQueue *wq;
} ThreadPool;

typedef struct _ThreadPoolOpera {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	void (*addJob)(void *_self, void *task);
} ThreadPoolOpera;

void *ntyThreadPoolInstance(void);
void ntyThreadPoolRelease(void);
int ntyThreadPoolPush(void *self, void *task);



#endif













