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


#ifndef __NATTY_CONNECTION_POOL__
#define __NATTY_CONNECTION_POOL__


#include "NattyAbstractClass.h"

#include <mysql/mysql.h>


typedef MYSQL 			nShrimpConn;
typedef MYSQL_RES 		nShrimpRes;
typedef MYSQL_ROW 		nShrimpRow;

typedef MYSQL_STMT 		nShrimpStmt;
typedef MYSQL_BIND 		nShrimpBind;
typedef MYSQL_FIELD 	nShrimpField;

typedef pthread_t 		nThread;
typedef pthread_cond_t	nSem;
typedef pthread_mutex_t	nMutex;
typedef pthread_key_t	nThreadKey;



#if 1 //Code From LIBZDB

#define wrapper(F) do { int status=F; \
        if (status!=0 && status!=ETIMEDOUT) \
                ntylog("Thread: %s\n", strerror(status)); \
        } while (0)
        
#define ntyThreadCreate(thread, threadFunc, threadArgs) \
        wrapper(pthread_create(&thread, NULL, threadFunc, (void*)threadArgs))
#define ntyThreadSelf() pthread_self()
#define ntyThreadDetach(thread) wrapper(pthread_detach(thread))
#define ntyThreadCancel(thread) wrapper(pthread_cancel(thread))
#define ntyThreadJoin(thread) wrapper(pthread_join(thread, NULL))


#define ntySemInit(sem) wrapper(pthread_cond_init(&sem, NULL))
#define ntySemWait(sem, mutex) wrapper(pthread_cond_wait(&sem, &mutex))
#define ntySemSignal(sem) wrapper(pthread_cond_signal(&sem))
#define ntySemBroadcast(sem) wrapper(pthread_cond_broadcast(&sem))
#define ntySemDestroy(sem) wrapper(pthread_cond_destroy(&sem))
#define ntySemTimeWait(sem, mutex, time) \
        wrapper(pthread_cond_timedwait(&sem, &mutex, &time))


#define ntyMutexInit(mutex) wrapper(pthread_mutex_init(&mutex, NULL))
#define ntyMutexDestroy(mutex) wrapper(pthread_mutex_destroy(&mutex))
#define ntyMutexLock(mutex) wrapper(pthread_mutex_lock(&mutex))
#define ntyMutexUnlock(mutex) wrapper(pthread_mutex_unlock(&mutex))

#define LOCK(mutex) do { nMutex *_yymutex=&(mutex); \
        wrapper(pthread_mutex_lock(_yymutex));
#define END_LOCK wrapper(pthread_mutex_unlock(_yymutex)); } while (0)

#define ntyThreadDataCreate(key) wrapper(pthread_key_create(&(key), NULL))
#define ntyThreadDataSet(key, value) pthread_setspecific((key), (value))
#define ntyThreadDataGet(key) pthread_getspecific((key))

#endif


#define NTY_DEFAULT_CONNPOOL_MAX		64
#define NTY_DEFAULT_CONNPOOL_MIN		30
#define NTY_DEFAULT_CONNPOOL_INC		4

#define NTY_DEFAULT_CONNECTION_TIMEOUT	30
#define NTY_DEFAULT_HANDLE_TIMEOUT		2
#define NTY_DEFAULT_REAP_TIMEOUT		1


#define NTY_DEFAULT_CONNPOOL_RATIO		0.6

typedef struct COLUMN {
        my_bool is_null;
        nShrimpField *field;
        unsigned long real_length;
        char *buffer;
} nColumn;

typedef struct NTY_MYSQL_SET {
	int needRebind;
	int columnCount;
	
	nShrimpRes *Res;
	nShrimpBind *Bind;
	nShrimpStmt *Stmt;

	nColumn *columns;
} nSnailsSet, *SnailsSet;

typedef struct NTY_MYSQL_CONN {
	nShrimpConn *Conn;	
	nSnailsSet *Snails;
	nShrimpRow Row;
	TIMESTAMP accessTime;
	
	void *arg;
	U8 enable;
	U8 status;
	U16 index;
	
	long con_lock;
	int error;
	void *Pool;
} nMysqlConn, nShrimp, *Shrimp;


typedef struct NTY_CONNECTION_POOL {

	nThread reaper;
	nSem alarm;
	volatile int stopped; //0 stop, 1 run
	nMutex mutex;

	int connectTimeout;
	int handleTimeout;
	int reapInterval;
	
	int max_num;
	int cur_num;
	int min_num;
	int active;
	nShrimp *ConnList;
	
} nConnPool, *ConnPool;

typedef struct NTY_EXCEPTION {
	U8 *message;
	
} nException;
	

nShrimp *ntyConnPoolGetConnection(nConnPool *Pool);
void ntyConnPoolRetConnection(nShrimp *shrimp);
void ntyConnPoolCheckConnection(nShrimp *shrimp);


long long ntyResultSetgetLLong(nSnailsSet *snail, int columnIndex);
const char* ntyResultSetgetString(nSnailsSet *snail, int columnIndex);
int ntyResultSetgetInt(nSnailsSet *snail, int columnIndex);
time_t ntyResultSetgetTimeStamp(nSnailsSet *snail, int columnIndex);


void *ntyConnPoolInitialize(nConnPool *Pool);
void *ntyConnPoolRelease(nConnPool *Pool);


nSnailsSet* ntyConnPoolExecuteQuery(nShrimp *shrimp, const char *format, ...);
void ntyConnPoolExecute(nShrimp *shrimp, const char *format, ...);

int ntyConnPoolGetSize(nConnPool *Pool);
int ntyConnPoolGetMax(nConnPool *Pool);
int ntyConnPoolGetActive(nConnPool *Pool);






#define TRY 		do {
#define CATCH(e)	goto next;
#define FINALLY		next:
#define END_TRY		} while(0)



typedef Shrimp Connection_T;
typedef SnailsSet ResultSet_T;
typedef ConnPool ConnectionPool_T;
typedef const char* URL_T;


#define ConnectionPool_getConnection	ntyConnPoolGetConnection
#define ResultSet_next					ntyShrimpStmtResultSetNext
#define ResultSet_getInt				ntyResultSetgetInt
#define ResultSet_getString				ntyResultSetgetString
#define ResultSet_getLLong				ntyResultSetgetLLong
#define ResultSet_getTimestamp			ntyResultSetgetTimeStamp

#define Connection_close				ntyConnPoolRetConnection

#define Connection_executeQuery			ntyConnPoolExecuteQuery
#define Connection_execute				ntyConnPoolExecute

#define ConnectionPool_size				ntyConnPoolGetSize
#define ConnectionPool_getMaxConnections				ntyConnPoolGetMax
#define ConnectionPool_active			ntyConnPoolGetActive

#endif





