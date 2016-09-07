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
#include "NattyUdpServer.h"
#include "NattyConfig.h"
#include "NattyRBTree.h"
#include "NattyFilter.h"

#include <sys/time.h>

#if ENABLE_NATTY_TIME_STAMP
extern pthread_mutex_t time_mutex;
#endif
int ntyHeartBeatDetectItem(void *node, void *mainloop, TIMESTAMP curStamp) {
	Client *client = node;
	struct ev_loop *loop = mainloop;
	TIMESTAMP duration = curStamp - client->stamp;

	if (client == NULL) {
		ntylog(" ntyHeartBeatDetectItem --> client node is Null\n");
		return -1;
	}

	ntylog(" ntyHeartBeatDetectItem --> client:%lld last connect:%ld, now:%ld\n", client->devId, client->stamp, curStamp);
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
		if(0 == ntyReleaseClientNodeByAddr(mainloop, &client->addr, client->watcher)) {
			ntylog("Release Client Node Success\n");
		} else {
			ntylog("Release Client Node Failed\n");
		}
	}

	return 0;
}


void ntyHeartBeatDetectTraversal(void *mainloop) {
	TIMESTAMP curStamp = 0;
#if ENABLE_NATTY_TIME_STAMP
	pthread_mutex_lock(&time_mutex);
	curStamp = time(NULL);
	pthread_mutex_unlock(&time_mutex);
#endif
	ntylog(" ntyHeartBeatDetectTraversal --> Start\n");

	void *pTree = ntyRBTreeInstance();
	if (curStamp == 0){ 
		ntylog(" ntyHeartBeatDetectTraversal --> TimeStamp value is Error\n");
		return ;
	}
	ntyRBTreeHeartBeatDetect(pTree, ntyHeartBeatDetectItem, mainloop, curStamp);
}




