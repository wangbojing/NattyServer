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
#include "NattyResult.h"

#include <string.h>
#include <wchar.h>

void* ntyConnectionPoolInitialize(ConnectionPool *pool) {
	pool->url = URL_new(MYSQL_DB_CONN_STRING);
	pool->nPool = ConnectionPool_new(pool->url);

	ConnectionPool_setInitialConnections(pool->nPool, 4);
    ConnectionPool_setMaxConnections(pool->nPool, 200);
    ConnectionPool_setConnectionTimeout(pool->nPool, 2);
    ConnectionPool_setReaper(pool->nPool, 1);
	
	ConnectionPool_start(pool->nPool);

	return pool;
}

void* ntyConnectionPoolDestory(ConnectionPool *pool) {
	ConnectionPool_free(&pool->nPool);
	ConnectionPool_stop(pool->nPool);
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
#if 1 //ConnectionPool is Full
	if(ntyConnectionPoolDynamicsSize(pConnectionPool)) {
		return NULL;
	}
#endif
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
#if 0
	if (size >= max*CONNECTION_SIZE_THRESHOLD_RATIO || active >= max*CONNECTION_SIZE_REAP_RATIO) {
		return 1;
	}
#endif	
	ntylog(" size:%d, active:%d, max:%d\n", size, active, max);
	return 0;
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
	if (pool == NULL) return con;
	if (con == NULL) {
#if 0
		int n = ConnectionPool_reapConnections(pool->nPool);
		Connection_T con = ConnectionPool_getConnection(pool->nPool);
#else
		return NULL;
#endif
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
	if (pool == NULL) return NTY_RESULT_BUSY;
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
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_APPIDLIST_SELECT_FORMAT, did);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					C_DEVID id = ResultSet_getLLong(r, 1);
					ntylog("app: %lld\n", id);
#if 1
					ntyVectorInsert(container, &id, sizeof(C_DEVID));
#else
					ntyFriendsTreeInsert(tree, id);
#endif
					ret = 0;
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


//NTY_DB_WATCHIDLIST_SELECT_FORMAT
static int ntyQueryWatchIDListSelect(void *self, C_DEVID aid, void *container) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_WATCHIDLIST_SELECT_FORMAT, aid);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					C_DEVID id = ResultSet_getLLong(r, 1);
#if 1
					ntylog(" ntyQueryWatchIDListSelect : %lld\n", id);
					ntyVectorInsert(container, &id, sizeof(C_DEVID));
#else
					ntyFriendsTreeInsert(tree, id);
#endif
					ret = 0;
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

//NTY_DB_DEV_APP_INSERT_FORMAT
static int ntyQueryDevAppRelationInsert(void *self, C_DEVID aid, U8 *imei) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_DEV_APP_INSERT_FORMAT, aid, imei);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					ret = ResultSet_getInt(r, 1);
				}
				ret = 0;
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
	if (pool == NULL) return NTY_RESULT_BUSY;
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
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {

			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_GROUP, imei, aid, name);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					ret = ResultSet_getInt(r, 1);
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

//NTY_DB_CHECK_GROUP
static int ntyQueryDevAppGroupCheckSelect(void *self, C_DEVID aid, C_DEVID imei) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {

			ntylog(" ntyQueryDevAppGroupCheckSelect ...\n");
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_CHECK_GROUP, imei, aid);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					ret = ResultSet_getInt(r, 1);
					ntylog(" ntyQueryDevAppGroupCheckSelect ...\n");
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


//NTY_DB_INSERT_BIND_GROUP
static int ntyExecuteDevAppGroupBindInsert(void *self, int msgId, C_DEVID *proposerId, U8 *phonenum) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
		
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_BIND_GROUP, msgId);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					ret = ResultSet_getInt(r, 1);
					
					*proposerId = ResultSet_getLLong(r, 2);
					const char *num = ResultSet_getString(r, 3);
					ntylog("ntyExecuteDevAppGroupBindInsert --> num:%s, proposerId:%lld\n", num, *proposerId);
					if (num != NULL) {
						memcpy(phonenum, num, strlen(num));
					}
					
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

//NTY_DB_DELETE_BIND_GROUP
static int ntyExecuteBindConfirmDelete(void *self, int msgId, C_DEVID *id) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_DELETE_BIND_GROUP, msgId);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					*id = ResultSet_getLLong(r, 1);
					ntylog("ntyExecuteBindConfirmDelete -->  id:%lld\n", *id);
					ret = 0;
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


//NTY_DB_DELETE_COMMON_OFFLINE_MSG
static int ntyExecuteCommonOfflineMsgDelete(void *self, int msgId, C_DEVID clientId) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_DELETE_COMMON_OFFLINE_MSG, msgId, clientId);
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


//NTY_DB_SELECT_COMMON_OFFLINE_MSG
static int ntyQueryCommonOfflineMsgSelect(void *self, C_DEVID deviceId, void *container) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			

			/*
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_SELECT_COMMON_OFFLINE_MSG, deviceId);
			while (ResultSet_next(r)) {
				int id = ResultSet_getInt(r, 1);
				//ntylog(" ntyQueryCommonOfflineMsgSelect : %lld\n", id);
				ntyVectorInsert(container, &id, sizeof(int));
			}
			*/
			ret = -1;
			
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_SELECT_COMMON_OFFLINE_MSG, deviceId);

			if (r != NULL) {
				while (ResultSet_next(r)) {
					int msgId = ResultSet_getInt(r, 1);
					C_DEVID r_senderId = ResultSet_getLLong(r, 2);
					C_DEVID r_groupId = ResultSet_getLLong(r, 3);
					const char *r_details = ResultSet_getString(r, 4);
					size_t details_len = strlen(r_details);
					
					CommonOfflineMsg *pCommonOfflineMsg = malloc(sizeof(CommonOfflineMsg));
					if (pCommonOfflineMsg == NULL) {
						ntylog(" %s --> malloc failed CommonOfflineMsg. \n", __func__);
						break;
					}
					
					memset(pCommonOfflineMsg, 0, sizeof(CommonOfflineMsg));
					
					pCommonOfflineMsg->msgId = msgId;
					pCommonOfflineMsg->senderId = r_senderId;
					pCommonOfflineMsg->groupId = r_groupId;

					pCommonOfflineMsg->details = malloc(details_len+1);
					if (pCommonOfflineMsg->details == NULL) {
						ntylog(" %s --> malloc failed CommonOfflineMsg->details\n", __func__);
						free(pCommonOfflineMsg);
						break;
					}
					
					memset(pCommonOfflineMsg->details, 0, details_len+1);
					memcpy(pCommonOfflineMsg->details, r_details, details_len);
					
					ntyVectorInsert(container, pCommonOfflineMsg, sizeof(CommonOfflineMsg));
					ret = 0;
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

//PROC_SELECT_VOICE_OFFLINE_MSG
static int ntyQueryVoiceOfflineMsgSelect(void *self, C_DEVID fromId, void *container) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			ret = -1;
			
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_SELECT_VOICE_OFFLINE_MSG, fromId);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					int msgId = ResultSet_getInt(r, 1);
					C_DEVID r_senderId = ResultSet_getLLong(r, 2);
					C_DEVID r_groupId = ResultSet_getLLong(r, 3);
					const char *r_details = ResultSet_getString(r, 4);
					long timeStamp = ResultSet_getTimestamp(r, 5);
					size_t details_len = strlen(r_details);
					
					nOfflineMsg *pOfflineMsg = malloc(sizeof(nOfflineMsg));
					if (pOfflineMsg == NULL) {
						ntylog(" %s --> malloc nOfflineMsg.details error. \n", __func__);
						break;
					}
					memset(pOfflineMsg, 0, sizeof(nOfflineMsg));
					
					pOfflineMsg->msgId = msgId;
					pOfflineMsg->senderId = r_senderId;
					pOfflineMsg->groupId = r_groupId;
					pOfflineMsg->timeStamp = timeStamp;
					pOfflineMsg->details = malloc(details_len+1);
					if (pOfflineMsg->details != NULL) {
						memset(pOfflineMsg->details, 0, details_len+1);
						memcpy(pOfflineMsg->details, r_details, details_len);
					}
					
					ntyVectorInsert(container, pOfflineMsg, sizeof(CommonOfflineMsg));
					ret = 0;
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



//NTY_DB_SELECT_COMMON_MSG
static int ntyQueryCommonMsgSelect(void *self, C_DEVID msgId, C_DEVID *senderId, C_DEVID *groupId, char *json) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
		/*
			U8 buffer[512];
			sprintf(buffer, NTY_DB_SELECT_COMMON_MSG, msgId);
			//ntylog(buffer);
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_SELECT_COMMON_MSG, msgId);
			while (ResultSet_next(r)) {
				C_DEVID r_senderId = ResultSet_getLLong(r, 1);
				C_DEVID r_groupId = ResultSet_getLLong(r, 2);
				const char *r_details = ResultSet_getString(r, 3);
				memcpy(json, r_details, strlen(r_details));
				*senderId = r_senderId;
				*groupId = r_groupId;
			}
			*/
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


//NTY_DB_INSERT_BIND_CONFIRM
static int ntyQueryBindConfirmInsert(void *self, C_DEVID admin, C_DEVID imei, U8 *name, U8 *wimage, C_DEVID proposer, U8 *call, U8 *uimage, int *msgId) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			U8 sql[512] = {0};
			sprintf(sql, NTY_DB_INSERT_BIND_CONFIRM, admin, imei, name, wimage, proposer, call, uimage);			
			ntylog("%s\n", sql);	
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_BIND_CONFIRM, admin, imei, name, wimage, proposer, call, uimage);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					*msgId = ResultSet_getInt(r, 1);
					ret = 0;
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

//NTY_DB_SELECT_PHONE_NUMBER
static int ntyQueryPhoneBookSelect(void *self, C_DEVID imei, C_DEVID userId, char *phonenum) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_SELECT_PHONE_NUMBER, imei, userId);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					const char *pnum = ResultSet_getString(r, 1);
					ntylog(" ntyQueryPhoneBookSelect --> PhoneNum:%s\n", pnum);
					memcpy(phonenum, pnum, strlen(pnum));

					ret = 0;
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

//NTY_DB_SELECT_ADMIN
static int ntyQueryAdminSelect(void *self, C_DEVID did, C_DEVID *appid) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {

			ResultSet_T r = Connection_executeQuery(con, NTY_DB_SELECT_ADMIN, did);
			if (r != NULL) {
				while (ResultSet_next(r)) {
#if 0
					ret = ResultSet_getInt(r, 1);
#else
					*appid = ResultSet_getLLong(r, 1);
#endif
					ret = 0;
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



//NTY_DB_SELECT_BIND_OFFLINE_MSG_TO_ADMIN
static int ntyQueryBindOfflineMsgToAdminSelect(void *self, C_DEVID fromId, void *container) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ret = -1;

			U8 sql[256] = {0};		
			sprintf(sql, NTY_DB_SELECT_BIND_OFFLINE_MSG_TO_ADMIN, fromId);			
			ntylog("%s\n", sql);
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_SELECT_BIND_OFFLINE_MSG_TO_ADMIN, fromId);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					int bId = ResultSet_getInt(r, 1);
					
					C_DEVID r_admin = ResultSet_getLLong(r, 2);
					C_DEVID r_imei = ResultSet_getLLong(r, 3);
					const char *r_watchname = ResultSet_getString(r, 4);
					const char *r_watchimage = ResultSet_getString(r, 5);
					C_DEVID r_proposer = ResultSet_getLLong(r, 6);
					const char *r_usercall = ResultSet_getString(r, 7);
					const char *r_userimage = ResultSet_getString(r, 8);
					BindOfflineMsgToAdmin *pMsgToAdmin = malloc(sizeof(BindOfflineMsgToAdmin));
					if (pMsgToAdmin == NULL) {
						ntylog(" %s --> malloc BindOfflineMsgToAdmin error. \n", __func__);
						break;
					}
					memset(pMsgToAdmin, 0, sizeof(BindOfflineMsgToAdmin));
					
					pMsgToAdmin->msgId = bId;
					pMsgToAdmin->IMEI = r_imei;
					pMsgToAdmin->admin = r_admin;
					pMsgToAdmin->proposer = r_proposer;

					size_t watchname_len = strlen(r_watchname);
					size_t watchimage_len = strlen(r_watchimage);
					size_t usercall_len = strlen(r_usercall);
					size_t userimage_len = strlen(r_userimage);

					pMsgToAdmin->watchName = malloc(watchname_len+1);
					if (pMsgToAdmin->watchName != NULL) {
						memset(pMsgToAdmin->watchName, 0, watchname_len+1);
						memcpy(pMsgToAdmin->watchName, r_watchname, watchname_len);
					}
						
					pMsgToAdmin->watchImage = malloc(watchimage_len+1);
					if (pMsgToAdmin->watchImage != NULL) {
						memset(pMsgToAdmin->watchImage, 0, watchimage_len+1);
						memcpy(pMsgToAdmin->watchImage, r_watchimage, watchimage_len);
					}
					pMsgToAdmin->userName = malloc(usercall_len+1);
					if (pMsgToAdmin->userName != NULL) {
						memset(pMsgToAdmin->userName, 0, usercall_len+1);
						memcpy(pMsgToAdmin->userName, r_usercall, usercall_len);
					}
					pMsgToAdmin->userImage = malloc(userimage_len+1);
					if (pMsgToAdmin->userImage != NULL) {
						memset(pMsgToAdmin->userImage, 0, userimage_len+1);
						memcpy(pMsgToAdmin->userImage, r_userimage, userimage_len);
					}
						
					ntyVectorAdd(container, pMsgToAdmin, sizeof(BindOfflineMsgToAdmin));	
					ret = 0;
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


//NTY_DB_SELECT_BIND_OFFLINE_MSG_TO_PROPOSER
static int ntyQueryBindOfflineMsgToProposerSelect(void *self, C_DEVID fromId, void *container) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ret = -1;
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_SELECT_BIND_OFFLINE_MSG_TO_PROPOSER, fromId);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					int bId = ResultSet_getInt(r, 1);

					C_DEVID r_admin = ResultSet_getLLong(r, 2);
					C_DEVID r_imei = ResultSet_getLLong(r, 3);
					const char *r_watchname = ResultSet_getString(r, 4);
					const char *r_watchimage = ResultSet_getString(r, 5);

					BindOfflineMsgToProposer *pMsgToProposer = malloc(sizeof(BindOfflineMsgToProposer));
					if (pMsgToProposer == NULL) {
						ntylog(" %s --> malloc BindOfflineMsgToProposer.details error. \n", __func__);
						break;
					}
					memset(pMsgToProposer, 0, sizeof(BindOfflineMsgToProposer));
										
					//pMsgToProposer->msgId = bId;
					//pMsgToProposer->IMEI = r_imei;
					//pMsgToProposer->admin = r_admin;
					//pMsgToProposer->proposer = r_proposer;
					ntyVectorAdd(container, pMsgToProposer, sizeof(BindOfflineMsgToProposer));
					ret = 0;
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



//NTY_DB_DEV_APP_DELETE_FORMAT
static int ntyExecuteDevAppGroupDelete(void *self, C_DEVID aid, C_DEVID did) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
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
	if (pool == NULL) return NTY_RESULT_BUSY;
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
		ntylog(" %s --> Connection_close\n", __func__);		ntyConnectionClose(con);	
	}	
	END_TRY;	
	return ret;
}



//NTY_DB_INSERT_LOCATION
static int ntyExecuteLocationNewInsert(void *self, C_DEVID did, U8 type, const char *lnglat, const char *info, const char *desc) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_INSERT_LOCATION, did, type, lnglat, info, desc);
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
	if (pool == NULL) return NTY_RESULT_BUSY;
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
	if (pool == NULL) return NTY_RESULT_BUSY;
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
	if (pool == NULL) return NTY_RESULT_BUSY;
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
	if (pool == NULL) return NTY_RESULT_BUSY;
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
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	//U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_PHNUM_VALUE_SELECT_FORMAT, did, iccid);

			if (r != NULL) {
				while (ResultSet_next(r)) {
					const char *u8pNum = ResultSet_getString(r, 1);
					int len = strlen(u8pNum);
					ntylog(" ntyQueryPhNumSelect --> len:%d\n", len);
					if (len < 20)
						memcpy(phnum, u8pNum, len);
				}
			}
			//strcpy(phnum, u8PhNum);
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
	if (pool == NULL) return NTY_RESULT_BUSY;
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
	if (pool == NULL) return NTY_RESULT_BUSY;
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
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog("ntyExecuteAppLogoutUpdate --> execute\n");
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
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog("ntyExecuteEfenceInsert --> executeQuery\n");
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_EFENCE, did, index, num, points, runtime);

			if (r != NULL) {
				if (ResultSet_next(r)) {
					*id = ResultSet_getInt(r, 1);
					ret = 0;
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


//NTY_DB_DELETE_EFENCE
int ntyExecuteEfenceDelete(void *self, C_DEVID aid, C_DEVID did, int index) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog("ntyExecuteEfenceDelete --> execute\n");
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
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog("ntyQueryICCIDSelect --> executeQuery\n");
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_PHNUM_VALUE_SELECT, did, iccid);

			if (r != NULL) {
				if (ResultSet_next(r)) {
					ntylog("ntyQueryICCIDSelect --> ResultSet_getString\n");
					const char *temp = ResultSet_getString(r, 1);
					ntylog("ntyQueryICCIDSelect --> ResultSet_getString : %s\n", temp);
					if (temp != NULL) {
						memcpy(phonenum, temp, strlen(temp));
					}

					ret = 0;
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


//NTY_DB_UPDATE_RUNTIME
int ntyExecuteRuntimeUpdate(void *self, C_DEVID aid, C_DEVID did, int auto_conn, U8 loss_report, U8 light_panel, const char *bell, int target_step) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
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
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
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
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			ntylog(" Connection_execute before ntyExecuteRuntimeLossReportUpdate \n");
			Connection_execute(con, NTY_DB_UPDATE_RUNTIME_LOSSREPORT, did, runtime_param);
			ntylog(" Connection_execute after ntyExecuteRuntimeLossReportUpdate \n");
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
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
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
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
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
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			ntylog(" ntyExecuteRuntimeTargetStepUpdate -->  before Connection_execute\n");
			Connection_execute(con, NTY_DB_UPDATE_RUNTIME_TARGETSTEP, did, runtime_param);
			ntylog(" ntyExecuteRuntimeTargetStepUpdate -->  after Connection_execute\n");
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
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			ntylog(" ntyExecuteTurnUpdate --> before Connection_execute\n ");
			Connection_execute(con, NTY_DB_UPDATE_TURN, did, status, ontime, offtime);
			ntylog(" ntyExecuteTurnUpdate --> after Connection_execute\n ");
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
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			ResultSet_T r = Connection_executeQuery(con, 
				NTY_DB_INSERT_PHONEBOOK, aid,did,contacts->image,contacts->name,contacts->telphone);
			if (r != NULL) {
				if (ResultSet_next(r)) {
					int tempId = ResultSet_getInt(r, 1);
					*contactsId = tempId;
					ret = 0;
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


//NTY_DB_UPDATE_PHONEBOOK
int ntyExecuteContactsUpdate(void *self, C_DEVID aid, C_DEVID did, Contacts *contacts, int contactsId) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;

	if (contacts == NULL) return NTY_RESULT_FAILED;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
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
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
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
int ntyExecuteScheduleInsert(void *self, C_DEVID aid, C_DEVID did, const char *daily, const char *time, int status, const char *details, int *scheduleId) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_SCHEDULE, did, daily, time, status, details);
			if (r != NULL) {
				if (ResultSet_next(r)) {
					int tempId = ResultSet_getInt(r, 1);
					*scheduleId = tempId;
					ret = 0;
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

//NTY_DB_DELETE_SCHEDULE
int ntyExecuteScheduleDelete(void *self, C_DEVID aid, C_DEVID did, int scheduleId) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
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
int ntyExecuteScheduleUpdate(void *self, C_DEVID aid, C_DEVID did, int scheduleId, const char *daily, const char *time, int status, const char *details) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog(" ntyExecuteScheduleUpdate --> execute\n");
			Connection_execute(con, NTY_DB_UPDATE_SCHEDULE, did, scheduleId, daily, time, status, details);
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
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog(" ntyExecuteScheduleSelect --> executeQuery\n");
			
			if (pScheduleAck != NULL && pScheduleAck->results.pSchedule != NULL) {
				size_t i = 0;
				ResultSet_T r = Connection_executeQuery(con, NTY_DB_SELECT_SCHEDULE, did);
				if (r != NULL) {
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

						ret = 0;
					}
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
int ntyExecuteTimeTablesUpdate(void *self, C_DEVID aid, C_DEVID did, const char *morning, U8 morning_turn, const char *afternoon,  U8 afternoon_turn, const char *daily, int *result) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_UPDATE_TIMETABLE, did, morning, morning_turn, afternoon, afternoon_turn, daily);
			ntylog(" ntyExecuteTimeTablesUpdate ntyExecuteTimeTablesUpdate --> r ::: \n");
			if (r != NULL) {
				while (ResultSet_next(r)) {
					int temp = ResultSet_getInt(r, 1);
					*result = temp;
					ntylog(" ntyExecuteTimeTablesUpdate temp --> r ::: %d\n", temp);
					ret = 0;
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

//NTY_DB_INSERT_COMMON_MSG
int ntyExecuteCommonMsgInsert(void *self, C_DEVID sid, C_DEVID gid, const char *detatils, int *msg) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog(" ntyExecuteCommonMsgInsert --> start\n");
			
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_COMMON_MSG, sid, gid, detatils);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					*msg = ResultSet_getInt(r, 1);	
					ret = 0;

					ntylog("ntyExecuteCommonMsgInsert msgId : %d\n", *msg);
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

//NTY_DB_INSERT_COMMON_ITEM_MSG
int ntyExecuteCommonItemMsgInsert(void *self, C_DEVID sid, C_DEVID gid, const char *detatils, int *msg) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog(" ntyExecuteCommonItemMsgInsert --> start\n");
			
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_COMMON_ITEM_MSG, sid, gid, detatils);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					*msg = ResultSet_getInt(r, 1);	
					ret = 0;

					ntylog("ntyExecuteCommonMsgInsert msgId : %d\n", *msg);
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


//NTY_DB_DELETE_COMMON_ITEM_MSG
int ntyExecuteCommonItemMsgDelete(void *self, int msgId) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog(" ntyExecuteCommonItemMsgDelete --> start\n");
			
			Connection_execute(con, NTY_DB_DELETE_COMMON_ITEM_MSG, msgId);
			ret = 0;
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



//NTY_DB_INSERT_COMMON_MSG_REJECT
int ntyExecuteCommonMsgToProposerInsert(void *self, C_DEVID sid, C_DEVID gid, const char *detatils, int *msg) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {

			ntylog(" ntyExecuteCommonMsgToProposerInsert --> executeQuery\n");
			
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_COMMON_MSG_REJECT, sid, gid, detatils);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					*msg = ResultSet_getInt(r, 1);	
					ret = 0;
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


//NTY_DB_UPDATE_DEVICE_STATUS
int ntyExecuteChangeDeviceOnlineStatus(void *self, C_DEVID did) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog(" ntyExecuteChangeDeviceOnlineStatus --> Connection_execute\n");
			Connection_execute(con, NTY_DB_UPDATE_DEVICE_STATUS, did, 1);
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
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_DEVICE_STATUS_SELECT_FORMAT, did);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					*online = ResultSet_getInt(r, 1);	
					ret = 0;
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

//NTY_DB_DEVICE_STATUS_RESET_FORMAT
int ntyQueryAppOnlineStatus(void *self, C_DEVID aid, int *online) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_APP_STATUS_SELECT_FORMAT, aid);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					*online = ResultSet_getInt(r, 1);	
					ret = 0;
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

//NTY_DB_INSERT_VOICE_MSG
int ntyQueryVoiceMsgInsert(void *self, C_DEVID senderId, C_DEVID gId, char *filename, int *msgId) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_VOICE_MSG, senderId, gId, filename);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					*msgId = ResultSet_getInt(r, 1);
					ret = 0;				
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

//NTY_DB_DELETE_VOICE_OFFLINE_MSG
int ntyExecuteVoiceOfflineMsgDelete(void *self, int index, C_DEVID userId) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_DELETE_VOICE_OFFLINE_MSG, index, userId);
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
//NTY_DB_SELECT_VOICE_MSG
int ntyQueryVoiceMsgSelect(void *self, int index,C_DEVID *senderId, C_DEVID *gId, U8 *filename, long *stamp) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			ResultSet_T r =Connection_executeQuery(con, NTY_DB_SELECT_VOICE_MSG, index);

			if (r != NULL) {
				while (ResultSet_next(r)) {
					//*id = ResultSet_getInt(r, 1);	
					*senderId = ResultSet_getLLong(r, 2);
					*gId = ResultSet_getLLong(r, 3);

					const char *details = ResultSet_getString(r, 4);
					int length = strlen(details);

					time_t nStamp = ResultSet_getTimestamp(r, 5);
					*stamp = nStamp;

					memcpy(filename, details, length);

					ntylog(" ntyQueryVoiceMsgSelect --> sender:%lld, gId:%lld\n", *senderId, *gId);
					ntylog(" ntyQueryVoiceMsgSelect --> details : %s\n", details);

					ret = 0;
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

//PROC_INSERT_ADMIN_GROUP
int ntyQueryAdminGroupInsert(void *self, C_DEVID devId, U8 *bname, C_DEVID fromId, U8 *userCall, U8 *wimage, U8 *uimage) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			ResultSet_T r =Connection_executeQuery(con, NTY_DB_INSERT_ADMIN_GROUP, devId, bname, fromId, userCall, wimage, uimage);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					ret = ResultSet_getInt(r, 1);	
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

int ntyQueryDevAppGroupCheckSelectHandle(C_DEVID aid, C_DEVID did) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryDevAppGroupCheckSelect(pool, aid, did);
}

int ntyExecuteDevAppGroupBindInsertHandle(int msgId, C_DEVID *proposerId, U8 *phonenum) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteDevAppGroupBindInsert(pool, msgId, proposerId, phonenum);
}

int ntyExecuteBindConfirmDeleteHandle(int msgId, C_DEVID *id) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteBindConfirmDelete(pool, msgId, id);
}


int ntyExecuteCommonOfflineMsgDeleteHandle(int msgId, C_DEVID clientId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteCommonOfflineMsgDelete(pool, msgId, clientId);
}

int ntyQueryCommonOfflineMsgSelectHandle(C_DEVID deviceId, void *container) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryCommonOfflineMsgSelect(pool, deviceId, container);
}

int ntyQueryVoiceOfflineMsgSelectHandle(C_DEVID fromId, void *container) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryVoiceOfflineMsgSelect(pool, fromId, container);
}

int ntyQueryCommonMsgSelectHandle(C_DEVID msgId, C_DEVID *senderId, C_DEVID *groupId, char *json) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryCommonMsgSelect(pool, msgId, senderId, groupId, json);
}

int ntyQueryBindConfirmInsertHandle(C_DEVID admin, C_DEVID imei, U8 *name, U8 *wimage, C_DEVID proposer, U8 *call, U8 *uimage, int *msgId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryBindConfirmInsert(pool, admin, imei, name, wimage, proposer, call, uimage, msgId);
}

int ntyQueryPhoneBookSelectHandle(C_DEVID imei, C_DEVID userId, char *phonenum) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryPhoneBookSelect(pool, imei, userId, phonenum);
}

int ntyQueryAdminSelectHandle(C_DEVID did, C_DEVID *appid) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryAdminSelect(pool, did, appid);
}

int ntyExecuteDevAppGroupDeleteHandle(C_DEVID aid, C_DEVID did) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteDevAppGroupDelete(pool, aid, did);
}

int ntyExecuteLocationInsertHandle(C_DEVID did, U8 *lng, U8 *lat, U8 type, U8 *info) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteLocationInsert(pool, did, lng, lat, type, info);
}

int ntyExecuteLocationNewInsertHandle(C_DEVID did, U8 type, const char *lnglat, const char *info, const char *desc) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteLocationNewInsert(pool, did, type, lnglat, info, desc);
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

int ntyExecuteScheduleInsertHandle(C_DEVID aid, C_DEVID did, const char *daily, const char *time, int status, const char *details, int *scheduleId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteScheduleInsert(pool, aid, did, daily, time, status, details, scheduleId);
}

int ntyExecuteScheduleDeleteHandle(C_DEVID aid, C_DEVID did, int scheduleId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteScheduleDelete(pool, aid, did, scheduleId);
}

int ntyExecuteScheduleUpdateHandle(C_DEVID aid, C_DEVID did, int scheduleId, const char *daily, const char *time, int status, const char *details) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteScheduleUpdate(pool, aid, did, scheduleId, daily, time, status, details);
}

int ntyExecuteScheduleSelectHandle(C_DEVID aid, C_DEVID did, ScheduleAck *pScheduleAck, size_t size) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteScheduleSelect(pool, aid, did, pScheduleAck, size);
}

int ntyExecuteTimeTablesUpdateHandle(C_DEVID aid, C_DEVID did, const char *morning, U8 morning_turn, const char *afternoon,  U8 afternoon_turn, const char *daily, int *result) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteTimeTablesUpdate(pool, aid, did, morning, morning_turn, afternoon, afternoon_turn, daily, result);
}

int ntyExecuteCommonMsgInsertHandle(C_DEVID sid, C_DEVID gid, const char *detatils, int *msg) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteCommonMsgInsert(pool, sid, gid, detatils, msg);
}

int ntyExecuteCommonItemMsgInsertHandle(C_DEVID sid, C_DEVID gid, const char *detatils, int *msg) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteCommonItemMsgInsert(pool, sid, gid, detatils, msg);
}

int ntyExecuteCommonItemMsgDeleteHandle(int msgId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteCommonItemMsgDelete(pool, msgId);
}


int ntyQueryVoiceMsgInsertHandle(C_DEVID senderId, C_DEVID gId, char *filename, U32 *msgId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryVoiceMsgInsert(pool, senderId, gId, filename, msgId);
}

int ntyExecuteVoiceOfflineMsgDeleteHandle(U32 index, C_DEVID userId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteVoiceOfflineMsgDelete(pool, index, userId);
}

int ntyQueryVoiceMsgSelectHandle(U32 index, C_DEVID *senderId, C_DEVID *gId, U8 *filename, long *stamp) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryVoiceMsgSelect(pool, index, senderId, gId, filename, stamp);
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

int ntyQueryAdminGroupInsertHandle(C_DEVID devId, U8 *bname, C_DEVID fromId, U8 *userCall, U8 *wimage, U8 *uimage) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryAdminGroupInsert(pool, devId, bname, fromId, userCall, wimage, uimage);
}

int ntyQueryBindOfflineMsgToAdminSelectHandle(C_DEVID fromId, void *container) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryBindOfflineMsgToAdminSelect(pool, fromId, container);
}

int ntyQueryBindOfflineMsgToProposerSelectHandle(C_DEVID fromId, void *container) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryBindOfflineMsgToProposerSelect(pool, fromId, container);
}

int ntyExecuteCommonMsgToProposerInsertHandle(C_DEVID sid, C_DEVID gid, const char *detatils, int *msg) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteCommonMsgToProposerInsert(pool, sid, gid, detatils, msg);
}

int ntyExecuteChangeDeviceOnlineStatusHandle(C_DEVID did) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteChangeDeviceOnlineStatus(pool, did);
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
#if 0
int main( ) {

	C_DEVID proposerId = 0x0;
	int msgId = 17;
	char phonenum[64] = {0};
	
	
	ntyExecuteDevAppGroupBindInsertHandle(msgId, &proposerId, phonenum);

	ntylog("phonenum: %s, proposerId:%lld\n", phonenum, proposerId);
}
#endif

