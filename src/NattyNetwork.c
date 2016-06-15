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
#include <netdb.h> 
#include <pthread.h>
#include <poll.h>
#include <sys/time.h>
#include <signal.h>

#include "NattyNetwork.h"



static int count = 0;
static U32 msgAck = 0x0;
static int level = LEVEL_LOGIN;
static C_DEVID devid = 0;
static C_DEVID friendId = 0;
static C_DEVID tempId = 0;
static U32 ackNum = 0;
static int sockfd_local = 0;

static int ntyNetworkResendFrame(void *_self);
void sendP2PConnectAck(C_DEVID friId, U32 ack);
void sendProxyDataPacketAck(C_DEVID friId, U32 ack);
void sendP2PDataPacketAck(C_DEVID friId, U32 ack);
void sendP2PConnectNotifyAck(C_DEVID friId, U32 ack);
void sendP2PConnectNotify(C_DEVID fromId, C_DEVID toId);
U8 ntyGetReqType(void *self);
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


void ntyMessageOnDataLost(void) {
	////////////////////////////////////////////////////////////
	//ntyCancelTimer();
	////////////////////////////////////////////////////////////
	printf("ntyMessageOnDataLost\n");

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
	
	void* pTimer = ntyNetworkTimerInstance();
	ntyStopTimer(pTimer);
#if 0
	if (LEVEL_P2PCONNECT == level) {
		level = LEVEL_P2PCONNECT;
	}
#endif
}

void ntyMessageOnAck(int signo) {
	//printf(" Get a Sigalarm, %d counts! \n", ++count);
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
}

void error(char *msg) {    
	perror(msg);    
	exit(0);
}

//(struct sockaddr *)&client->addr

static void* ntyNetworkCtor(void *_self, va_list *params) {
	Network *network = _self;
	network->onAck = ntyMessageOnAck;
	network->ackNum = 1;

#if 1 //Socket Init
	network->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (network->sockfd < 0) {
		error(" ERROR opening socket");
	}
	printf(" sockfd %d, %s, %d\n",network->sockfd, __FILE__, __LINE__);
#endif
	
	return network;
}

static void* ntyNetworkDtor(void *_self) {
	return _self;
}

static int ntyNetworkResendFrame(void *_self) {
	Network *network = _self;

	return sendto(network->sockfd, network->buffer, network->length, 0, 
			(struct sockaddr *)&network->addr, sizeof(struct sockaddr_in));
}


static int ntyNetworkSendFrame(void *_self, struct sockaddr_in *to, U8 *buf, int len) {
	//ntyStartTimer();
	Network *network = _self;	
	void* pTimer = ntyNetworkTimerInstance();
	if (buf[NTY_PROTO_MESSAGE_TYPE] != MSG_ACK) {
		ntyStartTimer(pTimer, network->onAck);	
		network->ackNum ++;
	}
	
	memcpy(&network->addr, to, sizeof(struct sockaddr_in));
	bzero(network->buffer, CACHE_BUFFER_SIZE);
	memcpy(network->buffer, buf, len);
	
	if (buf[NTY_PROTO_MESSAGE_TYPE] == MSG_REQ) {
		*(U32*)(&network->buffer[NTY_PROTO_ACKNUM_IDX]) = network->ackNum;
	}
	network->length = len;
	*(U32*)(&network->buffer[len-sizeof(U32)]) = ntyGenCrcValue(network->buffer, len-sizeof(U32));

	return sendto(network->sockfd, network->buffer, network->length, 0, 
		(struct sockaddr *)&network->addr, sizeof(struct sockaddr_in));
}

static int ntyNetworkRecvFrame(void *_self, U8 *buf, int len, struct sockaddr_in *from) {
	//ntyStartTimer();

	int n = 0;
	int clientLen = sizeof(struct sockaddr);
	struct sockaddr_in addr = {0};

	Network *network = _self;
	n = recvfrom(network->sockfd, buf, CACHE_BUFFER_SIZE, 0, (struct sockaddr*)&addr, (socklen_t*)&clientLen);
	U32 ackNum = *(U32*)(&buf[NTY_PROTO_ACKNUM_IDX]);

	memcpy(from, &addr, clientLen);
	if (buf[NTY_PROTO_MESSAGE_TYPE] == MSG_ACK) { //recv success		
		if (ackNum == network->ackNum + 1) {
			// CRC 
			
			// stop timer
			void* pTimer = ntyNetworkTimerInstance();
			ntyStopTimer(pTimer);

			return n;	
		} else {
			return -1;
		}
	} else if (buf[NTY_PROTO_MESSAGE_TYPE] == MSG_RET) {
		void* pTimer = ntyNetworkTimerInstance();
		ntyStopTimer(pTimer);

		//have send object
	} else if (buf[NTY_PROTO_MESSAGE_TYPE] == MSG_UPDATE) {
		void* pTimer = ntyNetworkTimerInstance();
		ntyStopTimer(pTimer);
	}

	return n;
	
}

static const NetworkOpera ntyNetworkOpera = {
	sizeof(Network),
	ntyNetworkCtor,
	ntyNetworkDtor,
	ntyNetworkSendFrame,
	ntyNetworkRecvFrame,
	ntyNetworkResendFrame,
};

const void *pNtyNetworkOpera = &ntyNetworkOpera;

static void *pNetworkOpera = NULL;

void *ntyNetworkInstance(void) {
	if (pNetworkOpera == NULL) {
		pNetworkOpera = New(pNtyNetworkOpera);
	}
	return pNetworkOpera;
}

void ntyNetworkRelease(void *self) {	
	return Delete(self);
}


int ntySendFrame(void *self, struct sockaddr_in *to, U8 *buf, int len) {
	const NetworkOpera *const * pNetworkOpera = self;

	if (self && (*pNetworkOpera) && (*pNetworkOpera)->send) {
		return (*pNetworkOpera)->send(self, to, buf, len);
	}
	return -1;
}

int ntyRecvFrame(void *self, U8 *buf, int len, struct sockaddr_in *from) {
	const NetworkOpera *const * pNetworkOpera = self;

	if (self && (*pNetworkOpera) && (*pNetworkOpera)->recv) {
		return (*pNetworkOpera)->recv(self, buf, len, from);
	}
	return -2;
}

int ntyGetSocket(void *self) {
	Network *network = self;
	return network->sockfd;
}

U8 ntyGetReqType(void *self) {
	Network *network = self;
	return network->buffer[NTY_PROTO_TYPE_IDX];
}

C_DEVID ntyGetDestDevId(void *self) {
	Network *network = self;
	return *(C_DEVID*)(&network->buffer[NTY_PROTO_DEST_DEVID_IDX]);
}

#if 0

struct sockaddr_in serveraddr;
int portno;
static U8 heartbeartRun = 0;
static U8 p2pHeartbeatRun = 0;

/*
 * heartbeat Packet
 * VERSION					1			BYTE
 * MESSAGE TYPE				1			BYTE (req, ack)
 * TYPE					1			BYTE 
 * DEVID					8			BYTE
 * ACKNUM					4			BYTE (Network Module Set Value)
 * CRC 					4			BYTE (Network Module Set Value)
 * 
 * send to server addr
 */

void *heartbeatThread(void *arg) {
	ThreadArg threadArg = *((ThreadArg*)arg);	
	//int devid = threadArg.devid;	
	//int sockfd = threadArg.sockfd;	
	int len, n;	
	U8 buf[NTY_LOGIN_ACK_LENGTH] = {0};	

	printf(" heartbeatThread running\n");
	if (heartbeartRun == 1) {		
		heartbeartRun = 1;		
		return NULL;	
	}	
	heartbeartRun = 1;
	void *pNetwork = ntyNetworkInstance();

	while (1) {		
		bzero(buf, NTY_LOGIN_ACK_LENGTH);
		sleep(HEARTBEAT_TIMEOUT);	
		
		buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;	
		buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_REQ;	
		buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_HEARTBEAT_REQ;		
		*(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]) = devid;
		
		len = NTY_PROTO_LOGIN_REQ_CRC_IDX+sizeof(U32);
		
		n = ntySendFrame(pNetwork, &serveraddr, buf, len);
	}
}



/*
 * p2p heartbeat ack
 * VERSION					1			BYTE
 * MESSAGE TYPE				1			BYTE (req, ack)
 * TYPE					1			BYTE 
 * DEVID					8			BYTE
 * ACKNUM					4			BYTE (Network Module Set Value)
 * DEST_DEVI				8			BYTE 
 * CRC 					4			BYTE (Network Module Set Value)
 * 
 * send to server addr
 */
void sendP2PHeartbeatAck(C_DEVID fromId, C_DEVID toId)  {
	U8 notify[NTY_LOGIN_ACK_LENGTH] = {0};
	int len = 0, n;

	void *pTree = ntyRBTreeInstance();
	FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, toId);
	if (pFriend == NULL || pFriend->isP2P == 0) {
		printf(" Client Id : %lld, P2P is not Success\n", toId);
		return ;
	}
	
	notify[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	notify[NTY_PROTO_MESSAGE_TYPE] = (U8)MSG_ACK;
	notify[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2P_HEARTBEAT_ACK;

	
	*(C_DEVID*)(&notify[NTY_PROTO_DEVID_IDX]) = fromId;
	*(C_DEVID*)(&notify[NTY_PROTO_DEST_DEVID_IDX]) = toId;
	
	len = NTY_PROTO_CRC_IDX + sizeof(U32);

	struct sockaddr_in friendaddr;
	friendaddr.sin_family = AF_INET;
	friendaddr.sin_addr.s_addr = pFriend->addr;				
	friendaddr.sin_port = pFriend->port;

	//pFriend->counter ++; //timeout count
	void *pNetwork = ntyNetworkInstance();
	n = ntySendFrame(pNetwork, &friendaddr, notify, len);
}


/*
 * p2p heartbeat Packet
 * VERSION					1			BYTE
 * MESSAGE TYPE				1			BYTE (req, ack)
 * TYPE					1			BYTE 
 * DEVID					8			BYTE
 * ACKNUM					4			BYTE (Network Module Set Value)
 * DEST_DEVI				8			BYTE 
 * CRC 					4			BYTE (Network Module Set Value)
 * 
 * send to server addr
 */
void sendP2PHeartbeat(C_DEVID fromId, C_DEVID toId)  {
	U8 notify[NTY_LOGIN_ACK_LENGTH] = {0};
	int len = 0, n;

	void *pTree = ntyRBTreeInstance();
	FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, toId);
	if (pFriend == NULL || pFriend->isP2P == 0) {
		printf(" Client Id : %lld, P2P is not Success, state:%d\n", toId, pFriend->isP2P);
		return ;
	}
	
	notify[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	notify[NTY_PROTO_MESSAGE_TYPE] = (U8)MSG_REQ;
	notify[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2P_HEARTBEAT_REQ;

	
	*(C_DEVID*)(&notify[NTY_PROTO_DEVID_IDX]) = fromId;
	*(C_DEVID*)(&notify[NTY_PROTO_DEST_DEVID_IDX]) = toId;
	
	len = NTY_PROTO_CRC_IDX + sizeof(U32);

	struct sockaddr_in friendaddr;
	friendaddr.sin_family = AF_INET;
	friendaddr.sin_addr.s_addr = pFriend->addr;				
	friendaddr.sin_port = pFriend->port;

	pFriend->counter ++; //timeout count
	void *pNetwork = ntyNetworkInstance();
	n = ntySendFrame(pNetwork, &friendaddr, notify, len);
}

void *heartbeatP2PThread(void *arg) {
	void *pTree = ntyRBTreeInstance();

	printf(" heartbeatP2PThread running\n");
	if (p2pHeartbeatRun == 0) {
		p2pHeartbeatRun = 1;
	} else {
		return NULL;
	}
	while (1) {
		sleep(P2P_HEARTBEAT_TIMEOUT);
		ntyFriendsTreeTraversalNotify(pTree, devid, sendP2PHeartbeat);
	}
}


void* recvThread(void *arg) {
	ThreadArg threadArg = *((ThreadArg*)arg);
	int devid = threadArg.devid;
	int sockfd = threadArg.sockfd;
	int ret, n, err;

	U8 buf[RECV_BUFFER_SIZE] = {0};
	struct sockaddr_in addr;
	int clientLen = sizeof(struct sockaddr_in);
	
	void *pNetwork = ntyNetworkInstance();

	struct pollfd fds;
	fds.fd = ntyGetSocket(pNetwork);
	fds.events = POLLIN;

	printf(" recvThread running\n");
	while (1) {
		ret = poll(&fds, 1, 5);
		if (ret) {
			bzero(buf, RECV_BUFFER_SIZE);
			//n = recvfrom(sockfd, buf, RECV_BUFFER_SIZE, 0, (struct sockaddr *) &addr, &clientLen);
			n = ntyRecvFrame(pNetwork, buf, RECV_BUFFER_SIZE, &addr);
#if 1
			printf("%d.%d.%d.%d:%d size:%d --> %x\n", *(unsigned char*)(&addr.sin_addr.s_addr), *((unsigned char*)(&addr.sin_addr.s_addr)+1),	
				*((unsigned char*)(&addr.sin_addr.s_addr)+2), *((unsigned char*)(&addr.sin_addr.s_addr)+3),	 addr.sin_port,
				n, buf[NTY_PROTO_TYPE_IDX]);
#endif			
			if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_LOGIN_ACK) {
			//NTY_PROTO_LOGIN_ACK
				int i = 0;
				
				int count = *(U16*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDS_COUNT_IDX]);
				void *pTree = ntyRBTreeInstance();

				for (i = 0;i < count;i ++) {
					C_DEVID friendId = *(C_DEVID*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_DEVID_IDX(i)]);

					FriendsInfo *friendInfo = ntyRBTreeInterfaceSearch(pTree, friendId);
					if (NULL == friendInfo) {
						FriendsInfo *pFriend = (FriendsInfo*)malloc(sizeof(FriendsInfo));
						assert(pFriend);
						pFriend->sockfd = sockfd;
						pFriend->isP2P = 0;
						pFriend->counter = 0;
						pFriend->addr = *(U32*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_ADDR_IDX(i)]);
						pFriend->port = *(U16*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_PORT_IDX(i)]);
						ntyRBTreeInterfaceInsert(pTree, friendId, pFriend);
					} else {
						friendInfo->sockfd = sockfd;
						friendInfo->isP2P = 0;
						friendInfo->counter = 0;
						friendInfo->addr = *(U32*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_ADDR_IDX(i)]);
						friendInfo->port = *(U16*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_PORT_IDX(i)]);
					}					
				}
#if 0		//cancel p2p notify, client setup p2p channel		
				level = LEVEL_P2PCONNECT_NOTIFY;
#else			
				level = LEVEL_DATAPACKET;
#endif
				//printf("NTY_PROTO_LOGIN_ACK\n");
				
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_NOTIFY_ACK) {
			//NTY_PROTO_P2P_NOTIFY_ACK
				//P2PConnect Notify Success
				//void *pTree = ntyRBTreeInstance();
				//sendP2PConnectReq(pTree, friendId);
				//
				friendId = *(C_DEVID*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]);
				void *pTree = ntyRBTreeInstance();
				FriendsInfo *friendInfo = ntyRBTreeInterfaceSearch(pTree, friendId);

				printf("%d.%d.%d.%d:%d\n", *(unsigned char*)(&friendInfo->addr), *((unsigned char*)(&friendInfo->addr)+1),	
				*((unsigned char*)(&friendInfo->addr)+2), *((unsigned char*)(&friendInfo->addr)+3),	 friendInfo->port
				);
				
				printf(" Start to Connect P2P client\n");
				level = LEVEL_P2PCONNECTFRIEND;
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_HEARTBEAT_ACK) {
			//NTY_PROTO_HEARTBEAT_ACK
				if (buf[NTY_PROTO_MESSAGE_TYPE] == MSG_UPDATE) {
					int i = 0;
					
					int count = *(U16*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDS_COUNT_IDX]);
					void *pTree = ntyRBTreeInstance();

					for (i = 0;i < count;i ++) {
						C_DEVID friendId = *(C_DEVID*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_DEVID_IDX(i)]);

						FriendsInfo *friendInfo = ntyRBTreeInterfaceSearch(pTree, friendId);
						if (NULL == friendInfo) {
							FriendsInfo *pFriend = (FriendsInfo*)malloc(sizeof(FriendsInfo));
							assert(pFriend);
							pFriend->sockfd = sockfd;
							pFriend->isP2P = 0;
							pFriend->counter = 0;
							pFriend->addr = *(U32*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_ADDR_IDX(i)]);
							pFriend->port = *(U16*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_PORT_IDX(i)]);
							ntyRBTreeInterfaceInsert(pTree, friendId, pFriend);
						} else {
							friendInfo->sockfd = sockfd;
							friendInfo->isP2P = 0;
							friendInfo->counter = 0;
							friendInfo->addr = *(U32*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_ADDR_IDX(i)]);
							friendInfo->port = *(U16*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_PORT_IDX(i)]);
						}					
					}
					level = LEVEL_P2PCONNECT_NOTIFY;
				}
				
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_CONNECT_REQ) {
				//NTY_PROTO_P2P_CONNECT_REQ

				U32 ack = *(U32*)(&buf[NTY_PROTO_ACKNUM_IDX]);
				friendId = *(C_DEVID*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]);	
				void *pTree = ntyRBTreeInstance();	

				FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, friendId);
				if (pFriend != NULL) {
					pFriend->sockfd = sockfd;
					pFriend->addr = addr.sin_addr.s_addr;
					pFriend->port = addr.sin_port;
					pFriend->isP2P = 0;
					pFriend->counter = 0;

					printf(" P2P client:%lld request connect\n", friendId);
				} else {
					FriendsInfo *friendInfo = (FriendsInfo*)malloc(sizeof(FriendsInfo));
					assert(pFriend);
					friendInfo->sockfd = sockfd;
					friendInfo->addr = addr.sin_addr.s_addr;
					friendInfo->port = addr.sin_port;
					friendInfo->isP2P = 0;
					friendInfo->counter = 0;
					ntyRBTreeInterfaceInsert(pTree, friendId, friendInfo);
				}

				sendP2PConnectAck(friendId, ack);	
				level = LEVEL_P2PDATAPACKET;				
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_CONNECT_ACK) {
				//NTY_PROTO_P2P_CONNECT_ACK
				
				//level = LEVEL_P2PDATAPACKET;
				friendId = *(C_DEVID*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]);	

				void *pTree = ntyRBTreeInstance();	
				FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, friendId);
				if (pFriend != NULL) {
					pFriend->isP2P = 1; 
				}				
				
				printf(" P2P client %lld connect Success\n", friendId);
				level = LEVEL_P2PDATAPACKET;
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_NOTIFY_REQ) {	
				//NTY_PROTO_P2P_NOTIFY_REQ

				void *pTree = ntyRBTreeInstance();
				
				friendId =  *(C_DEVID*)(&buf[NTY_PROTO_P2P_NOTIFY_DEVID_IDX]);
				U32 ack = *(U32*)(&buf[NTY_PROTO_P2P_NOTIFY_ACKNUM_IDX]);
				printf(" P2P Connect Notify: %lld\n", friendId);

				FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, friendId);
				if (pFriend != NULL) {
					pFriend->sockfd = sockfd;
					pFriend->addr = *(U32*)(&buf[NTY_PROTO_P2P_NOTIFY_IPADDR_IDX]);
					pFriend->port = *(U16*)(&buf[NTY_PROTO_P2P_NOTIFY_IPPORT_IDX]);
					pFriend->isP2P = 0;
					pFriend->counter = 0;
					printf(" P2P client:%lld\n", friendId);
					printf("%d.%d.%d.%d:%d\n", *(unsigned char*)(&pFriend->addr), *((unsigned char*)(&pFriend->addr)+1),	
						*((unsigned char*)(&pFriend->addr)+2), *((unsigned char*)(&pFriend->addr)+3),	 pFriend->port);
				} else {
					FriendsInfo *pFriend = (FriendsInfo*)malloc(sizeof(FriendsInfo));
					assert(pFriend);
					pFriend->sockfd = sockfd;
					pFriend->addr = *(U32*)(&buf[NTY_PROTO_P2P_NOTIFY_IPADDR_IDX]);
					pFriend->port = *(U16*)(&buf[NTY_PROTO_P2P_NOTIFY_IPPORT_IDX]);
					pFriend->isP2P = 0;
					pFriend->counter = 0;
					ntyRBTreeInterfaceInsert(pTree, friendId, pFriend);
				}
				//send ack to src devid
				sendP2PConnectNotifyAck(friendId, ack);
				//just now send p2p connect req
				//sendP2PConnectReq(pTree, friendId);
				level = LEVEL_P2PCONNECTFRIEND;
				
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_DATAPACKET_REQ) {
				//U16 cliCount = *(U16*)(&buf[NTY_PROTO_DATAPACKET_NOTIFY_CONTENT_COUNT_IDX]);
				U8 data[RECV_BUFFER_SIZE] = {0};//NTY_PROTO_DATAPACKET_NOTIFY_CONTENT_IDX
				U16 recByteCount = *(U16*)(&buf[NTY_PROTO_DATAPACKET_NOTIFY_CONTENT_COUNT_IDX]);
				C_DEVID friId = *(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]);
				U32 ack = *(U32*)(&buf[NTY_PROTO_ACKNUM_IDX]);

				memcpy(data, buf+NTY_PROTO_DATAPACKET_CONTENT_IDX, recByteCount);
				printf(" recv:%s\n", data);

				sendProxyDataPacketAck(friId, ack);
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_DATAPACKET_ACK) {
				printf(" send success\n");
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2PDATAPACKET_REQ) {
				U8 data[RECV_BUFFER_SIZE] = {0};
				U16 recByteCount = *(U16*)(&buf[NTY_PROTO_DATAPACKET_NOTIFY_CONTENT_COUNT_IDX]);
				C_DEVID friId = *(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]);
				U32 ack = *(U32*)(&buf[NTY_PROTO_ACKNUM_IDX]);
				
				void *pTree = ntyRBTreeInstance();
				FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, friendId);
				if (pFriend != NULL) {
					pFriend->isP2P = 1;
				}
				
				memcpy(data, buf+NTY_PROTO_DATAPACKET_CONTENT_IDX, recByteCount);
				printf(" P2P recv:%s\n", data);
				//sendP2PDataPacketReq(friId, data);
				sendP2PDataPacketAck(friId, ack);
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2PDATAPACKET_ACK) {
				printf(" P2P send success\n");
				level = LEVEL_P2PDATAPACKET;
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_HEARTBEAT_REQ) {
				C_DEVID fromId = *(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]);
				C_DEVID selfId = *(C_DEVID*)(&buf[NTY_PROTO_DEST_DEVID_IDX]);
				void *pTree = ntyRBTreeInstance();
				FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, fromId);
				if (pFriend != NULL) {
					pFriend->counter = 0;
					pFriend->isP2P = 1;
				}
				sendP2PHeartbeatAck(selfId, fromId);
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_HEARTBEAT_ACK) {
				C_DEVID fromId = *(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]);
				C_DEVID selfId = *(C_DEVID*)(&buf[NTY_PROTO_DEST_DEVID_IDX]);
				void *pTree = ntyRBTreeInstance();
				FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, fromId);
				if (pFriend != NULL) {
					pFriend->counter = 0;
					pFriend->isP2P = 1;
				}
			}
			usleep(1);
		}
	}
}

/*
 * Login Packet
 * VERSION					1			BYTE
 * MESSAGE TYPE				1			BYTE (req, ack)
 * TYPE					1			BYTE 
 * DEVID					8			BYTE
 * ACKNUM					4			BYTE (Network Module Set Value)
 * CRC 					4			BYTE (Network Module Set Value)
 * 
 * send to server addr
 */

void sendLoginPacket(void) {	
	int len, n;	
	U8 buf[RECV_BUFFER_SIZE] = {0};	

	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;	
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_REQ;	
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_LOGIN_REQ;
	*(C_DEVID*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]) = devid;	
	
	len = NTY_PROTO_LOGIN_REQ_CRC_IDX+sizeof(U32);				

	void *pNetwork = ntyNetworkInstance();
	n = ntySendFrame(pNetwork, &serveraddr, buf, len);
	
}

/*
 * P2P Connect Req
 * VERSION					1			 BYTE
 * MESSAGE TYPE			 	1			 BYTE (req, ack)
 * TYPE				 	1			 BYTE 
 * DEVID					8			 BYTE
 * ACKNUM					4			 BYTE (Network Module Set Value)
 * CRC 				 	4			 BYTE (Network Module Set Value)
 * 
 * send to friend addr
 *
 */

int sendP2PConnectReq(void* fTree, C_DEVID id) {
	//void *pRBTree = ntyRBTreeInstance();
	int len, n;	
	U8 buf[RECV_BUFFER_SIZE] = {0};
	struct sockaddr_in friendaddr;

	FriendsInfo *client = ntyRBTreeInterfaceSearch(fTree, id);
	if (client == NULL || (client->isP2P == 1)){//
		printf(" Client is not exist or P2P State : %d\n", client->isP2P);
		return -1;
	} //

	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_REQ;	
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2P_CONNECT_REQ;
	
	*(C_DEVID*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]) = (C_DEVID) devid;
	*(C_DEVID*)(&buf[NTY_PROTO_DEST_DEVID_IDX]) = id;
	len = NTY_PROTO_CRC_IDX+sizeof(U32);

	friendaddr.sin_family = AF_INET;
	friendaddr.sin_addr.s_addr = client->addr;				
	friendaddr.sin_port = client->port;
	
	printf("sendP2PConnectReq:%d.%d.%d.%d:%d\n", *(unsigned char*)(&friendaddr.sin_addr.s_addr), *((unsigned char*)(&friendaddr.sin_addr.s_addr)+1),													
				*((unsigned char*)(&friendaddr.sin_addr.s_addr)+2), *((unsigned char*)(&friendaddr.sin_addr.s_addr)+3),													
				friendaddr.sin_port);
	//msgAck |= SIGNAL_P2PCONNECT_REQ;
	if (client->addr == 0 || client->port == 0
		||client->addr == 0xFFFFFFFF || client->port == 0xFFFF) {
		client->isP2P = 0;
		return -1;
	}
	void *pNetwork = ntyNetworkInstance();
	n = ntySendFrame(pNetwork, &friendaddr, buf, len);
	//n = sendto(sockfd_local, buf, len, 0, (struct sockaddr *)&friendaddr, sizeof(friendaddr));

	return 0;
}

/*
 * P2P Connect Req
 * VERSION					1			 BYTE
 * MESSAGE TYPE			 	1			 BYTE (req, ack)
 * TYPE				 	1			 BYTE 
 * DEVID					8			 BYTE (self devid)
 * ACKNUM					4			 BYTE (Network Module Set Value)
 * CRC 				 	4			 BYTE (Network Module Set Value)
 * 
 * send to friend addr
 *
 */

void sendP2PConnectAck(C_DEVID friId, U32 ack) {
	int len, n;	
	U8 buf[RECV_BUFFER_SIZE] = {0};	

	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2P_CONNECT_ACK;
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_REQ;	
	
	*(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]) = (C_DEVID) devid;	
	*(U32*)(&buf[NTY_PROTO_ACKNUM_IDX]) = ack+1;
	*(C_DEVID*)(&buf[NTY_PROTO_DEST_DEVID_IDX]) = friId;
	
	len = NTY_PROTO_CRC_IDX+sizeof(U32);	
	struct sockaddr_in friendaddr;
	void *pTree = ntyRBTreeInstance();	
	FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, friId);

	friendaddr.sin_family = AF_INET;
	friendaddr.sin_addr.s_addr = pFriend->addr;				
	friendaddr.sin_port = pFriend->port;			

	void *pNetwork = ntyNetworkInstance();
	n = ntySendFrame(pNetwork, &friendaddr, buf, len);
	
}

/*
 * P2P DataPacket Req
 * VERSION					1			 BYTE
 * MESSAGE TYPE			 	1			 BYTE (req, ack)
 * TYPE				 	1			 BYTE 
 * DEVID					8			 BYTE (self devid)
 * ACKNUM					4			 BYTE (Network Module Set Value)
 * DEST DEVID				8			 BYTE (friend devid)
 * CONTENT COUNT				2			 BYTE 
 * CONTENT					*(CONTENT COUNT)	 BYTE 
 * CRC 				 	4			 BYTE (Network Module Set Value)
 * 
 * send to friend addr
 * 
 */
void sendP2PDataPacketReq(C_DEVID friId, U8 *buf, int length) {
	int n;

	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_REQ;	
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2PDATAPACKET_REQ;
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEVID_IDX]) = (C_DEVID) devid;
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEST_DEVID_IDX]) = friId;
	
	*(U16*)(&buf[NTY_PROTO_DATAPACKET_CONTENT_COUNT_IDX]) = (U16)length;
	length += NTY_PROTO_DATAPACKET_CONTENT_IDX;
	length += sizeof(U32);

	struct sockaddr_in friendaddr;
	void *pTree = ntyRBTreeInstance();	
	FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, friId);

	friendaddr.sin_family = AF_INET;
	friendaddr.sin_addr.s_addr = pFriend->addr;				
	friendaddr.sin_port = pFriend->port;

	void *pNetwork = ntyNetworkInstance();
	n = ntySendFrame(pNetwork, &friendaddr, buf, length);

}

/*
 * P2P DataPacket Ack
 * VERSION					1			 BYTE
 * MESSAGE TYPE			 	1			 BYTE (req, ack)
 * TYPE				 	1			 BYTE 
 * DEVID					8			 BYTE (self devid)
 * ACKNUM					4			 BYTE (Network Module Set Value)
 * DEST DEVID				8			 BYTE (friend devid)
 * CRC 				 	4			 BYTE (Network Module Set Value)
 * 
 * send to server addr, proxy to send one client
 * 
 */

void sendP2PDataPacketAck(C_DEVID friId, U32 ack) {
	int len, n;	
	U8 buf[RECV_BUFFER_SIZE] = {0}; 
	
	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_ACK; 
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2PDATAPACKET_ACK;

	*(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]) = (C_DEVID) devid;		
	*(U32*)(&buf[NTY_PROTO_ACKNUM_IDX]) = ack+1;
	*(C_DEVID*)(&buf[NTY_PROTO_DEST_DEVID_IDX]) = friId;
	
	len = NTY_PROTO_CRC_IDX+sizeof(U32);				

	struct sockaddr_in friendaddr;
	void *pTree = ntyRBTreeInstance();	
	FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, friId);

	friendaddr.sin_family = AF_INET;
	friendaddr.sin_addr.s_addr = pFriend->addr;				
	friendaddr.sin_port = pFriend->port;


	void *pNetwork = ntyNetworkInstance();
	n = ntySendFrame(pNetwork, &friendaddr, buf, len);
	
}


/*
 * Server Proxy Data Transport
 * VERSION					1			 BYTE
 * MESSAGE TYPE			 	1			 BYTE (req, ack)
 * TYPE				 	1			 BYTE 
 * DEVID					8			 BYTE (self devid)
 * ACKNUM					4			 BYTE (Network Module Set Value)
 * DEST DEVID				8			 BYTE (friend devid)
 * CONTENT COUNT				2			 BYTE 
 * CONTENT					*(CONTENT COUNT)	 BYTE 
 * CRC 				 	4			 BYTE (Network Module Set Value)
 * 
 * send to server addr, proxy to send one client
 * 
 */

void sendProxyDataPacketReq(C_DEVID friId, U8 *buf, int length) {
	int n = 0;

	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_REQ;	
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_DATAPACKET_REQ;
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEVID_IDX]) = (C_DEVID) devid;
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEST_DEVID_IDX]) = friId;
	
	*(U16*)(&buf[NTY_PROTO_DATAPACKET_CONTENT_COUNT_IDX]) = (U16)length;
	length += NTY_PROTO_DATAPACKET_CONTENT_IDX;
	length += sizeof(U32);

	void *pNetwork = ntyNetworkInstance();
	n = ntySendFrame(pNetwork, &serveraddr, buf, length);
	
}

/*
 * Server Proxy Data Transport
 * VERSION					1			 BYTE
 * MESSAGE TYPE			 	1			 BYTE (req, ack)
 * TYPE				 	1			 BYTE 
 * DEVID					8			 BYTE (self devid)
 * ACKNUM					4			 BYTE (Network Module Set Value)
 * DEST DEVID				8			 BYTE (friend devid)
 * CRC 				 	4			 BYTE (Network Module Set Value)
 * 
 * send to server addr, proxy to send one client
 * 
 */

void sendProxyDataPacketAck(C_DEVID friId, U32 ack) {
	int len, n;	
	U8 buf[RECV_BUFFER_SIZE] = {0}; 
	
	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_DATAPACKET_ACK;
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_ACK; 

	*(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]) = (C_DEVID) devid;		
	*(U32*)(&buf[NTY_PROTO_ACKNUM_IDX]) = ack+1;
	*(C_DEVID*)(&buf[NTY_PROTO_DEST_DEVID_IDX]) = friId;
	
	len = NTY_PROTO_CRC_IDX+sizeof(U32);				

	void *pNetwork = ntyNetworkInstance();
	n = ntySendFrame(pNetwork, &serveraddr, buf, len);
	
}

void sendP2PConnectNotify(C_DEVID fromId, C_DEVID toId) {
	U8 notify[NTY_LOGIN_ACK_LENGTH] = {0};
	int len = 0, n;
	
	notify[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	notify[NTY_PROTO_MESSAGE_TYPE] = (U8)MSG_REQ;
	notify[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2P_NOTIFY_REQ;

	
	*(C_DEVID*)(&notify[NTY_PROTO_P2P_NOTIFY_DEVID_IDX]) = fromId;
	*(C_DEVID*)(&notify[NTY_PROTO_P2P_NOTIFY_DEST_DEVID_IDX]) = toId;
	
	len = NTY_PROTO_P2P_NOTIFY_CRC_IDX + sizeof(U32);

	printf("send P2P Connect Notify\n");
	void *pNetwork = ntyNetworkInstance();
	n = ntySendFrame(pNetwork, &serveraddr, notify, len);
}


void sendP2PConnectNotifyAck(C_DEVID friId, U32 ack) {
	int len, n;	
	U8 buf[RECV_BUFFER_SIZE] = {0}; 
	
	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2P_NOTIFY_ACK;
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_ACK; 

	*(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]) = (C_DEVID) devid;		
	*(U32*)(&buf[NTY_PROTO_ACKNUM_IDX]) = ack+1;
	*(C_DEVID*)(&buf[NTY_PROTO_DEST_DEVID_IDX]) = friId;
	
	len = NTY_PROTO_CRC_IDX+sizeof(U32);				

	void *pNetwork = ntyNetworkInstance();
	n = ntySendFrame(pNetwork, &serveraddr, buf, len);
	
}


void *sendThread(void *arg) {
	void *pNetwork = ntyNetworkInstance();
	//U8 buf[RECV_BUFFER_SIZE];
	int times = 0;

	printf(" sendThread running\n");
	while (1) {
		//bzero(buf, RECV_BUFFER_SIZE);
		//buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
		//printf(" level : %d\n", level);
		if (level == LEVEL_LOGIN) {
			sendLoginPacket();
			//level = LEVEL_DEFAULT;
			//usleep(500*1000);
		} else if (level == LEVEL_P2PCONNECT_NOTIFY) {
			// 
			void *pTree = ntyRBTreeInstance();
			//printf("LEVEL_P2PCONNECT_NOTIFY\n");
			ntyFriendsTreeTraversalNotify(pTree, devid, sendP2PConnectNotify);
			level = LEVEL_DEFAULT;
			//usleep(500*1000);
		} else if (level == LEVEL_P2PCONNECTFRIEND) { 
			//
			//printf("LEVEL_P2PCONNECTFRIEND times : %d, friendId:%lld\n", times, friendId);
			if (times++ < 3) {
				void *pTree = ntyRBTreeInstance();
				sendP2PConnectReq(pTree, friendId);
			} else {
				times = 0;
				level = LEVEL_DATAPACKET;
			}
			sleep(1); 
			continue;
		} else if (level == LEVEL_P2PCONNECT) {
			//
			//printf("LEVEL_P2PCONNECT times : %d, friendId:%lld\n", times, friendId);
			if (times ++ < 3) {
				void *pTree = ntyRBTreeInstance();
				ntyFriendsTreeTraversal(pTree, sendP2PConnectReq);			
			} else {
				times = 0;
				level = LEVEL_DATAPACKET;
			}
			sleep(1);
			continue;
		} else {
			break;
		}
		sleep(HEARTBEAT_TIMEOUT);	
		times = 0; 
	}
}

void forkThread(void) {
	int err;
	ThreadArg threadArg = {0};
	pthread_t recvThread_id, sendThread_id, heartbeatThread_id, p2pHeartbeatThread_id;
	
	//sockfd_local = sockfd;
	threadArg.sockfd = 0;
	threadArg.devid = devid;

	//setup sendThread
	err = pthread_create(&sendThread_id, NULL, sendThread, &threadArg); 
	if (err != 0) { 	
		printf(" can't create thread:%s\n", strerror(err)); 	
		exit(0);	
	}	
	usleep(1);

	//setup recvThread
	err = pthread_create(&recvThread_id, NULL, recvThread, &threadArg); 
	if (err != 0) { 	
		printf(" can't create thread:%s\n", strerror(err)); 	
		exit(0);	
	}	
	usleep(1);
	
	//printf(" Setup heartbeat Thread\n");	
	err = pthread_create(&heartbeatThread_id, NULL, heartbeatThread, &threadArg);				
	if (err != 0) { 				
		printf(" can't create thread:%s\n", strerror(err)); 
		exit(0);				
	}
	usleep(1);
#if 0	
	//printf(" Setup p2p heartbeat Thread\n");	
	err = pthread_create(&p2pHeartbeatThread_id, NULL, heartbeatP2PThread, &threadArg); 			
	if (err != 0) { 				
		printf(" can't create thread:%s\n", strerror(err)); 
		exit(0);				
	}
	usleep(1);
#endif
}

#if 0

int main () {
	
#if 1
	int sockfd, n;
	struct sockaddr_in addr;	
	struct hostent *server;
	
	char *hostname = SERVER_NAME;
	portno = SERVER_PORT;
	//int devid = 0;
	U8 buf[RECV_BUFFER_SIZE];

#if 0	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		error(" ERROR opening socket");
	}
#else
	void *pNetwork = ntyNetworkInstance();
#endif
	
	server = gethostbyname(hostname);    
	if (server == NULL) {        
		fprintf(stderr,"ERROR, no such host as %s\n", hostname);  
		exit(0);    
	}
	
#if 1        
	printf(" Press DevId <1 or 2>: ");    
	n = scanf("%lld", &devid);
#else	
	srand(time(NULL));  	
	devid = rand() % 5000;
#endif

	//ntyInitTimer();
	ntyGenCrcTable();
	/* build the server's Internet address */    
	bzero((char *) &serveraddr, sizeof(serveraddr));    
	serveraddr.sin_family = AF_INET;    
	bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);    
	serveraddr.sin_port = htons(portno);

	forkThread();
	
	while (1) {
		bzero(buf, RECV_BUFFER_SIZE);
		buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
		if (level == LEVEL_P2PDATAPACKET) {
			U8 *tempBuf;

			tempBuf = &buf[NTY_PROTO_DATAPACKET_CONTENT_IDX];
			printf("P2P client --> Please enter msg: ");	    	
			char *ptr = fgets(tempBuf, RECV_BUFFER_SIZE-NTY_PROTO_DATAPACKET_CONTENT_IDX, stdin);	
			int len = strlen(tempBuf);
#if 1
			if (tempId == 0) {
				void *pTree = ntyRBTreeInstance();
				C_DEVID friId = ntyFriendsTreeGetFristNodeKey(pTree);
				tempId = friId;
			}
#endif
			if (level != LEVEL_P2PDATAPACKET) continue;
			sendP2PDataPacketReq(tempId, buf, len);
		} else if (level == LEVEL_DATAPACKET) {
			U8 *tempBuf;

			tempBuf = &buf[NTY_PROTO_DATAPACKET_CONTENT_IDX];
			char *ptr = fgets(tempBuf, RECV_BUFFER_SIZE-NTY_PROTO_DATAPACKET_CONTENT_IDX, stdin);
			int len = strlen(tempBuf);
#if 1		
			if (tempId == 0) {
				void *pTree = ntyRBTreeInstance();
				C_DEVID friId = ntyFriendsTreeGetFristNodeKey(pTree);
				tempId = friId;
			}
			printf("Proxy Please send msg: %lld\n", tempId);			
#endif
			if (level != LEVEL_DATAPACKET) continue;
			sendProxyDataPacketReq(tempId, buf, len);
		}
		usleep(1);
	}
#endif	
	return 0;
}
#endif

#endif
