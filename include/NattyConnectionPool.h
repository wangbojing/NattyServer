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


typedef MYSQL nShrimpConn;
typedef MYSQL_RES nShrimpRes;
typedef MYSQL_ROW nShrimpRow;

typedef MYSQL_STMT nShrimpStmt;
typedef MYSQL_BIND nShrimpBind;
typedef MYSQL_FIELD nShrimpField;



#define NTY_DEFAULT_CONNPOOL_MAX		64
#define NTY_DEFAULT_CONNPOOL_MIN		30
#define NTY_DEFAULT_CONNPOOL_INC		4

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
	
	void *arg;
	U8 enable;
	U8 status;
	U16 index;
	
	long con_lock;
	int error;
	void *Pool;
} nMysqlConn, nShrimp, *Shrimp;


typedef struct NTY_CONNECTION_POOL {
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





