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

#include <pthread.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>
#include <pthread.h>

#include "NattyProtoClient.h"
#include "NattyTimer.h"


static void ntySetupHeartBeatThread(void* self);
static void ntySetupRecvProcThread(void *self);
static void ntySendLogin(void *self);
static void ntySendLogout(void *self);
static void ntySendTimeout(int len);
static void* ntyRecvProc(void *arg);


#if 1 //

typedef void* (*RECV_CALLBACK)(void *arg);

typedef enum {
	STATUS_NETWORK_LOGIN,
	STATUS_NETWORK_PROXYDATA,
	STATUS_NETWORK_LOGOUT,
} StatusNetwork;


typedef struct _NATTYPROTOCOL {
	const void *_;
	C_DEVID devid; //App Or Device Id
	U8 level;
	U8 recvBuffer[RECV_BUFFER_SIZE];
	U16 recvLen;
	PROXY_CALLBACK onProxyCallback; //just for java
	RECV_CALLBACK onRecvCallback; //recv
	PROXY_CALLBACK onProxyFailed; //send data failed
	PROXY_CALLBACK onProxySuccess; //send data success
	U8 heartbeartRun;
	U8 p2pHeartbeatRun;
	struct sockaddr_in serveraddr;
} NattyProto;

typedef struct _NATTYPROTO_OPERA {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	void* (*heartbeat)(void *_self); //start 
	void (*login)(void *_self); //argument is optional
	void (*logout)(void *_self); //argument is optional
	void (*proxyReq)(void *_self, C_DEVID toId, U8 *buf, int length);
	void (*proxyAck)(void *_self, C_DEVID friId, U32 ack);
#if (NEY_PROTO_VERSION > 'A')
	int* (*p2pconnectReq)(void *_self, void* fTree, C_DEVID id); //for p2p
	int* (*p2pconnectAck)(void *_self, void* fTree, C_DEVID id); //for p2p
	void (*p2pdataReq)(void *_self, C_DEVID toId, U8 *buf, int length);
	void (*p2pdataAck)(void *_self, C_DEVID toId, U8 *buf, int length);
	void (*p2pheartbeatReq)(void* _self);
	void (*p2pheartbeatAck)(void *_self, C_DEVID toId, U8 *buf, int length);
#endif
} NattyProtoOpera;



#endif




void* ntyProtoClientCtor(void *_self, va_list *params) {
	NattyProto *proto = _self;
	struct hostent *server = NULL;

	proto->onRecvCallback = ntyRecvProc;
	proto->level = STATUS_NETWORK_LOGIN;
	proto->p2pHeartbeatRun = 0;
	proto->heartbeartRun = 0;
	proto->recvLen = 0;
	proto->devid = 0;

#if 1 //server addr init
	server = gethostbyname(SERVER_NAME);    
	if (server == NULL) {        
		fprintf(stderr,"ERROR, no such host as %s\n", SERVER_NAME);  
		//exit(0);   
		bzero((char *) &proto->serveraddr, sizeof(proto->serveraddr)); 
		return proto;
	}
	
	bzero((char *) &proto->serveraddr, sizeof(proto->serveraddr));    
	proto->serveraddr.sin_family = AF_INET;    
	bcopy((char *)server->h_addr, (char *)&proto->serveraddr.sin_addr.s_addr, server->h_length);    
	proto->serveraddr.sin_port = htons(SERVER_PORT);

	ntyGenCrcTable();

#if 1 //set network callback
	void *pNetwork = ntyNetworkInstance();
	((Network*)pNetwork)->onDataLost = ntySendTimeout;
#endif

#endif

	return proto;
}

void* ntyProtoClientDtor(void *_self) {
	NattyProto *proto = _self;

	proto->onRecvCallback = NULL;
	proto->level = STATUS_NETWORK_LOGOUT;
	proto->p2pHeartbeatRun = 0;
	proto->heartbeartRun = 0;
	proto->recvLen = 0;

#if 1 //should send logout packet to server
#endif

	return proto;
}

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

void* ntyProtoClientHeartBeat(void *_self) {
	NattyProto *proto = _self;

	int len, n;	
	U8 buf[NTY_LOGIN_ACK_LENGTH] = {0};	

	printf(" heartbeatThread running\n");
	if (proto->heartbeartRun == 1) {		
		proto->heartbeartRun = 1;		
		return NULL;	
	}	
	proto->heartbeartRun = 1;
	void *pNetwork = ntyNetworkInstance();

	while (1) {		
		bzero(buf, NTY_LOGIN_ACK_LENGTH);
		sleep(HEARTBEAT_TIMEOUT);	
		if (proto->devid == 0) continue; //set devid
		
		buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;	
		buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_REQ;	
		buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_HEARTBEAT_REQ;		
		*(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]) = proto->devid;
		
		len = NTY_PROTO_LOGIN_REQ_CRC_IDX+sizeof(U32);
		
		n = ntySendFrame(pNetwork, &proto->serveraddr, buf, len);
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
void ntyProtoClientLogin(void *_self) {
	NattyProto *proto = _self;
	int len, n;	
	U8 buf[RECV_BUFFER_SIZE] = {0};	

	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;	
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_REQ;	
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_LOGIN_REQ;
	*(C_DEVID*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]) = proto->devid;	
	
	len = NTY_PROTO_LOGIN_REQ_CRC_IDX+sizeof(U32);				

	printf(" ntyProtoClientLogin %d\n", __LINE__);
	void *pNetwork = ntyNetworkInstance();
	n = ntySendFrame(pNetwork, &proto->serveraddr, buf, len);
}

void ntyProtoClientLogout(void *_self) {
	NattyProto *proto = _self;
	int len, n;	
	U8 buf[RECV_BUFFER_SIZE] = {0};	

	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;	
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_REQ;	
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_LOGOUT_REQ;
	*(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]) = proto->devid;	
	
	len = NTY_PROTO_LOGIN_REQ_CRC_IDX+sizeof(U32);				

	void *pNetwork = ntyNetworkInstance();
	n = ntySendFrame(pNetwork, &proto->serveraddr, buf, len);
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

void ntyProtoClientProxyReq(void *_self, C_DEVID toId, U8 *buf, int length) {
	int n = 0;
	NattyProto *proto = _self;

	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_REQ;	
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_DATAPACKET_REQ;
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEVID_IDX]) = (C_DEVID) proto->devid;
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEST_DEVID_IDX]) = toId;
	
	*(U16*)(&buf[NTY_PROTO_DATAPACKET_CONTENT_COUNT_IDX]) = (U16)length;
	length += NTY_PROTO_DATAPACKET_CONTENT_IDX;
	length += sizeof(U32);

	void *pNetwork = ntyNetworkInstance();
	n = ntySendFrame(pNetwork, &proto->serveraddr, buf, length);
	
}

void ntyProtoClientProxyAck(void *_self, C_DEVID toId, U32 ack) {
	int len, n;	
	NattyProto *proto = _self;
	U8 buf[RECV_BUFFER_SIZE] = {0}; 
	
	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_DATAPACKET_ACK;
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_ACK; 

	*(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]) = (C_DEVID) proto->devid;		
	*(U32*)(&buf[NTY_PROTO_ACKNUM_IDX]) = ack+1;
	*(C_DEVID*)(&buf[NTY_PROTO_DEST_DEVID_IDX]) = toId;
	
	len = NTY_PROTO_CRC_IDX+sizeof(U32);				

	void *pNetwork = ntyNetworkInstance();
	n = ntySendFrame(pNetwork, &proto->serveraddr, buf, len);
}


static const NattyProtoOpera ntyProtoOpera = {
	sizeof(NattyProto),
	ntyProtoClientCtor,
	ntyProtoClientDtor,
	ntyProtoClientHeartBeat,
	ntyProtoClientLogin,
	ntyProtoClientLogout,
	ntyProtoClientProxyReq,
	ntyProtoClientProxyAck,
#if (NEY_PROTO_VERSION > 'A')
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
#endif	
};

const void *pNattyProtoOpera = &ntyProtoOpera;

static void *pProtoOpera = NULL;

void *ntyProtoInstance(void) {
	if (pProtoOpera == NULL) {
		printf("ntyProtoInstance\n");
		pProtoOpera = New(pNattyProtoOpera);
	}
	return pProtoOpera;
}

void ntyProtoRelease(void *self) {
	return Delete(self);
}

static void ntySetupHeartBeatThread(void* self) {
	NattyProtoOpera * const * protoOpera = self;
	int err;
	pthread_t heartbeatThread_id;

	if (self && (*protoOpera) && (*protoOpera)->heartbeat) {		
		err = pthread_create(&heartbeatThread_id, NULL, (*protoOpera)->heartbeat, self);				
		if (err != 0) { 				
			printf(" can't create thread:%s\n", strerror(err)); 
			exit(0);				
		}
	}
}

static void ntySetupRecvProcThread(void *self) {
	//NattyProtoOpera * const * protoOpera = self;
	NattyProto *proto = self;
	int err;
	pthread_t recvThread_id;

	if (self && proto && proto->onRecvCallback) {		
		err = pthread_create(&recvThread_id, NULL, proto->onRecvCallback, self);				
		if (err != 0) { 				
			printf(" can't create thread:%s\n", strerror(err)); 
			exit(0);				
		}
	}
}

static void ntySendLogin(void *self) {
	NattyProtoOpera * const * protoOpera = self;

	printf(" ntySendLogin %d\n", __LINE__);
	if (self && (*protoOpera) && (*protoOpera)->login) {
		return (*protoOpera)->login(self);
	}
}

static void ntySendLogout(void *self) {
	NattyProtoOpera * const * protoOpera = self;

	if (self && (*protoOpera) && (*protoOpera)->logout) {
		return (*protoOpera)->logout(self);
	}
}

int ntySendDataPacket(C_DEVID toId, U8 *data, int length) {
	int n = 0;
	void *self = ntyProtoInstance();
	NattyProtoOpera * const *protoOpera = self;
	NattyProto *proto = self;
	
	U8 buf[NTY_PROXYDATA_PACKET_LENGTH] = {0};
	
#if 0
	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_REQ;	
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_DATAPACKET_REQ;
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEVID_IDX]) = (C_DEVID) proto->devid;
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEST_DEVID_IDX]) = toId;
	
	*(U16*)(&buf[NTY_PROTO_DATAPACKET_CONTENT_COUNT_IDX]) = (U16)length;
	length += NTY_PROTO_DATAPACKET_CONTENT_IDX;
	length += sizeof(U32);
	void *pNetwork = ntyNetworkInstance();
	n = ntySendFrame(pNetwork, &proto->serveraddr, buf, length);
#else
	U8 *tempBuf = &buf[NTY_PROTO_DATAPACKET_CONTENT_IDX];
	memcpy(tempBuf, data, length);
	printf(" toId : %lld \n", toId);
	if (proto && (*protoOpera) && (*protoOpera)->proxyReq) {
		(*protoOpera)->proxyReq(proto, toId, buf, length);
		return 0;
	}
	return -1;
#endif
	
}

int ntySendMassDataPacket(U8 *data, int length) {	
	void *pTree = ntyRBTreeInstance();
	ntyRBTreeOperaMass(pTree, ntySendDataPacket, data, length);

	return 0;
}

void ntySetSendSuccessCallback(PROXY_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	proto->onProxySuccess = cb;
}

void ntySetSendFailedCallback(PROXY_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->onProxyFailed = cb;
	}
}

void ntySetProxyCallback(PROXY_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->onProxyCallback = cb;
	}
}

void ntySetDevId(C_DEVID id) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->devid = id;
		printf("ntySendLogin \n");
		
		ntySendLogin(proto);
		ntySetupHeartBeatThread(proto); //setup heart proc
		ntySetupRecvProcThread(proto); //setup recv proc
	}
}

U8* ntyGetRecvBuffer(void) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->recvBuffer[NTY_PROTO_DATAPACKET_CONTENT_IDX+proto->recvLen] = 0x0;
		return proto->recvBuffer+NTY_PROTO_DATAPACKET_CONTENT_IDX;
	}
	return NULL;
}

static void ntySendTimeout(int len) {
	NattyProto* proto = ntyProtoInstance();
	if (proto && proto->onProxyFailed) {
		proto->onProxyFailed(len);
	}

	void* pTimer = ntyNetworkTimerInstance();
	ntyStopTimer(pTimer);
}

static void* ntyRecvProc(void *arg) {
	struct sockaddr_in addr;
	int clientLen = sizeof(struct sockaddr_in);
	NattyProto *proto = arg;
	NattyProtoOpera * const *protoOpera = arg;
	U8 *buf = proto->recvBuffer;

	int ret;
	void *pNetwork = ntyNetworkInstance();

	struct pollfd fds;
	fds.fd = ntyGetSocket(pNetwork);
	fds.events = POLLIN;

	printf(" ntyRecvProc %d\n", fds.fd);
	while (1) {
		ret = poll(&fds, 1, 5);
		if (ret) {
			bzero(buf, RECV_BUFFER_SIZE);
			proto->recvLen = ntyRecvFrame(pNetwork, buf, RECV_BUFFER_SIZE, &addr);

			printf("%d.%d.%d.%d:%d, length:%d --> %x, id:%lld\n", *(unsigned char*)(&addr.sin_addr.s_addr), *((unsigned char*)(&addr.sin_addr.s_addr)+1),													
				*((unsigned char*)(&addr.sin_addr.s_addr)+2), *((unsigned char*)(&addr.sin_addr.s_addr)+3),													
				addr.sin_port, proto->recvLen, buf[NTY_PROTO_TYPE_IDX], *(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]));	
			
			
			if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_LOGIN_ACK) {
				int i = 0;
				
				int count = *(U16*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDS_COUNT_IDX]);
				void *pTree = ntyRBTreeInstance();

				for (i = 0;i < count;i ++) {
					C_DEVID friendId = *(C_DEVID*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_DEVID_IDX(i)]);

					FriendsInfo *friendInfo = ntyRBTreeInterfaceSearch(pTree, friendId);
					if (NULL == friendInfo) {
						FriendsInfo *pFriend = (FriendsInfo*)malloc(sizeof(FriendsInfo));
						assert(pFriend);
						pFriend->sockfd = ntyGetSocket(pNetwork);;
						pFriend->isP2P = 0;
						pFriend->counter = 0;
						pFriend->addr = *(U32*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_ADDR_IDX(i)]);
						pFriend->port = *(U16*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_PORT_IDX(i)]);
						ntyRBTreeInterfaceInsert(pTree, friendId, pFriend);
					} else {
						friendInfo->sockfd = ntyGetSocket(pNetwork);;
						friendInfo->isP2P = 0;
						friendInfo->counter = 0;
						friendInfo->addr = *(U32*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_ADDR_IDX(i)]);
						friendInfo->port = *(U16*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_PORT_IDX(i)]);
					}					
				}
		
				proto->level = LEVEL_DATAPACKET;
				//printf("NTY_PROTO_LOGIN_ACK\n");
				
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_DATAPACKET_REQ) {
				//U16 cliCount = *(U16*)(&buf[NTY_PROTO_DATAPACKET_NOTIFY_CONTENT_COUNT_IDX]);
				U8 data[RECV_BUFFER_SIZE] = {0};//NTY_PROTO_DATAPACKET_NOTIFY_CONTENT_IDX
				U16 recByteCount = *(U16*)(&buf[NTY_PROTO_DATAPACKET_NOTIFY_CONTENT_COUNT_IDX]);
				C_DEVID friId = *(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]);
				U32 ack = *(U32*)(&buf[NTY_PROTO_ACKNUM_IDX]);

				memcpy(data, buf+NTY_PROTO_DATAPACKET_CONTENT_IDX, recByteCount);
				printf(" recv:%s\n", data);
				
				//sendProxyDataPacketAck(friId, ack);
				if (arg && (*protoOpera) && (*protoOpera)->proxyAck) {
					(*protoOpera)->proxyAck(proto, friId, ack);
				}

				if (proto->onProxyCallback) {
					proto->onProxyCallback(proto->recvLen-NTY_PROTO_DATAPACKET_CONTENT_IDX-sizeof(U32));
				}
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_DATAPACKET_ACK) {
				printf(" send success\n");
				if (proto->onProxySuccess) {
					proto->onProxySuccess(0);
				}
			} 
		}
	}
}






