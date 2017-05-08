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


#include "NattyHBD.h"
//#include "NattyUdpServer.h"

#include <sys/time.h>
#include <unistd.h>

#if ENABLE_NATTY_TIME_STAMP
extern pthread_mutex_t time_mutex;
#endif

#if 0
int ntyHeartBeatDetectItem(void *node, void *mainloop, TIMESTAMP curStamp) {
#if 0
	Client *client = node;
#else
	RBTreeNode *rNode = node;
	Client *client = rNode->value; 
#endif
	
	struct ev_loop *loop = mainloop;
	TIMESTAMP duration = curStamp - client->stamp;

	if (client == NULL) {
		ntylog(" ntyHeartBeatDetectItem --> client node is Null\n");
		return -1;
	}
	ntylog(" ntyHeartBeatDetectItem --> client:%lld,  rNode:%lld\n", client->devId, rNode->key);
#if 0
	if (client->devId != rNode->key) {
		ntylog(" ntyHeartBeatDetectItem --> client->devId != rNode->key\n");
#if 1 //RBTREE after before
		client->devId = rNode->key;
#endif
	}
#endif
	if (client->devId == NATTY_NULL_DEVID) {
#if 0		
		ntylog("Client DevID == 0 ---> Addr : %d.%d.%d.%d:%d \n", *(unsigned char*)(&client->addr.sin_addr.s_addr), *((unsigned char*)(&client->addr.sin_addr.s_addr)+1),													
				*((unsigned char*)(&client->addr.sin_addr.s_addr)+2), *((unsigned char*)(&client->addr.sin_addr.s_addr)+3),													
				client->addr.sin_port);	
#endif		
		return 0;
	}

	if (duration >  NATTY_HEARTBEAT_THRESHOLD) { //release client
		//U8 u8ResultBuffer[256] = {0};
		ntylog(" ntyHeartBeatDetectItem --> client:%lld timeout\n", client->devId);
#if 0
		ntylog(" ntyHeartBeatDetectItem --> Notify All Friends\n");
		sprintf(u8ResultBuffer, "Set Disconnect 1");
		ntyBoardcastAllFriendsById(client->devId, u8ResultBuffer, strlen(u8ResultBuffer));
#else
		ntyBoardcastAllFriendsNotifyDisconnect(client->devId);
#endif

		ntylog(" ntyHeartBeatDetectItem --> Release Client Node Id: %lld\n", client->devId);
		if (client->watcher == NULL) {
			ntylog(" ntyHeartBeatDetectItem --> client->watcher is Null\n");
		} 
		if (mainloop == NULL) {
			ntylog(" ntyHeartBeatDetectItem --> mainloop is Null\n");
		}
#if 0
		if(0 == ntyReleaseClientNodeByAddr(mainloop, &client->addr, client->watcher)) {
			ntylog("Release Client Node Success\n");
		} else {
			ntylog("Release Client Node Failed\n");
		}
#else
		if (0 == ntyReleaseClientNodeByNode(mainloop, client)) {
			ntylog("Release Client Node Success\n");
		} else {
			ntylog("Release Client Node Failed\n");
		}
#endif
	}

	//usleep(1);
	
	return 0;
}


void ntyHeartBeatDetectTraversal(void *mainloop) {
	TIMESTAMP curStamp = 0;
#if ENABLE_NATTY_TIME_STAMP
#if 0
	pthread_mutex_lock(&time_mutex);
	curStamp = time(NULL);
	pthread_mutex_unlock(&time_mutex);
#else
	curStamp = ntyTimeStampGenrator();
#endif
#endif
	ntylog(" ntyHeartBeatDetectTraversal --> Start\n");

	void *pTree = ntyRBTreeInstance();
	if (curStamp == 0){ 
		ntylog(" ntyHeartBeatDetectTraversal --> TimeStamp value is Error\n");
		return ;
	}
	ntyRBTreeHeartBeatDetect(pTree, ntyHeartBeatDetectItem, mainloop, curStamp);
}
#else

int ntyCheckOnlineAlarmNotify(NITIMER_ID id, void *arg, int len) {
	void **pArg = arg;
	Client *client = *pArg;

	
	return NTY_RESULT_SUCCESS;
}


static int ntyHeartBeatDetectItem(void *client, C_DEVID clientId) {
	TIMESTAMP stamp = 0;
	
	ntylog("ntyHeartBeatDetectItem --> clientId:%lld\n", clientId);
	
	BPTreeHeap *heap = ntyBHeapInstance();
	NRecord *record = ntyBHeapSelect(heap, clientId);
	
	
	if (record == NULL) {
		ntylog(" ntyHashMapTraversal --> %lld is not exist\n", clientId);
		return NTY_RESULT_NOEXIST;
	}

	Client* pClient = (Client*)record->value;
	if (pClient == NULL) {
		ntylog(" ntyHashMapTraversal --> pClient == NULL\n");
		return NTY_RESULT_NOEXIST;
	}

	if (pClient->stamp == 0) return NTY_RESULT_FAILED;
	
#if ENABLE_NATTY_TIME_STAMP //TIME Stamp 	
	stamp = ntyTimeStampGenrator();
#endif

	TIMESTAMP dur_time = stamp - pClient->stamp;

	if (dur_time > NATTY_HEARTBEAT_THRESHOLD && pClient->deviceType == NTY_PROTO_CLIENT_WATCH) { //timeout
		ntylog("ntyHeartBeatDetectItem timeout --> %lld, prepare to cleanup client \n", clientId);
		ntyClientCleanup(client);
	}

	return 0;
}


int ntyHashMapTraversal(void *client, C_DEVID clientId) {
	return ntyHeartBeatDetectItem(client, clientId);
}




#endif


