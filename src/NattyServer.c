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

//#include "NattyUtils.h"
#include "NattyTcpServer.h"
#include "NattyUdpServer.h"
#include "NattyMulticast.h"
#include "NattyDaveMQ.h"
#include "NattyHash.h"
#include "NattyUtils.h"
#include "NattyHttpCurl.h"
#include "NattySignal.h"
#include "NattyPush.h"


void *ntyStartupTcpServerThread(void *arg) {
	if (arg == NULL) return NULL;
	ntylog(" ... TCP Server Startup ... \n");
	ntyTcpServerRun(arg);
}

void *ntyStartupUdpServerThread(void *arg) {
	if (arg == NULL) return NULL;
	ntylog(" ... UDP Server Startup ... \n");
	ntyUdpServerRun(arg);
} 

void *ntyStartupMulticastServerThread(void *arg) {
	if (arg == NULL) return NULL;
	ntylog(" ... Multicast Server Startup ... \n");
	ntyMulticastServerRun(arg);
} 

void *ntyStartupPushConnectionPoolThread( void *arg ){
	ntylog("... Push connection pool startup ...\n");
	ntyPushConnectionPoolInstance();
}

int main() {
	//void* ntyServerInfo = New(ntyUdpServerInstance());
	int i = 0, rc = -1;
	pthread_t thread_id[PROTO_TYPE_COUNT] = {0}; 

	//ntylog(" ... Server Startup ...\n");
	ntyDisplay();
	
#if ENABLE_SIGNAL_SUBSYSTEM
	ntySignalRegister();
#endif

#if 1 //Curl init
	ntyHttpCurlGlobalInit();
	ntyThreadPoolInit();
	ntyConnectionPoolInit();
#endif	
	ntyDaveMqStart();
	ntylog(" ... Dave Message Queue Startup ... \n");
	//void *pHash = ntyHashTableInstance();

	ntylog(" ... Hash Table Startup ... \n");
	for (i = 0;i < PROTO_TYPE_COUNT;i ++) {
		if (i == PROTO_TYPE_TCP) { //startup tcp server
			void *server = ntyTcpServerInstance();
			ntylog("ntyTcpServerInstance\n");
			rc = pthread_create(&thread_id[i], NULL, ntyStartupTcpServerThread, server);
			if (rc) {
				ntylog("ERROR; return code is %d\n", rc);
			}
		} else if (i == PROTO_TYPE_UDP) { //startup udp server
			void *server = ntyUdpServerInstance();
			ntylog("ntyUdpServerInstance\n");
			rc = pthread_create(&thread_id[i], NULL, ntyStartupUdpServerThread, server);
			if (rc) {
				ntylog("ERROR; return code is %d\n", rc);
			}
		} else if (i == PROTO_TYPE_MULTICAST) { //startup multicast server
#if ENABLE_MULTICAST_SYNC
			void *server = ntyMulticastServerInstance();
			ntylog("ntyMulticastServerInstance\n");
			rc = pthread_create(&thread_id[i], NULL, ntyStartupMulticastServerThread, server);
			if (rc) {
				ntylog("ERROR; return code is %d\n", rc);
			}
#endif
		}
	}

	#if 1
	pthread_t pushThreadId;
	int nRet = pthread_create( &pushThreadId, NULL, ntyStartupPushConnectionPoolThread, NULL );
	if ( nRet != 0 ){
		ntylog( "ntyStartupPushConnectionPoolThread error,return code:%d\n",nRet );
	}
	#endif	

	for (i = 0;i < PROTO_TYPE_COUNT;i ++) {
		pthread_join(thread_id[i], NULL);
	}
	pthread_join( pushThreadId, NULL );
	
	return 0;
}




