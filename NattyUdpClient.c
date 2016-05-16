/*
 *  Author : WangBoJing , email : 1989wangbojing@gmail.com
 * 
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of NALEX Inc. (C) 2016
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

#include "NattyUdpClient.h"



#define SERVER_NAME		"112.93.116.188" //"127.0.0.1"
#define SERVER_PORT		8888
#define RECV_BUFFER_SIZE	(1024+16)
#define SENT_TIMEOUT	3


static int count = 0;
static U32 msgAck = 0x0;
static int level = 0;
static C_DEVID devid = 0;
static C_DEVID friendId = 0;
static C_DEVID tempId = 0;
static U32 ackNum = 0;
static int sockfd_local = 0;

static int ntyNetworkResendFrame(void *_self);
void sendP2PConnectAck(C_DEVID friId, U32 ack);
void sendProxyDataPacketAck(C_DEVID friId, U32 ack);
void sendP2PDataPacketAck(C_DEVID friId, U32 ack);




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
	void* pTimer = ntyNetworkTimerInstance();
	ntyStopTimer(pTimer);
#if 1
	if (LEVEL_P2PCONNECT == level) {
		level = LEVEL_DATAPACKET;
	}
#endif
}

void ntyMessageOnAck(int signo) {
	//printf(" Get a Sigalarm, %d counts! \n", ++count);
	if (++count > SENT_TIMEOUT) {
		ntyMessageOnDataLost();
	} else {
		void *pNetwork = ntyNetworkInstance();
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
	
	ntyStartTimer(pTimer, network->onAck);	
	network->ackNum ++;
	
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
	n = recvfrom(network->sockfd, buf, CACHE_BUFFER_SIZE, 0, (struct sockaddr*)&addr, &clientLen);
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



struct sockaddr_in serveraddr;
int portno;
static U8 heartbeartRun = 0;

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

	if (heartbeartRun == 1) {		
		heartbeartRun = 1;		
		return NULL;	
	}	

	void *pNetwork = ntyNetworkInstance();

	while (1) {		
		bzero(buf, NTY_LOGIN_ACK_LENGTH);
		
		buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;	
		buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_REQ;	
		buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_HEARTBEAT_REQ;		
		*(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]) = devid;
		
		len = NTY_PROTO_LOGIN_REQ_CRC_IDX+sizeof(U32);
		
		n = ntySendFrame(pNetwork, &serveraddr, buf, len);
		sleep(20);	
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
	pthread_t heartbeatThread_id;

	void *pNetwork = ntyNetworkInstance();

	struct pollfd fds;
	fds.fd = ntyGetSocket(pNetwork);
	fds.events = POLLIN;


	while (1) {
		ret = poll(&fds, 1, 5);
		if (ret) {
			bzero(buf, RECV_BUFFER_SIZE);
			//n = recvfrom(sockfd, buf, RECV_BUFFER_SIZE, 0, (struct sockaddr *) &addr, &clientLen);
			n = ntyRecvFrame(pNetwork, buf, RECV_BUFFER_SIZE, &addr);
#if 0
			printf("%d.%d.%d.%d:%d size:%d --> %x\n", *(unsigned char*)(&addr.sin_addr.s_addr), *((unsigned char*)(&addr.sin_addr.s_addr)+1),	
				*((unsigned char*)(&addr.sin_addr.s_addr)+2), *((unsigned char*)(&addr.sin_addr.s_addr)+3),	 addr.sin_port,
				n, buf[NTY_PROTO_TYPE_IDX]);
#endif			
			if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_LOGIN_ACK) {
				int i = 0;
				//msgAck &= (~SIGNAL_LOGIN_ACK);
				level = LEVEL_P2PCONNECT;
				
				int count = *(U16*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDS_COUNT_IDX]);
				void *pTree = ntyRBTreeInstance();

				for (i = 0;i < count;i ++) {
					C_DEVID friendId = *(C_DEVID*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_DEVID_IDX(i)]);
					
					FriendsInfo *pFriend = (FriendsInfo*)malloc(sizeof(FriendsInfo));
					assert(pFriend);
					pFriend->sockfd = sockfd;
					pFriend->addr = *(U32*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_ADDR_IDX(i)]);
					pFriend->port = *(U16*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_PORT_IDX(i)]);
					ntyRBTreeInterfaceInsert(pTree, friendId, pFriend);
				}
			
				printf(" Setup heartbeat Thread\n");	
				err = pthread_create(&heartbeatThread_id, NULL, heartbeatThread, arg);				
				if (err != 0) {					
					printf(" can't create thread:%s\n", strerror(err));	
					exit(0);				
				}
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_CONNECT_REQ) {
				//msgAck &= (~SIGNAL_P2PCONNECT_ACK);
				//level = LEVEL_P2PCONNECT_ACK;

				U32 ack = *(U32*)(&buf[NTY_PROTO_ACKNUM_IDX]);
				friendId = *(C_DEVID*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]);	
				void *pTree = ntyRBTreeInstance();	

				FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, friendId);
				if (pFriend != NULL) {
					pFriend->isP2P = 1;
					pFriend->port = addr.sin_port;
					pFriend->addr = addr.sin_addr.s_addr;

					printf(" P2P client:%lld request connect\n", friendId);
					sendP2PConnectAck(friendId, ack);
					level = LEVEL_P2PDATAPACKET;
				}
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_CONNECT_ACK) {
				level = LEVEL_P2PDATAPACKET;
				friendId = *(C_DEVID*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]);	
				printf(" P2P client %lld connect Success\n", friendId);
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_NOTIFY_REQ) {	
#if 0
				clientaddr.sin_family = AF_INET;
				clientaddr.sin_addr.s_addr = *(U32*)(&buf[NTY_PROTO_P2P_NOTIFY_IPADDR_IDX]);		
				clientaddr.sin_port = *(U16*)(&buf[NTY_PROTO_P2P_NOTIFY_IPPORT_IDX]);	
				printf(" Notify %d.%d.%d.%d:%d\n", *(unsigned char*)(&clientaddr.sin_addr.s_addr), *((unsigned char*)(&clientaddr.sin_addr.s_addr)+1),	
					*((unsigned char*)(&clientaddr.sin_addr.s_addr)+2), *((unsigned char*)(&clientaddr.sin_addr.s_addr)+3),	
					clientaddr.sin_port);								
				level = 2;			
#else
				void *pTree = ntyRBTreeInstance();

				
				friendId =  *(C_DEVID*)(&buf[NTY_PROTO_P2P_NOTIFY_DEVID_IDX]);
				FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, friendId);
				if (pFriend != NULL) {
					pFriend->isP2P = 1;
					pFriend->port = *(U16*)(&buf[NTY_PROTO_P2P_NOTIFY_IPPORT_IDX]);
					pFriend->addr = *(U32*)(&buf[NTY_PROTO_P2P_NOTIFY_IPADDR_IDX]);

					printf(" P2P client:%lld request connect\n", friendId);
				}	
				level = LEVEL_P2PCONNECTFRIEND;
				//*(U32*)(&notify[NTY_PROTO_P2P_NOTIFY_IPADDR_IDX]) = pClient->addr.sin_addr.s_addr;
				//*(U16*)(&notify[NTY_PROTO_P2P_NOTIFY_IPPORT_IDX]) = pClient->addr.sin_port;
#endif
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

				memcpy(data, buf+NTY_PROTO_DATAPACKET_CONTENT_IDX, recByteCount);
				printf(" P2P recv:%s\n", data);
				//sendP2PDataPacketReq(friId, data);
				sendP2PDataPacketAck(friId, ack);
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2PDATAPACKET_ACK) {
				printf(" P2P send success\n");
			}

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
	if (client == NULL || client->isP2P) return ;

	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_REQ;	
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2P_CONNECT_REQ;
	*(C_DEVID*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]) = (C_DEVID) devid;
			
	len = NTY_PROTO_LOGIN_REQ_CRC_IDX+sizeof(U32);

	friendaddr.sin_family = AF_INET;
	friendaddr.sin_addr.s_addr = client->addr;				
	friendaddr.sin_port = client->port;
	
	printf("%d.%d.%d.%d:%d\n", *(unsigned char*)(&friendaddr.sin_addr.s_addr), *((unsigned char*)(&friendaddr.sin_addr.s_addr)+1),													
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
	len = NTY_PROTO_LOGIN_REQ_CRC_IDX+sizeof(U32);				

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
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEST_DEVID_IDX]) = friId;
	
	len = NTY_PROTO_LOGIN_REQ_CRC_IDX+sizeof(U32);				

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
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2PDATAPACKET_ACK;
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_ACK; 

	*(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]) = (C_DEVID) devid;		
	*(U32*)(&buf[NTY_PROTO_ACKNUM_IDX]) = ack+1;
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEST_DEVID_IDX]) = friId;
	
	len = NTY_PROTO_LOGIN_REQ_CRC_IDX+sizeof(U32);				

	void *pNetwork = ntyNetworkInstance();
	n = ntySendFrame(pNetwork, &serveraddr, buf, len);
	
}

int main () {
	
#if 1
	int sockfd, err;
	pthread_t recvThread_id, heartbeartThread_id;
	struct sockaddr_in addr;	
	struct hostent *server;
	
	char *hostname = SERVER_NAME;
	portno = SERVER_PORT;
	ThreadArg threadArg = {0};
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
	scanf("%lld", &devid);
#else	
	srand(time(NULL));  	
	devid = rand() % 5000;
#endif

	//sockfd_local = sockfd;
	threadArg.sockfd = 0;
	threadArg.devid = devid;
	err = pthread_create(&recvThread_id, NULL, recvThread, &threadArg);	
	if (err != 0) {		
		printf(" can't create thread:%s\n", strerror(err));		
		exit(0);	
	}

	//ntyInitTimer();
	ntyGenCrcTable();
	/* build the server's Internet address */    
	bzero((char *) &serveraddr, sizeof(serveraddr));    
	serveraddr.sin_family = AF_INET;    
	bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);    
	serveraddr.sin_port = htons(portno);

	while (1) {
		bzero(buf, RECV_BUFFER_SIZE);
		buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
		if (level == LEVEL_LOGIN) {
			sendLoginPacket();
		} else if (level == LEVEL_HEARTBEART) {

		} else if (level == LEVEL_P2PCONNECTFRIEND) { 
			void *pTree = ntyRBTreeInstance();
			sendP2PConnectReq(pTree, friendId);
			level = LEVEL_DATAPACKET;
		} else if (level == LEVEL_P2PCONNECT) {
			void *pTree = ntyRBTreeInstance();
			ntyFriendsTreeTraversal(pTree, sendP2PConnectReq);			
			level = LEVEL_DATAPACKET;
		} else if (level == LEVEL_P2PCONNECT_ACK) {
			//sendP2PConnectAck(sockfd, devid);
			//level = LEVEL_DATAPACKET;
		} else if (level == LEVEL_P2PDATAPACKET) {
			U8 *tempBuf;

			tempBuf = &buf[NTY_PROTO_DATAPACKET_CONTENT_IDX];
			printf("P2P client --> Please enter msg: ");	    	
			fgets(tempBuf, RECV_BUFFER_SIZE-NTY_PROTO_DATAPACKET_CONTENT_IDX, stdin);	
			int len = strlen(tempBuf);
#if 1
			if (tempId == 0) {
				void *pTree = ntyRBTreeInstance();
				C_DEVID friId = ntyFriendsTreeGetFristNodeKey(pTree);
				tempId = friId;
			}
#endif
			sendP2PDataPacketReq(tempId, buf, len);
		} else if (level == LEVEL_DATAPACKET) {
			U8 *tempBuf;

			tempBuf = &buf[NTY_PROTO_DATAPACKET_CONTENT_IDX];
			fgets(tempBuf, RECV_BUFFER_SIZE-NTY_PROTO_DATAPACKET_CONTENT_IDX, stdin);
			int len = strlen(tempBuf);
			printf("Proxy Please send %d msg: ", len);	
#if 1		
			if (tempId == 0) {
				void *pTree = ntyRBTreeInstance();
				C_DEVID friId = ntyFriendsTreeGetFristNodeKey(pTree);
				tempId = friId;
			}
#endif
			sendProxyDataPacketReq(tempId, buf, len);
		}

		sleep(2);
	}
#endif	
	return 0;
}


