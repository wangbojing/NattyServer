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




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <pthread.h>
#include <poll.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <netinet/tcp.h>


#include "NattyNetwork.h"
#include "NattyResult.h"


static int count = 0;
static U32 msgAck = 0x0;
static int level = LEVEL_LOGIN;
static C_DEVID devid = 0;
static C_DEVID friendId = 0;
static C_DEVID tempId = 0;
static U32 ackNum = 0;
static int sockfd_local = 0;

static int ntyNetworkResendFrame(void *_self);
static int ntyClientSocketReconnect(void *self);
void sendP2PConnectAck(C_DEVID friId, U32 ack);
void sendProxyDataPacketAck(C_DEVID friId, U32 ack);
void sendP2PDataPacketAck(C_DEVID friId, U32 ack);
void sendP2PConnectNotifyAck(C_DEVID friId, U32 ack);
void sendP2PConnectNotify(C_DEVID fromId, C_DEVID toId);
C_DEVID ntyGetDestDevId(void *self);



#define NTY_CRCTABLE_LENGTH			256
#define NTY_CRC_KEY		0x04c11db7ul
static U32 u32CrcTable[NTY_CRCTABLE_LENGTH] = {0};
void ntyGenCrcTable(void) {	
	U16 i,j;	
	U32 u32CrcNum = 0;	
	for (i = 0;i < NTY_CRCTABLE_LENGTH;i ++) {		
		U32 u32CrcNum = (i << 24);		
		for (j = 0;j < 8;j ++) {			
			if (u32CrcNum & 0x80000000L) {				
				u32CrcNum = (u32CrcNum << 1) ^ NTY_CRC_KEY;
			} else {				
				u32CrcNum = (u32CrcNum << 1);			
			}		
		}		
		u32CrcTable[i] = u32CrcNum;	
	}
}

U32 ntyGenCrcValue(U8 *buf, int length) {	
	U32 u32CRC = 0xFFFFFFFF;		
	while (length -- > 0) {		
		u32CRC = (u32CRC << 8) ^ u32CrcTable[((u32CRC >> 24) ^ *buf++) & 0xFF];	
	}	
	return u32CRC;
}

#if 0
void ntyMessageOnDataLost(void) {
	////////////////////////////////////////////////////////////
	//ntyCancelTimer();
	////////////////////////////////////////////////////////////
	ntydbg("ntyMessageOnDataLost\n");

	void *pNetwork = ntyNetworkInstance();
	U8 u8ReqType = ntyGetReqType(pNetwork);
	if (u8ReqType == NTY_PROTO_P2P_HEARTBEAT_REQ) {
		C_DEVID destDevId = ntyGetDestDevId(pNetwork);
		void *pTree = ntyRBTreeInstance();
		FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, destDevId);
		if(pFriend->counter > P2P_HEARTBEAT_TIMEOUT_COUNTR) {
			pFriend->isP2P = 0;
			//don't take reconnect
		}
	}
	
	//void* pTimer = ntyNetworkTimerInstance();
	//ntyStopTimer(pTimer);
#if 0
	if (LEVEL_P2PCONNECT == level) {
		level = LEVEL_P2PCONNECT;
	}
#endif
}

#endif

void ntyMessageOnAck(int signo) {
	//ntylog(" Get a Sigalarm, %d counts! \n", ++count);
#if 0
	void *pNetwork = ntyNetworkInstance();
	if (++count > SENT_TIMEOUT) {
#if 0 // this action should post to java 
		ntyMessageOnDataLost();
#else
		if (pNetwork && ((Network *)pNetwork)->onDataLost) {
			((Network *)pNetwork)->onDataLost(count);
			count = 0;
		}
#endif
	} else {
		ntyNetworkResendFrame(pNetwork);
	}
#endif
}

void error(char *msg) {    
	perror(msg);    
	exit(0);
}


static int ntyClientStartup(Network *network, const char *host, const char *service) {
	int res = -1;
	struct addrinfo *result, *rp;
	struct addrinfo hints;

	trace(" ntyClientStartup Enter\n");
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;          /* Any protocol */

	res = getaddrinfo(host, service, &hints, &result);
	if (res != 0) {
		fprintf(stderr, "getaddrinfo: %d %s\n", res,  gai_strerror(res));
		return NTY_RESULT_FAILED;
	}
#if 0
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		network->sockfd = socket(AF_INET, SOCK_STREAM, 0)
	}
#else
	rp = result;
	while (rp != NULL) {
		network->sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (network->sockfd < 0) continue;

		if (rp->ai_family == AF_INET) {
			struct sockaddr_in *addr = (struct sockaddr_in*)(rp->ai_addr);
			addr->sin_port = htons(SERVER_PORT);
			char *p = inet_ntoa(addr->sin_addr);
			trace("address : %s\n", p);
		} else if (rp->ai_family == AF_INET6) {
			struct sockaddr_in6 *addr = (struct sockaddr_in6*)(rp->ai_addr);
			addr->sin6_port = htons(SERVER_PORT);
		}

		res = connect(network->sockfd, rp->ai_addr, rp->ai_addrlen);
		if (res == 0) {
			trace(" Connect Server: %s Success\n", SERVER_HOSTNAME);
#if 0
			memcpy(&network->addr, rp->ai_addr, rp->ai_addrlen);
#else
			memcpy(&network->addr, rp, sizeof(struct addrinfo));
#endif
			break;
		}

		close(network->sockfd);
		network->sockfd = -1;
		rp = rp->ai_next;
	}

	freeaddrinfo(result);

	return NTY_RESULT_SUCCESS;
#endif
}

static int ntyClientShutdown(Network *network) {
	close(network->sockfd);
	network->sockfd = -1;
	return NTY_RESULT_SUCCESS;
}

static void* ntyClientSocketCtor(void *self, va_list *params) {
	int res = -1;
	Network *network = self;
	network->sockfd = -1;

	if (network->addr.ai_family == AF_INET) {
		struct sockaddr_in *addr = (struct sockaddr_in*)(network->addr.ai_addr);
		addr->sin_port = htons(SERVER_PORT);
		char *p = inet_ntoa(addr->sin_addr);
		trace("address : %s\n", p);

		if (*(unsigned char*)(&addr->sin_addr.s_addr) == 0x0) {
			res = ntyClientStartup(network, SERVER_HOSTNAME, "echo");
		} else {
			res = ntyClientSocketReconnect(network);
			if (res != NTY_RESULT_SUCCESS) {
				res = ntyClientStartup(network, SERVER_HOSTNAME, "echo");
			}
		}
		
	} else {
		res = ntyClientStartup(network, SERVER_HOSTNAME, "echo");
	}

	return network;
}

static void* ntyClientSocketDtor(void *self) {
	Network *network = self;
	ntyClientShutdown(network);
	
	return self;
}

static int ntyClientSocketSendFrame(void *self, U8 *buffer, int len) {
	ClientSocket *nSocket = self;
	buffer[NTY_PROTO_DEVTYPE_IDX] = NTY_PROTO_SELFTYPE;
	*(U32*)(&buffer[len-sizeof(U32)]) = ntyGenCrcValue(buffer, len-sizeof(U32));
	
	return send(nSocket->sockfd, buffer, len, 0);
}

static int ntyClientSocketRecvFrame(void *self, U8 *buffer, int len) {
	ClientSocket *nSocket = self;

	return recv(nSocket->sockfd, buffer, len, 0);
}

static int ntyClientSocketReconnect(void *self) {
	Network *network = self;

	struct addrinfo *rp = &network->addr;

	if (0 != connect(network->sockfd, rp->ai_addr, rp->ai_addrlen)) {
		return NTY_RESULT_FAILED;
	}

	return NTY_RESULT_SUCCESS;
}


static const ClientSocketHandle ntyClientSocketHandle = {
	sizeof(Network),
	ntyClientSocketCtor,
	ntyClientSocketDtor,
	ntyClientSocketSendFrame,
	ntyClientSocketRecvFrame,
	NULL,
	ntyClientSocketReconnect,
};
const void *pNtyClientSocketHandle = &ntyClientSocketHandle;

static void *pClientSocket = NULL;

void *ntyNetworkInstance(void) { //Singleton
	if (pClientSocket == NULL) {
		pClientSocket = New(pNtyClientSocketHandle);
	}
	return pClientSocket;
}

void* ntyNetworkRelease(void) {	 //Singleton
	if (pClientSocket != NULL) {
		Delete(pClientSocket);
		pClientSocket = NULL;
	}
	return NULL;
}

int ntySendFrame(void *self,U8 *buffer, int len) {
	const ClientSocketHandle *const * pClientSocketHandle = self;

	if (self && (*pClientSocketHandle) && (*pClientSocketHandle)->send) {
		return (*pClientSocketHandle)->send(self, buffer, len);
	}
	return NTY_RESULT_FAILED;
}

int ntyRecvFrame(void *self, U8 *buffer, int len) {
	const ClientSocketHandle *const * pClientSocketHandle = self;

	if (self && (*pClientSocketHandle) && (*pClientSocketHandle)->recv) {
		return (*pClientSocketHandle)->recv(self, buffer, len);
	}
	return NTY_RESULT_FAILED;
}

int ntyReconnect(void *self) {
	const ClientSocketHandle *const * pClientSocketHandle = self;

	if (self && (*pClientSocketHandle) && (*pClientSocketHandle)->reconnect) {
		return (*pClientSocketHandle)->reconnect(self);
	}
	return NTY_RESULT_FAILED;
}

int ntyGetSocket(void *self) {
	Network *network = self;
	return network->sockfd;
}

U8 ntyGetReqType(void *self) {
	Network *network = self;
	//return network->buffer[NTY_PROTO_MSGTYPE_IDX];
	return NTY_PROTO_VERSION;
}


C_DEVID ntyGetDestDevId(void *self) {
	Network *network = self;
	C_DEVID destId = 0;

	//memcpy(&destId, &network->buffer[NTY_PROTO_DEST_DEVID_IDX], sizeof(C_DEVID));
	
	return destId;
}

