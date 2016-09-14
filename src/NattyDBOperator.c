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


#include "NattyDBOperator.h"
#include "NattyRBTree.h"


void* ntyConnectionPoolInitialize(ConnectionPool *pool) {
	pool->url = URL_new(MYSQL_DB_CONN_STRING);
	pool->nPool = ConnectionPool_new(pool->url);
	ConnectionPool_start(pool->nPool);

	return pool;
}

void* ntyConnectionPoolDestory(ConnectionPool *pool) {
	ConnectionPool_free(&pool->nPool);
	URL_free(&pool->url);

	pool->nPool = NULL;
	pool->url = NULL;
	pool = NULL;

	return pool;
}

void *ntyConnectionPoolCtor(void *self) {
	return ntyConnectionPoolInitialize(self);
}

void *ntyConnectionPoolDtor(void *self) {
	return ntyConnectionPoolDestory(self);
}


static const ConnectionPoolHandle ntyConnectionPoolHandle = {
	sizeof(ConnectionPool),
	ntyConnectionPoolCtor,
	ntyConnectionPoolDtor,
	NULL,
	NULL,
};

const void *pNtyConnectionPoolHandle = &ntyConnectionPoolHandle;
static void *pConnectionPool = NULL;

void *ntyConnectionPoolInstance(void) {
	if (pConnectionPool == NULL) {
		void *pCPool = New(pNtyConnectionPoolHandle);
		if ((unsigned long)NULL != cmpxchg((void*)(&pConnectionPool), (unsigned long)NULL, (unsigned long)pCPool, WORD_WIDTH)) {
			Delete(pCPool);
		}
	}
	
	return pConnectionPool;
}

void ntyConnectionPoolRelease(void *self) {	
	Delete(self);
	pConnectionPool = NULL;
}



#if 0
int ntyConnectionPoolExecute(void *_self, U8 *sql) {
	ConnectionPool *pool = _self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
		
	TRY 
	{
		Connection_execute(con, sql);
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		Connection_close(con);
	}
	END_TRY;

	return ret;
}


int ntyConnectionPoolQuery(void *_self, U8 *sql, void ***result, int *length) {
	ConnectionPool *pool = _self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
		

	TRY 
	{
		ResultSet_T r = Connection_executeQuery(con, sql);
		while (ResultSet_next(r)) {
			
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		Connection_close(con);
	}
	END_TRY;

	return ret;
}
#endif


//NTY_DB_WATCH_INSERT_FORMAT

static int ntyExecuteWatchInsert(void *self, U8 *imei) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		Connection_execute(con, NTY_DB_WATCH_INSERT_FORMAT, imei);
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		Connection_close(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_APPIDLIST_SELECT_FORMAT
static int ntyQueryAppIDListSelect(void *self, C_DEVID did, void *tree) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		ResultSet_T r = Connection_executeQuery(con, NTY_DB_APPIDLIST_SELECT_FORMAT, did);
		while (ResultSet_next(r)) {
			C_DEVID id = ResultSet_getLLong(r, 1);
#if 1
			ntyFriendsTreeInsert(tree, id);
#endif
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		Connection_close(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_WATCHIDLIST_SELECT_FORMAT
static int ntyQueryWatchIDListSelect(void *self, C_DEVID aid, void *tree) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		ResultSet_T r = Connection_executeQuery(con, NTY_DB_WATCHIDLIST_SELECT_FORMAT, aid);
		while (ResultSet_next(r)) {
			C_DEVID id = ResultSet_getLLong(r, 1);
#if 1
			ntyFriendsTreeInsert(tree, id);
#endif
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		Connection_close(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_DEV_APP_INSERT_FORMAT
static int ntyQueryDevAppRelationInsert(void *self, C_DEVID aid, U8 *imei) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		ResultSet_T r = Connection_executeQuery(con, NTY_DB_DEV_APP_INSERT_FORMAT, aid, imei);
		while (ResultSet_next(r)) {
			ret = ResultSet_getInt(r, 1);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		Connection_close(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_DEV_APP_DELETE_FORMAT
static int ntyExecuteDevAppRelationDelete(void *self, C_DEVID aid, C_DEVID did) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		Connection_execute(con, NTY_DB_DEV_APP_DELETE_FORMAT, aid, did);
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		Connection_close(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_LOCATION_INSERT_FORMAT
static int ntyExecuteLocationInsert(void *self, C_DEVID did, U8 *lng, U8 *lat, U8 *info) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		Connection_execute(con, NTY_DB_LOCATION_INSERT_FORMAT, did, lng, lat, info);
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		Connection_close(con);
	}
	END_TRY;

	return ret;
}



//NTY_DB_LOCATION_INSERT_FORMAT
static int ntyExecuteStepInsert(void *self, C_DEVID did, int value) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		Connection_execute(con, NTY_DB_STEP_INSERT_FORMAT, did, value);
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		Connection_close(con);
	}
	END_TRY;

	return ret;
}



//NTY_DB_LOCATION_INSERT_FORMAT
int ntyExecuteHeartRateInsert(void *self, C_DEVID did, int value) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		Connection_execute(con, NTY_DB_HEARTRATE_INSERT_FORMAT, did, value);
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		Connection_close(con);
	}
	END_TRY;

	return ret;
}


int ntyExecuteWatchInsertHandle(U8 *imei) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteWatchInsert(pool, imei);
}

int ntyQueryAppIDListSelectHandle(C_DEVID did, void *tree) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryAppIDListSelect(pool, did, tree);
}


int ntyQueryWatchIDListSelectHandle(C_DEVID aid, void *tree) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryWatchIDListSelect(pool, aid, tree);
}

int ntyQueryDevAppRelationInsertHandle(C_DEVID aid, U8 *imei) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryDevAppRelationInsert(pool, aid, imei);
}

int ntyExecuteDevAppRelationDeleteHandle(C_DEVID aid, C_DEVID did) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteDevAppRelationDelete(pool, aid, did);
}


int ntyExecuteLocationInsertHandle( C_DEVID did, U8 *lng, U8 *lat, U8 *info) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteLocationInsert(pool, did, lng, lat, info);
}

int ntyExecuteStepInsertHandle(C_DEVID did, int value) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteStepInsert(pool, did, value);
}

int ntyExecuteHeartRateInsertHandle(C_DEVID did, int value) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteHeartRateInsert(pool, did, value);
}





