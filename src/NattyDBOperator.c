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

#include <string.h>
#include <wchar.h>

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
	ntyConnectionPoolDynamicsSize(pConnectionPool);
	return pConnectionPool;
}

static void *ntyGetConnectionPoolInstance(void) {
	return pConnectionPool;
}

int ntyConnectionPoolDynamicsSize(void *self) {
	ConnectionPool *pool = self;
	int size = ConnectionPool_size(pool->nPool);
	int active = ConnectionPool_active(pool->nPool);
	int max = ConnectionPool_getMaxConnections(pool->nPool);

	if (size > max * CONNECTION_SIZE_THRESHOLD_RATIO) {
		int n = ConnectionPool_reapConnections(pool->nPool);
		if (n < max * CONNECTION_SIZE_REAP_RATIO) {
			ntylog(" Connection Need to Raise Connection Size\n");
		}
	}
	ntylog(" size:%d, active:%d, max:%d\n", size, active, max);
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


Connection_T ntyCheckConnection(void *self, Connection_T con) {
	ConnectionPool *pool = self;
	if (con == NULL) {
		int n = ConnectionPool_reapConnections(pool->nPool);
		Connection_T con = ConnectionPool_getConnection(pool->nPool);
	}
	return con;
}

void ntyConnectionClose(Connection_T con) {
	if (con != NULL) {
		Connection_close(con);
	}
}

//NTY_DB_WATCH_INSERT_FORMAT

static int ntyExecuteWatchInsert(void *self, U8 *imei) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	
	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_WATCH_INSERT_FORMAT, imei);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_APPIDLIST_SELECT_FORMAT
static int ntyQueryAppIDListSelect(void *self, C_DEVID did, void *container) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_APPIDLIST_SELECT_FORMAT, did);
			while (ResultSet_next(r)) {
				C_DEVID id = ResultSet_getLLong(r, 1);
				ntylog("app: %lld\n", id);
#if 1
				ntyVectorAdd(container, &id, sizeof(C_DEVID));
#else
				ntyFriendsTreeInsert(tree, id);
#endif
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_WATCHIDLIST_SELECT_FORMAT
static int ntyQueryWatchIDListSelect(void *self, C_DEVID aid, void *container) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_WATCHIDLIST_SELECT_FORMAT, aid);
			while (ResultSet_next(r)) {
				C_DEVID id = ResultSet_getLLong(r, 1);
#if 1
				ntylog(" ntyQueryWatchIDListSelect : %lld\n", id);
				ntyVectorAdd(container, &id, sizeof(C_DEVID));
#else
				ntyFriendsTreeInsert(tree, id);
#endif
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
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
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_DEV_APP_INSERT_FORMAT, aid, imei);
			while (ResultSet_next(r)) {
				ret = ResultSet_getInt(r, 1);
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
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
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_DEV_APP_DELETE_FORMAT, aid, did);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_INSERT_GROUP
static int ntyQueryDevAppGroupInsert(void *self, C_DEVID aid, C_DEVID imei, U8 *name) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {

			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_GROUP, imei, aid, name);
			while (ResultSet_next(r)) {
				ret = ResultSet_getInt(r, 1);
			}

		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_DEV_APP_DELETE_FORMAT
static int ntyExecuteDevAppGroupDelete(void *self, C_DEVID aid, C_DEVID did) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_DELETE_GROUP, did, aid);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}



//NTY_DB_LOCATION_INSERT_FORMAT
static int ntyExecuteLocationInsert(void *self, C_DEVID did, U8 *lng, U8 *lat, U8 type, U8 *info) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			U8 sql[256];
			Connection_execute(con, NTY_DB_NAMES_UTF8_SET_FORMAT);
			sprintf(sql, NTY_DB_LOCATION_INSERT_FORMAT, did, lng, lat, type, info);
			ntylog("%s", sql);
			Connection_execute(con, NTY_DB_LOCATION_INSERT_FORMAT, did, lng, lat, type, info);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
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
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_STEP_INSERT_FORMAT, did, value);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
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
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_HEARTRATE_INSERT_FORMAT, did, value);
		}	
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_DEVICELOGIN_UPDATE_FORMAT
int ntyExecuteDeviceLoginUpdate(void *self, C_DEVID did) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_DEVICELOGIN_UPDATE_FORMAT, did);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_DEVICELOGOUT_UPDATE_FORMAT
int ntyExecuteDeviceLogoutUpdate(void *self, C_DEVID did) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_DEVICELOGOUT_UPDATE_FORMAT, did);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_PHNUM_VALUE_SELECT_FORMAT
int ntyQueryPhNumSelect(void *self, C_DEVID did, U8 *iccid, U8 *phnum) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_PHNUM_VALUE_SELECT_FORMAT, did, iccid);
			while (ResultSet_next(r)) {
				const char *u8pNum = ResultSet_getString(r, 1);
				int len = strlen(u8pNum);
				ntylog(" ntyQueryPhNumSelect --> len:%d\n", len);
				if (len < 20)
					memcpy(u8PhNum, u8pNum, len);
			}
			strcpy(phnum, u8PhNum);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_DEVICE_STATUS_RESET_FORMAT
int ntyExecuteDeviceStatusReset(void *self) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_DEVICE_STATUS_RESET_FORMAT);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_APPLOGIN_UPDATE_FORMAT
int ntyExecuteAppLoginUpdate(void *self, C_DEVID aid) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_APPLOGIN_UPDATE_FORMAT, aid);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_APPLOGOUT_UPDATE_FORMAT
int ntyExecuteAppLogoutUpdate(void *self, C_DEVID aid) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_APPLOGOUT_UPDATE_FORMAT, aid);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}



//NTY_DB_INSERT_EFENCE
int ntyExecuteEfenceInsert(void *self, C_DEVID aid, C_DEVID did, int index, int num, U8 *points, U8 *runtime, int *id) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			U8 buffer[512];

			sprintf(buffer, NTY_DB_INSERT_EFENCE, did, index, num, points, runtime);
			ntylog(" sql : %s\n", buffer);
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_EFENCE, did, index, num, points, runtime);
			if (ResultSet_next(r)) {
				*id = ResultSet_getInt(r, 1);
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_DELETE_EFENCE
int ntyExecuteEfenceDelete(void *self, C_DEVID aid, C_DEVID did, int index) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			U8 buffer[512];

			sprintf(buffer, NTY_DB_DELETE_EFENCE, did, index);
			ntylog(" sql : %s\n", buffer);
			Connection_execute(con, NTY_DB_DELETE_EFENCE, did, index);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_SELECT_ICCID
int ntyQueryICCIDSelect(void *self, C_DEVID did, const char *iccid, char *phonenum) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			U8 buffer[512];
			sprintf(buffer, NTY_DB_SELECT_ICCID, iccid);
			ntylog(" sql : %s\n", buffer);
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_SELECT_ICCID, iccid);
			if (ResultSet_next(r)) {
				const char *temp = ResultSet_getString(r, 1);
				memcpy(phonenum, temp, strlen(temp));
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_UPDATE_RUNTIME
int ntyExecuteRuntimeUpdate(void *self, C_DEVID aid, C_DEVID did, int auto_conn, U8 loss_report, U8 light_panel, const char *bell, int target_step) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			U8 buffer[512];
			ntylog(" sql : %s\n", buffer);
			Connection_execute(con, NTY_DB_UPDATE_RUNTIME, did, auto_conn, loss_report, light_panel, bell, target_step);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_UPDATE_RUNTIME_AUTOCONN
int ntyExecuteRuntimeAutoConnUpdate(void *self, C_DEVID aid, C_DEVID did, int runtime_param) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			U8 buffer[512];
			ntylog(" sql : %s\n", buffer);
			Connection_execute(con, NTY_DB_UPDATE_RUNTIME_AUTOCONN, did, runtime_param);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_UPDATE_RUNTIME_LOSSREPORT
int ntyExecuteRuntimeLossReportUpdate(void *self, C_DEVID aid, C_DEVID did, U8 runtime_param) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			U8 buffer[512];
			ntylog(" sql : %s\n", buffer);
			Connection_execute(con, NTY_DB_UPDATE_RUNTIME_LOSSREPORT, did, runtime_param);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_UPDATE_RUNTIME_LIGHTPANEL
int ntyExecuteRuntimeLightPanelUpdate(void *self, C_DEVID aid, C_DEVID did, U8 runtime_param) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			U8 buffer[512];
			ntylog(" sql : %s\n", buffer);
			Connection_execute(con, NTY_DB_UPDATE_RUNTIME_LIGHTPANEL, did, runtime_param);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_UPDATE_RUNTIME_BELL
int ntyExecuteRuntimeBellUpdate(void *self, C_DEVID aid, C_DEVID did, const char *runtime_param) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			U8 buffer[512];
			ntylog(" sql : %s\n", buffer);
			Connection_execute(con, NTY_DB_UPDATE_RUNTIME_BELL, did, runtime_param);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_UPDATE_RUNTIME_TARGETSTEP
int ntyExecuteRuntimeTargetStepUpdate(void *self, C_DEVID aid, C_DEVID did, int runtime_param) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			U8 buffer[512];
			ntylog(" sql : %s\n", buffer);
			Connection_execute(con, NTY_DB_UPDATE_RUNTIME_TARGETSTEP, did, runtime_param);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_UPDATE_TURN
int ntyExecuteTurnUpdate(void *self, C_DEVID aid, C_DEVID did, U8 status, const char *ontime, const char *offtime) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			U8 buffer[512];
			ntylog(" sql : %s\n", buffer);
			Connection_execute(con, NTY_DB_UPDATE_TURN, did, status, ontime, offtime);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_INSERT_PHONEBOOK
int ntyExecuteContactsInsert(void *self, C_DEVID aid, C_DEVID did, Contacts *contacts, int *contactsId) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			//U8 admin = atoi(contacts->admin);
			//U8 app = atoi(contacts->app);
			U8 buffer[512] = {0};
			sprintf(buffer, NTY_DB_INSERT_PHONEBOOK, aid,did,contacts->image,contacts->name,contacts->telphone);
			ntylog(" sql : %s\n", buffer);
			ResultSet_T r = Connection_executeQuery(con, 
				NTY_DB_INSERT_PHONEBOOK, aid,did,contacts->image,contacts->name,contacts->telphone);
			if (ResultSet_next(r)) {
				int tempId = ResultSet_getInt(r, 1);
				*contactsId = tempId;
			}
		}
	}
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_UPDATE_PHONEBOOK
int ntyExecuteContactsUpdate(void *self, C_DEVID aid, C_DEVID did, Contacts *contacts, int contactsId) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			//U8 admin = atoi(contacts->admin);
			//U8 app = atoi(contacts->app);
			U8 buffer[512] = {0};
			sprintf(buffer, NTY_DB_UPDATE_PHONEBOOK, aid,did,contacts->image,contacts->name,contacts->telphone, contactsId);
			ntylog(" sql : %s\n", buffer);
			Connection_execute(con, NTY_DB_UPDATE_PHONEBOOK, aid,did,contacts->image,contacts->name,contacts->telphone, contactsId);
		}
	}
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_DELETE_PHONEBOOK
int ntyExecuteContactsDelete(void *self, C_DEVID aid, C_DEVID did, int contactsId) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			U8 buffer[512] = {0};
			sprintf(buffer, NTY_DB_DELETE_PHONEBOOK, did,contactsId);
			ntylog(" sql : %s\n", buffer);
			Connection_execute(con, NTY_DB_DELETE_PHONEBOOK, did, contactsId);
		}
	}
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_INSERT_SCHEDULE
int ntyExecuteScheduleInsert(void *self, C_DEVID aid, C_DEVID did, const char *daily, const char *time, const char *details, int *scheduleId) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			U8 buffer[512] = {0};
			sprintf(buffer, NTY_DB_INSERT_SCHEDULE, did, daily, time, details);
			ntylog(" sql : %s\n", buffer);
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_SCHEDULE, did, daily, time, details);
			if (ResultSet_next(r)) {
				int tempId = ResultSet_getInt(r, 1);
				*scheduleId = tempId;
			}
		}
	}
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_DELETE_SCHEDULE
int ntyExecuteScheduleDelete(void *self, C_DEVID aid, C_DEVID did, int scheduleId) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			U8 buffer[512];
			ntylog(" sql : %s\n", buffer);
			Connection_execute(con, NTY_DB_DELETE_SCHEDULE, did, scheduleId);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_UPDATE_SCHEDULE
int ntyExecuteScheduleUpdate(void *self, C_DEVID aid, C_DEVID did, int scheduleId, const char *daily, const char *time, const char *details) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			U8 buffer[512];
			ntylog(" sql : %s\n", buffer);
			Connection_execute(con, NTY_DB_UPDATE_SCHEDULE, did, scheduleId, daily, time, details);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_SELECT_SCHEDULE
int ntyExecuteScheduleSelect(void *self, C_DEVID aid, C_DEVID did, ScheduleAck *pScheduleAck, size_t size) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			U8 buffer[512];
			ntylog(" sql : %s\n", buffer);
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_SELECT_SCHEDULE, did);
			if (pScheduleAck != NULL && pScheduleAck->results.pSchedule != NULL) {
				size_t i = 0;
				while (ResultSet_next(r)) {
					pScheduleAck->results.size = i;
					if (i>=size) {
						break;
					}
					pScheduleAck->results.pSchedule[i].id = ResultSet_getString(r, 1);
					pScheduleAck->results.pSchedule[i].daily  = ResultSet_getString(r, 3);
					pScheduleAck->results.pSchedule[i].time = ResultSet_getString(r, 4);
					pScheduleAck->results.pSchedule[i].details = ResultSet_getString(r, 5);
					i++;
				}
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_UPDATE_TIMETABLE
int ntyExecuteTimeTablesUpdate(void *self, C_DEVID aid, C_DEVID did, const char *morning, U8 morning_turn, const char *afternoon,  U8 afternoon_turn, const char *daily, int *id) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			U8 buffer[512];
			ntylog(" sql : %s\n", buffer);
			ResultSet_T r =Connection_executeQuery(con, NTY_DB_UPDATE_TIMETABLE, did, morning, morning_turn, afternoon, afternoon_turn, daily, *id);
			while (ResultSet_next(r)) {
				*id = ResultSet_getInt(r, 1);				
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_DEVICE_STATUS_RESET_FORMAT
int ntyQueryDeviceOnlineStatus(void *self, C_DEVID did, int *online) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_DEVICE_STATUS_SELECT_FORMAT, did);
			while (ResultSet_next(r)) {
				*online = ResultSet_getInt(r, 1);				
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_DEVICE_STATUS_RESET_FORMAT
int ntyQueryAppOnlineStatus(void *self, C_DEVID aid, int *online) {
	ConnectionPool *pool = self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_APP_STATUS_SELECT_FORMAT, aid);
			while (ResultSet_next(r)) {
				*online = ResultSet_getInt(r, 1);				
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

int ntyExecuteWatchInsertHandle(U8 *imei) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteWatchInsert(pool, imei);
}

int ntyQueryAppIDListSelectHandle(C_DEVID did, void *container) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryAppIDListSelect(pool, did, container);
}


int ntyQueryWatchIDListSelectHandle(C_DEVID aid, void *container) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryWatchIDListSelect(pool, aid, container);
}

int ntyQueryDevAppRelationInsertHandle(C_DEVID aid, U8 *imei) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryDevAppRelationInsert(pool, aid, imei);
}


int ntyQueryDevAppGroupInsertHandle(C_DEVID aid, C_DEVID did) {
	void *pool = ntyConnectionPoolInstance();

	U8 *Name = "Father";
	return ntyQueryDevAppGroupInsert(pool, aid, did, Name);
}


int ntyExecuteDevAppGroupDeleteHandle(C_DEVID aid, C_DEVID did) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteDevAppGroupDelete(pool, aid, did);
}

int ntyExecuteLocationInsertHandle( C_DEVID did, U8 *lng, U8 *lat, U8 type, U8 *info) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteLocationInsert(pool, did, lng, lat, type, info);
}

int ntyExecuteStepInsertHandle(C_DEVID did, int value) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteStepInsert(pool, did, value);
}

int ntyExecuteHeartRateInsertHandle(C_DEVID did, int value) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteHeartRateInsert(pool, did, value);
}

int ntyExecuteDeviceLoginUpdateHandle(C_DEVID did) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteDeviceLoginUpdate(pool, did);
}

int ntyExecuteDeviceLogoutUpdateHandle(C_DEVID did) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteDeviceLogoutUpdate(pool, did);
}

int ntyExecuteAppLoginUpdateHandle(C_DEVID aid) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteAppLoginUpdate(pool, aid);
}

int ntyExecuteAppLogoutUpdateHandle(C_DEVID aid) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteAppLogoutUpdate(pool, aid);
}
int ntyExecuteEfenceInsertHandle(C_DEVID aid, C_DEVID did, int index, int num, U8 *points, U8 *runtime, int *id) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteEfenceInsert(pool, aid, did, index, num, points, runtime, id);
}

int ntyExecuteEfenceDeleteHandle(C_DEVID aid, C_DEVID did, int index) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteEfenceDelete(pool, aid, did, index);
}

int ntyExecuteICCIDSelectHandle(C_DEVID did, const char *iccid, char *phonenum) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryICCIDSelect(pool, did, iccid, phonenum);
}

int ntyExecuteRuntimeUpdateHandle(C_DEVID aid, C_DEVID did, int auto_conn, U8 loss_report, U8 light_panel, const char *bell, int target_step) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteRuntimeUpdate(pool, aid, did, auto_conn, loss_report, light_panel, bell, target_step);
}

int ntyExecuteRuntimeAutoConnUpdateHandle(C_DEVID aid, C_DEVID did, int runtime_param) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteRuntimeAutoConnUpdate(pool, aid, did, runtime_param);
}

int ntyExecuteRuntimeLossReportUpdateHandle(C_DEVID aid, C_DEVID did, U8 runtime_param) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteRuntimeLossReportUpdate(pool, aid, did, runtime_param);
}

int ntyExecuteRuntimeLightPanelUpdateHandle(C_DEVID aid, C_DEVID did, U8 runtime_param) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteRuntimeLightPanelUpdate(pool, aid, did, runtime_param);
}

int ntyExecuteRuntimeBellUpdateHandle(C_DEVID aid, C_DEVID did, const char *runtime_param) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteRuntimeBellUpdate(pool, aid, did, runtime_param);
}

int ntyExecuteRuntimeTargetStepUpdateHandle(C_DEVID aid, C_DEVID did, int runtime_param) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteRuntimeTargetStepUpdate(pool, aid, did, runtime_param);
}

int ntyExecuteTurnUpdateHandle(C_DEVID aid, C_DEVID did, U8 status, const char *ontime, const char *offtime) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteTurnUpdate(pool, aid, did, status, ontime, offtime);
}

int ntyExecuteContactsInsertHandle(C_DEVID aid, C_DEVID did, Contacts *contacts, int *contactsId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteContactsInsert(pool, aid, did, contacts, contactsId);
}

int ntyExecuteContactsUpdateHandle(C_DEVID aid, C_DEVID did, Contacts *contacts, int contactsId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteContactsUpdate(pool, aid, did, contacts, contactsId);
}

int ntyExecuteContactsDeleteHandle(C_DEVID aid, C_DEVID did, int contactsId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteContactsDelete(pool, aid, did, contactsId);
}

int ntyExecuteScheduleInsertHandle(C_DEVID aid, C_DEVID did, const char *daily, const char *time, const char *details, int *scheduleId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteScheduleInsert(pool, aid, did, daily, time, details, scheduleId);
}

int ntyExecuteScheduleDeleteHandle(C_DEVID aid, C_DEVID did, int scheduleId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteScheduleDelete(pool, aid, did, scheduleId);
}

int ntyExecuteScheduleUpdateHandle(C_DEVID aid, C_DEVID did, int scheduleId, const char *daily, const char *time, const char *details) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteScheduleUpdate(pool, aid, did, scheduleId, daily, time, details);
}

int ntyExecuteScheduleSelectHandle(C_DEVID aid, C_DEVID did, ScheduleAck *pScheduleAck, size_t size) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteScheduleSelect(pool, aid, did, pScheduleAck, size);
}

int ntyExecuteTimeTablesUpdateHandle(C_DEVID aid, C_DEVID did, const char *morning, U8 morning_turn, const char *afternoon,  U8 afternoon_turn, const char *daily, int *id) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteTimeTablesUpdate(pool, aid, did, morning, morning_turn, afternoon, afternoon_turn, daily, id);
}

int ntyQueryPhNumSelectHandle(C_DEVID did, U8 *iccid, U8 *phnum) {
	void *pool = ntyConnectionPoolInstance();
	ntylog(" %s:%lld\n", iccid, did);
	return ntyQueryPhNumSelect(pool, did, iccid, phnum);
}

int ntyExecuteDeviceStatusResetHandle(void) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteDeviceStatusReset(pool);
}

int ntyQueryDeviceOnlineStatusHandle(C_DEVID did, int *online) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryDeviceOnlineStatus(pool, did, online);
}

int ntyQueryAppOnlineStatusHandle(C_DEVID aid, int *online) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryAppOnlineStatus(pool, aid, online);
}


int ntyConnectionPoolInit(void) {
	//void *pool = ntyConnectionPoolInstance();
	ntyExecuteDeviceStatusResetHandle();
}

void ntyConnectionPoolDeInit(void) {
	void *pool = ntyGetConnectionPoolInstance();
	if (pool != NULL) {
		ntyConnectionPoolRelease(pool);
	}
}


