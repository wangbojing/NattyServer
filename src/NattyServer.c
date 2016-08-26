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
#include "NattyDaveMQ.h"

void *ntyStartupTcpServerThread(void *arg) {
	if (arg == NULL) return NULL;
	ntydbg(" ... TCP Server Startup ... \n");
	ntyTcpServerRun(arg);
}

void *ntyStartupUdpServerThread(void *arg) {
	if (arg == NULL) return NULL;
	ntydbg(" ... UDP Server Startup ... \n");
	ntyUdpServerRun(arg);
} 

int main() {
	//void* ntyServerInfo = New(ntyUdpServerInstance());
	int i = 0, rc = -1;
	pthread_t thread_id[PROTO_TYPE_COUNT] = {0}; 
	
	ntyDisplay();
	ntyDaveMqStart();

	
	ntydbg("Server Startup\n");
	for (i = 0;i < PROTO_TYPE_COUNT;i ++) {
		if (i == PROTO_TYPE_TCP) { //startup tcp server
			void *server = ntyTcpServerInstance();
			ntydbg("ntyTcpServerInstance\n");
			rc = pthread_create(&thread_id[i], NULL, ntyStartupTcpServerThread, server);
			if (rc) {
				ntylog("ERROR; return code is %d\n", rc);
			}
		} else if (i == PROTO_TYPE_UDP) { //startup udp server
			void *server = ntyUdpServerInstance();
			ntydbg("ntyUdpServerInstance\n");
			rc = pthread_create(&thread_id[i], NULL, ntyStartupUdpServerThread, server);
			if (rc) {
				ntylog("ERROR; return code is %d\n", rc);
			}
		}
	}

	for (i = 0;i < PROTO_TYPE_COUNT;i ++) {
		pthread_join(thread_id[i], NULL);
	}

	return 0;
}




