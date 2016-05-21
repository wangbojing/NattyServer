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

#include <string.h>
#include <stdlib.h>
#include "NattyThreadPool.h"



#define QUEUE_ADD(item, list) {	\
	item->prev = NULL;			\
	item->next = list;			\
	list = item;				\
}

#define QUEUE_REMOVE(item, list) {							\
	if (item->prev != NULL) item->prev->next = item->next;	\
	if (item->next != NULL) item->next->prev = item->prev;	\
	if (list == item) list = item->next;					\
	item->prev = item->next = NULL;							\
}


static void *ntyRunner(void *arg) {
	Worker *worker = (Worker*)arg;
	Job *job = NULL;

	while (1) {
		pthread_mutex_lock(&worker->workqueue->jobs_mutex);
		while (worker->workqueue->waiting_jobs == NULL) {
			if (worker->terminate) break;
			pthread_cond_wait(&worker->workqueue->jobs_cond, &worker->workqueue->jobs_mutex);
		}

		if (worker->terminate) break;

		job = worker->workqueue->waiting_jobs;
		if (job != NULL) {
			QUEUE_REMOVE(job, worker->workqueue->waiting_jobs);
		}
		pthread_mutex_unlock(&worker->workqueue->jobs_mutex);

		if (job == NULL) continue;
		job->job_function(job);
	}
	free(worker);
	pthread_exit(NULL);
}

static int ntyWorkQueueInit(WorkQueue *wq, int numWorkers) {
	int i = 0;
	Worker *worker;
	pthread_cond_t blank_cond = PTHREAD_COND_INITIALIZER;
	pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;

	if (numWorkers < 1) numWorkers = 1;
	memset(wq, 0, sizeof(WorkQueue));
	memcpy(&wq->jobs_mutex, &blank_mutex, sizeof(wq->jobs_mutex));
	memcpy(&wq->jobs_cond, &blank_mutex, sizeof(wq->jobs_cond));

	for (i = 0;i < numWorkers;i ++) {
		if ((worker = malloc(sizeof(Worker))) == NULL) {
			perror("Failed to allocate all workers");
			return 1;
		}
		memset(worker, 0, sizeof(Worker));
		worker->workqueue = wq;
		if (pthread_create(&worker->thread, NULL, ntyRunner, (void*)worker)) {
			perror("Failed to start all worker threads");
			free(worker);

			return 1;
		}
		QUEUE_ADD(worker, worker->workqueue->workers);		
	}
	
	return 0;
}

static void ntyWorkQueueShutdown(WorkQueue *wq) {
	Worker *worker = NULL;
	for (worker = wq->workers;worker != NULL;worker = worker->next) {
		worker->terminate = 1;
	}

	pthread_mutex_lock(&wq->jobs_mutex);
	wq->workers = NULL;
	wq->waiting_jobs = NULL;
	pthread_cond_broadcast(&wq->jobs_cond);
	pthread_mutex_unlock(&wq->jobs_mutex);
}

static void ntyWorkQueueAddJob(WorkQueue *wq, Job *job) {
	pthread_mutex_lock(&wq->jobs_mutex);
	QUEUE_ADD(job, wq->waiting_jobs);
	pthread_cond_broadcast(&wq->jobs_cond);
	pthread_mutex_unlock(&wq->jobs_mutex);
}



static void* ntyThreadPoolCtor(void *_self, va_list *params) {
	ThreadPool *pool = (ThreadPool*)_self;

	pool->wq = (WorkQueue*)malloc(sizeof(WorkQueue));
	ntyWorkQueueInit(pool->wq, NTY_THREAD_POOL_NUM);

	return pool;
}
static void* ntyThreadPoolDtor(void *_self) {
	ThreadPool *pool = (ThreadPool*)_self;

	ntyWorkQueueShutdown(pool->wq);
	free(pool->wq);

	return pool;
}


static void ntyThreadPoolAddJob(void *_self, void *task) {
	ThreadPool *pool = (ThreadPool*)_self;
	Job *job = (Job*)task;
	
	ntyWorkQueueAddJob(pool->wq, job);
}

static const ThreadPoolOpera ntyThreadPoolOpera = {
	sizeof(ThreadPool),
	ntyThreadPoolCtor,
	ntyThreadPoolDtor,
	ntyThreadPoolAddJob,
};

const void *pNtyThreadPoolOpera = &ntyThreadPoolOpera;


static void *pThreadPool = NULL;

void *ntyThreadPoolInstance(void) {
	if (pThreadPool == NULL) {
		pThreadPool = New(pNtyThreadPoolOpera);
	}
	return pThreadPool;
}

void ntyThreadPoolRelease(void) {	
	return Delete(pThreadPool);
}

int ntyThreadPoolPush(void *self, void *task) {
	ThreadPoolOpera **pThreadPoolOpera = self;

	if (self && (*pThreadPoolOpera) && (*pThreadPoolOpera)->addJob) {
		(*pThreadPoolOpera)->addJob(self, task);
		return 0;
	}
	return 1;
}



