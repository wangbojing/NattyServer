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


#include "NattyRBTree.h"
#include "NattyFilter.h"
#include "NattySession.h"
#include "NattyDaveMQ.h"
#include "NattyConfig.h"
#include "NattyHash.h"
#include "NattyNodeAgent.h"
#include "NattyDBOperator.h"
#include "NattyUtils.h"
#include "NattyMulticast.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <ev.h>

#define JEMALLOC_NO_DEMANGLE 1
#define JEMALLOC_NO_RENAME	 1
#include <jemalloc/jemalloc.h>


#if ENABLE_NATTY_TIME_STAMP
#include <pthread.h>
pthread_mutex_t time_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t loop_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif


static void *ntyClientListCtor(void *_self, va_list *params) {
	SingleList *self = _self;
	self->head = (Node*)calloc(1, sizeof(Node));
	self->head->next = NULL;
	self->count = 0;
	
	return self;
}

static void *ntyClientListDtor(void *_self) {
	SingleList *self = _self;
	Node **p = &self->head;

	while ((*p) != NULL) {
		Node *node = *p;
		*p = node->next;
		free(node);
	}
	return self;
}


static void ntyClientListInsert(void *_self, int id) {
	SingleList *self = _self;
	Node *node = (Node*)calloc(1, sizeof(Node));
	Node **p = (Node**)&self->head;
#if 0
	for (; (*p) != NULL; p = &(*p)->next) ;

	node->data = data;
	node->next = p;
	*p = node;
#else
	self->count ++; //client list count
	node->clientId = id;
	node->next = (*p)->next;
	(*p)->next = node;
#endif
}


static int ntyClientListRemove(void *_self, int id) {
	SingleList *self = _self;
	Node **p = (Node **)&self->head;

	while ((*p) != NULL) {
		Node *node = *p;
		if (node->clientId == id) {
			*p = node->next;
			self->count --; //client list count
			free(node);

			return 0;
		} else {
			p = &(*p)->next;
		}
	}
	return 1;
}

static C_DEVID* ntyClientListIterator(const void *_self) {
	const SingleList *self = _self;
	Node **p = &self->head->next;
	C_DEVID* pClientList = (C_DEVID*)malloc(self->count * sizeof(C_DEVID));

	for (; (*p) != NULL; p = &(*p)->next) {		
		*pClientList = (*p)->clientId;
	}
	return pClientList;
}

static void ntyClientListPrint(const void *_self) {
	const SingleList *self = _self;
	Node **p = &self->head->next;

	while ((*p) != NULL) {
		//print_fn((*p)->data);
		//ntylog(" %ld ", (*p)->clientId);
		p = &(*p)->next;
	}
	ntylog("\n");
}

static const List ntyClientList = {
	sizeof(SingleList),
	ntyClientListCtor,
	ntyClientListDtor,
	ntyClientListInsert,
	ntyClientListRemove,
	ntyClientListIterator,
	ntyClientListPrint,
};

const void *pNtyClientList = &ntyClientList;



void Insert(void *self, int Id) {
	List **pListOpera = self;
	if (self && (*pListOpera) && (*pListOpera)->insert) {
		(*pListOpera)->insert(self, Id);
	}
}

int Remove(void *self, int Id) {
	List **pListOpera = self;
	if (self && (*pListOpera) && (*pListOpera)->remove) {
		return (*pListOpera)->remove(self, Id);
	}
	return -1;
}

U64* Iterator(void *self) {
	List **pListOpera = self;
	if (self && (*pListOpera) && (*pListOpera)->iterator) {
		return (*pListOpera)->iterator(self);
	}
	return NULL;
}



void *ntyPacketCtor(void *_self, va_list *params) {
	Packet *self = _self;
	return self;
}


void *ntyPacketDtor(void *_self) {
	Packet *self = _self;
	self->succ = NULL;
	
	return self;
}

void ntyPacketSetSuccessor(void *_self, void *_succ) {
	Packet *self = _self;
	self->succ = _succ;
}

void* ntyPacketGetSuccessor(const void *_self) {
	const Packet *self = _self;
	return self->succ;
}

void ntyP2PNotifyClient(UdpClient *client, U8 *notify) {
	int length = 0;
	
	notify[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2P_NOTIFY_REQ;
	
	length = NTY_PROTO_P2P_NOTIFY_CRC_IDX;
	*(U32*)(&notify[length]) = ntyGenCrcValue(notify, length);
	length += sizeof(U32);

	ntySendBuffer(client, notify, length);

	return ;
}

static int ntyUpdateClientListIpAddr(UdpClient *client, C_DEVID key, U32 ackNum) {
	int i = 0;
	UdpClient *pClient = client;
	void *pRBTree = ntyRBTreeInstance();

	C_DEVID* friendsList = Iterator(pClient->friends);
	int count = ((SingleList*)pClient->friends)->count;
	for (i = 0;i < count;i ++) {
#if 1
		UdpClient *cv = ntyRBTreeInterfaceSearch(pRBTree, *(friendsList+i));
		if (cv != NULL) {
			
			U8 ackNotify[NTY_P2P_NOTIFY_ACK_LENGTH] = {0};
			*(C_DEVID*)(&ackNotify[NTY_PROTO_P2P_NOTIFY_DEVID_IDX]) = key;
			*(U32*)(&ackNotify[NTY_PROTO_P2P_NOTIFY_ACKNUM_IDX]) = ackNum;
			
			*(U32*)(&ackNotify[NTY_PROTO_P2P_NOTIFY_IPADDR_IDX]) = client->addr.sin_addr.s_addr;
			*(U16*)(&ackNotify[NTY_PROTO_P2P_NOTIFY_IPPORT_IDX]) = client->addr.sin_port;

			ntyP2PNotifyClient(cv, ackNotify);
		}
#else
		U8 ackNotify[NTY_P2P_NOTIFY_ACK_LENGTH] = {0};
		*(U32*)(&ackNotify[NTY_PROTO_P2P_NOTIFY_DEVID_IDX]) = key;
		*(U32*)(&ackNotify[NTY_PROTO_P2P_NOTIFY_ACKNUM_IDX]) = ackNum;

		ntyP2PNotifyClient(cv, ackNotify);
#endif
	}
	free(friendsList);
}


static void ntyClientFriendsList(UdpClient *client, U8 *ack) {
	int i = 0, length;
	void *pRBTree = ntyRBTreeInstance();
	UdpClient *pClient = client;

	C_DEVID* friendsList = Iterator(pClient->friends);
	int count = ((SingleList*)pClient->friends)->count;
	*(U16*)(&ack[NTY_PROTO_LOGIN_ACK_FRIENDS_COUNT_IDX]) = (U16)count;
	for (i = 0;i < count;i ++) {
		UdpClient *cv = ntyRBTreeInterfaceSearch(pRBTree, *(friendsList+i));
		if (cv != NULL) {			
			*(U32*)(&ack[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_ADDR_IDX(i)]) = (U32)(cv->addr.sin_addr.s_addr);
			*(U16*)(&ack[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_PORT_IDX(i)]) = (U16)(cv->addr.sin_port);
		}
		*(U16*)(&ack[NTY_PROTO_LOGIN_ACK_FRIENDS_COUNT_IDX]) = (U16)count;
		*(C_DEVID*)(&ack[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_DEVID_IDX(i)]) = *(friendsList+i);
				
	}
	free(friendsList);
	
	if (count == 0) {
		length = NTY_PROTO_LOGIN_ACK_FRIENDSLIST_START_IDX;
		*(U32*)(&ack[length]) = ntyGenCrcValue(ack, length+1);				
	} else {
		length = NTY_PROTO_LOGIN_ACK_CRC_IDX(count);
		*(U32*)(&ack[length]) = ntyGenCrcValue(ack, length+1);				
	}
	length = length + 1 + sizeof(U32);

	ntySendBuffer(client, ack, length);
}

extern void *ntyTcpServerGetMainloop(void);


static int ntyAddClientNodeToRBTree(unsigned char *buffer, int length, const void * obj) {
	const Client *client = obj;
	void *pRBTree = ntyRBTreeInstance();

	ntylog(" DevId :%lld\n", client->devId);
	if (client->devId == NATTY_NULL_DEVID) return -1;
	
	struct sockaddr_in addr;
	memcpy(&addr, &client->addr, sizeof(struct sockaddr_in));

	Client *selfNode = ntyRBTreeInterfaceSearch(pRBTree, client->devId);
	if (selfNode == NULL) {
		if (0 != ntyUpdateNodeToHashTable(&addr, client->devId, client->sockfd)) {
			ntylog(" ntyAddClientNodeToRBTree --> HashTable Node Id is Conflict\n");
			return -2;
		}
		
		Client *pClient = (Client*)malloc(sizeof(Client)); //new selfnode
		pClient->clientType = client->clientType;
		pClient->addr.sin_addr.s_addr = client->addr.sin_addr.s_addr;
		pClient->addr.sin_port = client->addr.sin_port;
		pClient->ackNum = 0x0;
		pClient->devId = client->devId;
		pClient->rLength = 0;
		pClient->recvBuffer = malloc(PACKET_BUFFER_SIZE);
		
		pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;
		memcpy(&pClient->buffer_mutex, &blank_mutex, sizeof(pClient->buffer_mutex));
#if ENABLE_NATTY_TIME_STAMP //TIME Stamp 	
		pClient->stamp = ntyTimeStampGenrator();
#endif
		pClient->sockfd = client->sockfd;
		if (client->clientType == PROTO_TYPE_TCP) {
			pClient->watcher = client->watcher;
		} else {
			pClient->watcher = NULL;
		}

		pClient->friends = ntyFriendsTreeInstance(); //new friends tree
#if ENABLE_CONNECTION_POOL
		if (buffer[NEY_PROTO_VERSION_IDX] == NTY_PROTO_APP_VERSION) { //App
			pClient->deviceType = DEVICE_TYPE_APP;
			if(-1 == ntyQueryWatchIDListSelectHandle(client->devId, pClient->friends)) {
				ntylog(" ntyQueryWatchIDListSelectHandle Failed \n");
			}
		} else if (buffer[NEY_PROTO_VERSION_IDX] == NTY_PROTO_DEVICE_VERSION) { //Device
			pClient->deviceType = DEVICE_TYPE_WATCH;
			if (-1 == ntyQueryAppIDListSelectHandle(client->devId, pClient->friends)) {
				ntylog(" ntyQueryAppIDListSelectHandle Failed \n");
			} else {
				ntyExecuteDeviceLoginUpdateHandle(client->devId); //SYNC MYSQL
			}
		} else {
			ntylog(" Protocol Version is Error : %c\n", buffer[NEY_PROTO_VERSION_IDX]);
			//free(pClient);
			//return ;
		}
#endif
		
		//insert rb-tree
		if (ntyRBTreeInterfaceInsert(pRBTree, pClient->devId, pClient)) {
			ntylog("Client is Exist or conflict Handle\n");
			ntyRBTreeRelease(pClient->friends);
			
			free(pClient);
			return -1;
		}
		ntylog(" New Client RBTREENODE --> %lld\n", pClient->devId);	
		
#if 1
		ntyBoardcastAllFriendsNotifyConnect(pClient->devId);
#endif
		return 1;
	} else if (selfNode != NULL && (!ntyClientCompare (client, selfNode))) {
		UdpClient *pClient = selfNode;//(UdpClient*)malloc(sizeof(UdpClient));
#if 1
		ntylog(" ntyAddClientNodeToRBTree --> client: %d.%d.%d.%d:%d --> cliValue: %d.%d.%d.%d:%d, ip have changed\n", 
			*(unsigned char*)(&client->addr.sin_addr.s_addr), *((unsigned char*)(&client->addr.sin_addr.s_addr)+1),													
			*((unsigned char*)(&client->addr.sin_addr.s_addr)+2), *((unsigned char*)(&client->addr.sin_addr.s_addr)+3),													
			client->addr.sin_port, *(unsigned char*)(&selfNode->addr.sin_addr.s_addr), *((unsigned char*)(&selfNode->addr.sin_addr.s_addr)+1),													
			*((unsigned char*)(&selfNode->addr.sin_addr.s_addr)+2), *((unsigned char*)(&selfNode->addr.sin_addr.s_addr)+3),													
			selfNode->addr.sin_port);
#endif

		int ret = ntyUpdateNodeToHashTable(&addr, client->devId, client->sockfd);
		if (ret != 0) { 			
			ntylog(" ntyAddClientNodeToRBTree --> HashTable Node Id is Conflict\n");
			return -2;
		}
		
		struct ev_loop *loop = ntyTcpServerGetMainloop();
#if 1 //Release Before Node
		if(0 == ntyReleaseClientNodeHashTable(&pClient->addr)) { //release before hash node
			ntylog(" ntyAddClientNodeToRBTree --> Ip have changed and release hash table\n");
		}
		if(0 == ntyReleaseClientNodeSocket(loop, pClient->watcher, pClient->watcher->fd)) {
			ntylog(" ntyAddClientNodeToRBTree --> Ip have changed and release client socket\n");
		}
#endif

		pClient->clientType = client->clientType;
		pClient->addr.sin_addr.s_addr = client->addr.sin_addr.s_addr;
		pClient->addr.sin_port = client->addr.sin_port;
		pClient->ackNum = 0x0;
		pClient->rLength = 0;
#if 0		
		if (pClient->recvBuffer != NULL) {
			free(pClient->recvBuffer);
			
			pClient->rLength = 0;
			pClient->recvBuffer = NULL;
		}
#endif
#if ENABLE_NATTY_TIME_STAMP //TIME Stamp 	
		pClient->stamp = ntyTimeStampGenrator();
#endif
		pClient->sockfd = client->sockfd;
		if (client->clientType == PROTO_TYPE_TCP) {
			pClient->watcher = client->watcher;
		} else {
			pClient->watcher = NULL;
		}
		
		if (pClient->devId != client->devId) {
			ntylog(" pClient->devId != client->devId \n");
			//pClient->devId = client->devId;
		}

		if (pClient->friends == NULL) {
			pClient->friends = ntyFriendsTreeInstance();
			
#if ENABLE_CONNECTION_POOL
			if (buffer[NEY_PROTO_VERSION_IDX] == NTY_PROTO_APP_VERSION) { //App
				if(-1 == ntyQueryWatchIDListSelectHandle(pClient->devId, pClient->friends)) {
					ntylog(" ntyQueryWatchIDListSelectHandle Failed \n");
				}
			} else if (buffer[NEY_PROTO_VERSION_IDX] == NTY_PROTO_DEVICE_VERSION) { //Device
				if (-1 == ntyQueryAppIDListSelectHandle(pClient->devId, pClient->friends)) {
					ntylog(" ntyQueryAppIDListSelectHandle Failed \n");
				}
			} else {
				ntylog(" Protocol Version is Error : %c\n", buffer[NEY_PROTO_VERSION_IDX]);
				//free(pClient);
				//return ;
			}
#endif

		}

		return 2;
	} 
	
	
#if ENABLE_NATTY_TIME_STAMP //TIME Stamp 	
	selfNode->stamp = ntyTimeStampGenrator();
#endif

	return 0;
}


/*
 * Login Packet
 * Login Packet, HeartBeatPacket, LogoutPacket etc. should use templete designer pattern
 */
void ntyLoginPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	const UdpClient *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_LOGIN_REQ) {
		int i = 0;
		void *pRBTree = ntyRBTreeInstance();
		
		ntyAddClientNodeToRBTree(buffer, length, obj);
		Client *pClient = (Client*)ntyRBTreeInterfaceSearch(pRBTree, client->devId);
		if (pClient != NULL) {
			ntySendFriendsTreeIpAddr(pClient, 1);

			ntylog(" Buffer Version : %x\n", buffer[NEY_PROTO_VERSION_IDX]);
			if (buffer[NEY_PROTO_VERSION_IDX] == NTY_PROTO_DEVICE_VERSION)
				ntySendDeviceTimeCheckAck(pClient, client->ackNum+1);
		}

		//ntylog("Login deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);		
		//send login ack

	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
	}
}


static const ProtocolFilter ntyLoginFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyLoginPacketHandleRequest,
};

/*
 * HeartBeatPacket
 */
void ntyHeartBeatPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	const UdpClient *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_HEARTBEAT_REQ) {

		ntyAddClientNodeToRBTree(buffer, length, obj);

	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
	}
}


static const ProtocolFilter ntyHeartBeatFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyHeartBeatPacketHandleRequest,
};


/*
 * Logout Packet
 */
void ntyLogoutPacketHandleRequest(const void *_self, U8 *buffer, int length, const void* obj) {
	const UdpClient *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_LOGOUT_REQ) {
		//delete key
#if 0
		void *pRBTree = ntyRBTreeInstance();
		C_DEVID key = *(C_DEVID*)(buffer+NTY_PROTO_HEARTBEAT_DEVID_IDX);
		U32 ackNum = *(U32*)(buffer+NTY_PROTO_HEARTBEAT_ACKNUM_IDX)+1;
		const UdpClient *client = obj;
		unsigned char ack[NTY_HEARTBEAT_ACK_LENGTH] = {0};
		struct ev_loop *loop = ntyTcpServerGetMainloop();

		UdpClient *cliValue = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, key);
		if (cliValue != NULL) {
			Delete(cliValue->friends);
		}

		ntyReleaseClientNodeSocket(loop, cliValue->watcher, cliValue->sockfd);
		
		ntyRBTreeInterfaceDelete(pRBTree, key);
		ntylog("Logout deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);

		//send logout ack
		ack[NTY_PROTO_TYPE_IDX] = NTY_PROTO_LOGOUT_ACK;
		memcpy(ack+1, &ackNum, NTY_ACKNUM_LENGTH);
		ntySendBuffer(client, ack, NTY_LOGOUT_ACK_LENGTH+4);
#else
#if 0
		ntylog("ntyLogoutPacketHandleRequest --> ");
		U32 ackNum = *(U32*)(buffer+NTY_PROTO_ACKNUM_IDX)+1;
		struct ev_loop *loop = ntyTcpServerGetMainloop();

		struct sockaddr_in addr;
		memcpy(&addr, &client->addr, sizeof(struct sockaddr_in));

		int ret = ntyReleaseClientNodeByDevID(loop, client->watcher, client->devId);
		ASSERT(ret == 0);

		ret = ntyDeleteNodeFromHashTable(&addr, client->devId);
		//ASSERT(ret == 0);
#endif

#endif
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
	}
}

static const ProtocolFilter ntyLogoutFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyLogoutPacketHandleRequest,
};

/*
 * P2P Packet
 */
void ntyP2PAddrReqPacketHandleRequest(const void *_self, U8 *buffer, int length, const void* obj) {
	//const UdpClient *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_ADDR_REQ) {
		int i = 0, length;
		void *pRBTree = ntyRBTreeInstance();

		C_DEVID key = *(C_DEVID*)(buffer+NTY_PROTO_P2PADDR_REQ_DEVID_IDX);
		U32 ackNum = *(U32*)(buffer+NTY_PROTO_P2PADDR_REQ_ACKNUM_IDX)+1;

		UdpClient *client = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, key);;
		U16 count = *(U16*)(&buffer[NTY_PROTO_P2PADDR_REQ_FRIENDS_COUNT_IDX]);
		U8 ack[NTY_P2PADDR_ACK_LENGTH] = {0};
#if 0		
		//deal with P2P addr Req
		for (i = 0;i < count;i ++) {
			C_DEVID reqKey = *(C_DEVID*)(&buffer[NTY_PROTO_P2PADDR_REQ_FRIENDS_DEVID_IDX(i)]);
			UdpClient *cliValue = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, reqKey);
			if (cliValue != NULL) {
				*(U32*)(&ack[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_ADDR_IDX(i)]) = (U32)(cliValue->addr.sin_addr.s_addr);
				*(U16*)(&ack[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_PORT_IDX(i)]) = (U16)(cliValue->addr.sin_port);

				
				U8 ackNotify[NTY_P2P_NOTIFY_ACK_LENGTH] = {0};
				*(C_DEVID*)(&ackNotify[NTY_PROTO_P2P_NOTIFY_DEVID_IDX]) = key;
				*(U32*)(&ackNotify[NTY_PROTO_P2P_NOTIFY_ACKNUM_IDX]) = ackNum;

				ntyP2PNotifyClient(cliValue, ackNotify);				
			}
			*(U16*)(&ack[NTY_PROTO_LOGIN_ACK_FRIENDS_COUNT_IDX]) = (U16)count;
			*(C_DEVID*)(&ack[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_DEVID_IDX(i)]) = reqKey;				
		}
		//send P2P ack
		*(U16*)(&ack[NTY_PROTO_P2PADDR_REQ_FRIENDS_COUNT_IDX]) = (U16)count;

		if (count == 0) {
			length = NTY_PROTO_P2PADDR_ACK_FRIENDSLIST_START_IDX;
			*(U32*)&ack[length] = ntyGenCrcValue(ack, length+1);				
		} else {
			length = NTY_PROTO_P2PADDR_ACK_CRC_IDX(count);
			*(U32*)&ack[length] = ntyGenCrcValue(ack, length+1);				
		}
		length = length + 1 + sizeof(U32);
		ntylog("P2P addr Req deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);

		ack[0] = 'A';
		ack[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2P_ADDR_ACK;
		//memcpy(ack+NTY_PROTO_LOGIN_ACK_ACKNUM_IDX, &ackNum, NTY_ACKNUM_LENGTH);
		*(U32*)(&ack[NTY_PROTO_LOGIN_ACK_ACKNUM_IDX]) = ackNum;
		ntySendBuffer(client, ack, length);
#else

		//client key is available and 

		//send ip addr to client		
		C_DEVID *devIdList = (C_DEVID*)malloc(count*sizeof(C_DEVID));
		for (i = 0;i < count;i ++) {
			*(devIdList+i) = *(C_DEVID*)(&buffer[NTY_PROTO_P2PADDR_REQ_FRIENDS_DEVID_IDX(i)]);
			//notify friend to connect this client
			ntyNotifyFriendConnect(client, *(devIdList+i));
		}
		ntySendIpAddrFriendsList(client, devIdList, count);
		free(devIdList);

		ntylog("P2P addr Req deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
#endif		
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
	}
}

static const ProtocolFilter ntyP2PAddrFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyP2PAddrReqPacketHandleRequest,
};


/*
 * this packet use for client have no NAT Punchthrough
 * use data for server proxy
 */

/*
 * Server Proxy Data Transport
 * VERSION					1			 BYTE
 * MESSAGE TYPE			 	1			 BYTE (req, ack)
 * TYPE				 	1			 BYTE 
 * DEVID					8			 BYTE (self devid)
 * ACKNUM					4			 BYTE (Network Module Set Value)
 * DEST DEVID				8			 BYTE (friend devid)
 * CONTENT COUNT			2			 BYTE 
 * CONTENT					*(CONTENT COUNT)	 BYTE 
 * CRC 				 	4			 BYTE (Network Module Set Value)
 * 
 * send to server addr, proxy to send one client
 * 
 */

void ntyUserDataPacketHandleRequest(const void *_self, unsigned char *buffer, int length,const void* obj) {
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_DATAPACKET_REQ) {

		C_DEVID destDevId = 0;//*(C_DEVID*)(&buffer[NTY_PROTO_DATAPACKET_DEST_DEVID_IDX]);
		ntyU8ArrayToU64(buffer+NTY_PROTO_DATAPACKET_DEST_DEVID_IDX, &destDevId);
		void *pRBTree = ntyRBTreeInstance();
		const Client *client = obj;
		int result = 0;
		
		U16 recByteCount = *(U16*)(&buffer[NTY_PROTO_DATAPACKET_CONTENT_COUNT_IDX]);
		ntylog(" ntyUserDataPacketHandleRequest --> recByteCount:%d, destDevId:%lld\n", recByteCount, destDevId);

		ntyAddClientNodeToRBTree(buffer, length, obj);

		U8 *data = (U8*)malloc((recByteCount+1)*sizeof(U8));
		memset(data, 0, (recByteCount+1));
		memcpy(data, buffer+NTY_PROTO_DATAPACKET_CONTENT_IDX, recByteCount);
		ntylog("data : %s\n", data);

		Client *destClient = (Client*)ntyRBTreeInterfaceSearch(pRBTree, destDevId);
		//ntylog(" destClient ... %llx\n", destDevId);
		if (destClient == NULL) {			
			if (destDevId == NTY_PROTO_MULTICAST_DEVID) { //destDevId == 0x0 boardcast all client
				int ret = -1;
				ntylog("destClient broadcast :%d, id:%lld\n", length, client->devId);

				buffer[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_RET;
				if (0 > (ret = ntyClassifyMessageType(client->devId, NTY_PROTO_MULTICAST_DEVID, data, recByteCount))) {
					ntylog(" ret : %d\n", ret);
					#if 0 //boardcast all client
					ntySendBuffer(client, buffer, length);
					#else
					ntyBoardcastAllFriends(client, data, recByteCount);
					#endif

				}
			} else {
				ntylog("destClient is no exist :%d, destId:%lld, id:%lld\n", length, destDevId, client->devId);
				int online = 0;
#if 1 //select device Login status, this operator should use redis, select in redis 
				if (buffer[NEY_PROTO_VERSION_IDX] == NTY_PROTO_DEVICE_VERSION) {
					ntyQueryDeviceOnlineStatusHandle(destDevId, &online);
				} else if (buffer[NEY_PROTO_VERSION_IDX] == NTY_PROTO_APP_VERSION) {
					ntyQueryAppOnlineStatusHandle(destDevId, &online);
				} else {
					ntylog(" ntyUserDataPacketHandleRequest --> Packet : %x\n", buffer[NEY_PROTO_VERSION_IDX]);
					assert(0);
				}
#endif
#if ENABLE_MULTICAST_SYNC //multicast 
				//buffer[NTY_PROTO_TYPE_IDX] = NTY_PROTO_MULTICAST_REQ;
				if (online == 1) { //no exist
					result = ntyMulticastSend(buffer, length);
				} else if (online == 0){
					result = ntySendBuffer(client, buffer, length);
				} else {
					assert(0);
				}
#else
				result = ntySendBuffer(client, buffer, length);
#endif

			}
		} else {
			int ret = -1;
#if 1
			ntylog(" Proxy data: %s, destDevId:%lld %d.%d.%d.%d:%d\n", data, destDevId, *(unsigned char*)(&destClient->addr.sin_addr.s_addr),
				*((unsigned char*)(&destClient->addr.sin_addr.s_addr)+1), *((unsigned char*)(&destClient->addr.sin_addr.s_addr)+2), 
				*((unsigned char*)(&destClient->addr.sin_addr.s_addr)+3), destClient->addr.sin_port);
			
#endif
			if (0 > (ret = ntyClassifyMessageType(client->devId, destClient->devId, data, recByteCount))) {
				ntylog(" ret : %d\n", ret);
				ntySendBuffer(destClient, buffer, length);
			}

		}
		
#if ENABLE_NODE_AGENT_SAVE //devId , data, recByteCount, token is set
		ntyNodeAgentProcess(data, recByteCount, client->devId);
#endif
		free(data);
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
	}

	return ;
}


static const ProtocolFilter ntyUserDataPacketFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyUserDataPacketHandleRequest,
};

void ntyUserDataPacketAckHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	const UdpClient *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_DATAPACKET_ACK) {
		C_DEVID destDevId = *(C_DEVID*)(&buffer[NTY_PROTO_DATAPACKET_DEST_DEVID_IDX]);
		void *pRBTree = ntyRBTreeInstance();
		UdpClient *destClient = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, destDevId);
		ntylog("%x %x %x %x\n", buffer[NTY_PROTO_MESSAGE_TYPE], buffer[NTY_PROTO_TYPE_IDX], 
			buffer[NTY_PROTO_TYPE_IDX+1], buffer[NTY_PROTO_TYPE_IDX+2]);
		if (destClient == NULL) {
			//C_DEVID selfDevId = *(C_DEVID*)(&buffer[NTY_PROTO_DEVID_IDX]);
			buffer[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_RET;
			U32 ack = *(U32*)(&buffer[NTY_PROTO_ACKNUM_IDX]);
			*(U32*)(&buffer[NTY_PROTO_ACKNUM_IDX]) = ack+1;
			
			ntySendBuffer(client, buffer, length);
		} else {
			ntySendBuffer(destClient, buffer, length);
		}
		ntylog("User Data Packet Ack deal\n");
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
	}

}

static const ProtocolFilter ntyUserDataPacketAckFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyUserDataPacketAckHandleRequest,
};

void ntyP2PConnectNotifyPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	const UdpClient *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_NOTIFY_REQ) {
		C_DEVID from = *(C_DEVID*)(&buffer[NTY_PROTO_P2P_NOTIFY_DEVID_IDX]);
		C_DEVID to = *(C_DEVID*)(&buffer[NTY_PROTO_P2P_NOTIFY_DEST_DEVID_IDX]);

		void *pRBTree = ntyRBTreeInstance();
		UdpClient *fromClient = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, from);
		if (fromClient != NULL) {
			fromClient->ackNum = *(U32*)(&buffer[NTY_PROTO_P2P_NOTIFY_ACKNUM_IDX]);
		}
		ntyNotifyFriendMessage(from, to);
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
	}

}


static const ProtocolFilter ntyP2PConnectNotifyPacketFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyP2PConnectNotifyPacketHandleRequest,
};

void ntyP2PConnectNotifyPacketAckHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	const UdpClient *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_NOTIFY_ACK) {
		C_DEVID destDevId = *(C_DEVID*)(&buffer[NTY_PROTO_DATAPACKET_DEST_DEVID_IDX]);
		void *pRBTree = ntyRBTreeInstance();
		UdpClient *destClient = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, destDevId);
		ntylog(" destDevId:%lld\n", destDevId);
		if (destClient == NULL) {
			//C_DEVID selfDevId = *(C_DEVID*)(&buffer[NTY_PROTO_DEVID_IDX]);
			buffer[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_RET;
			U32 ack = *(U32*)(&buffer[NTY_PROTO_ACKNUM_IDX]);
			*(U32*)(&buffer[NTY_PROTO_ACKNUM_IDX]) = ack+1;

			ntylog("send to srcClient\n");
			ntySendBuffer(client, buffer, length);
		} else {
			ntylog("send to destClient\n");
			ntySendBuffer(destClient, buffer, length);
		}
		ntylog("P2P Connect Notify Packet Ack deal\n");
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
	}

}

static const ProtocolFilter ntyP2PConnectNotifyPacketAckFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyP2PConnectNotifyPacketAckHandleRequest,
};


void ntyTimeCheckHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	const UdpClient *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_TIME_CHECK_REQ) {
		C_DEVID key = *(C_DEVID*)(buffer+NTY_PROTO_LOGIN_REQ_DEVID_IDX);
		U32 ackNum = *(U32*)(buffer+NTY_PROTO_LOGIN_REQ_ACKNUM_IDX)+1;
		
		void *pRBTree = ntyRBTreeInstance();
		UdpClient *cliValue = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, key);
		
		ntySendDeviceTimeCheckAck(client, ackNum);
		
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
	}

}


static const ProtocolFilter ntyTimeCheckFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyTimeCheckHandleRequest,
};

void ntyUnBindDevicePacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	const UdpClient *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_UNBIND_REQ && buffer[NEY_PROTO_VERSION_IDX] == NTY_PROTO_APP_VERSION) {
		C_DEVID AppId = *(C_DEVID*)(buffer+NTY_PROTO_UNBIND_APPID_IDX);
		C_DEVID DeviceId = *(C_DEVID*)(buffer+NTY_PROTO_UNBIND_DEVICEID_IDX);

#if ENABLE_CONNECTION_POOL
		int ret = ntyExecuteDevAppRelationDeleteHandle(AppId, DeviceId);
		if (ret == -1) {
			ntylog(" ntyUnBindDevicePacketHandleRequest --> DB Exception\n");
			ret = 4;
		} else if (ret == 0) {
			void *pRBTree = ntyRBTreeInstance();
			Client *aclient = (Client*)ntyRBTreeInterfaceSearch(pRBTree, AppId);
			if (aclient != NULL) {
				if (aclient->friends != NULL) {
					ntyFriendsTreeDelete(aclient->friends, DeviceId);
				}
			}

			Client *dclient = (Client*)ntyRBTreeInterfaceSearch(pRBTree, DeviceId);
			if (dclient != NULL) {
				if (dclient->friends != NULL) {
					ntyFriendsTreeDelete(dclient->friends, AppId);
				}
			}
		}
		ntyProtoUnBindAck(AppId, DeviceId, ret);
#endif		
		
		
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
	}

}


static const ProtocolFilter ntyUnBindDeviceFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyUnBindDevicePacketHandleRequest,
};

void ntyBindDevicePacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	const Client *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_BIND_REQ && buffer[NEY_PROTO_VERSION_IDX] == NTY_PROTO_APP_VERSION) {
		C_DEVID AppId = *(C_DEVID*)(buffer+NTY_PROTO_BIND_APPID_IDX);
		C_DEVID DeviceId = *(C_DEVID*)(buffer+NTY_PROTO_BIND_DEVICEID_IDX);

#if ENABLE_CONNECTION_POOL
		U8 DevImei[16] = {0};
		sprintf(DevImei, "%llx", DeviceId);
		int ret = ntyQueryDevAppRelationInsertHandle(AppId, DevImei);
		if (ret == -1) {
			ntylog(" ntyBindDevicePacketHandleRequest --> DB Exception\n");
			ret = 4;
		} else if (ret == 0) { //Bind Success Update RBTree
			void *pRBTree = ntyRBTreeInstance();
			Client *aclient = (Client*)ntyRBTreeInterfaceSearch(pRBTree, AppId);
			if (aclient != NULL) {
				if (aclient->friends != NULL) {
					ntyFriendsTreeInsert(aclient->friends, DeviceId);
				}
			}

			Client *dclient = (Client*)ntyRBTreeInterfaceSearch(pRBTree, DeviceId);
			if (dclient != NULL) {
				if (dclient->friends != NULL) {
					ntyFriendsTreeInsert(dclient->friends, AppId);
				}
			}
		}
		ntyProtoBindAck(AppId, DeviceId, ret);
#endif		
		//if ()
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
	}

}


static const ProtocolFilter ntyBindDeviceFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyBindDevicePacketHandleRequest,
};


void ntyMulticastReqPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	const Client *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_MULTICAST_REQ && client->clientType == PROTO_TYPE_MULTICAST) {
		C_DEVID toId = 0;
		ntyU8ArrayToU64(buffer+NTY_PROTO_DEST_DEVID_IDX, &toId);

		void *pRBTree = ntyRBTreeInstance();
		Client *toClient = (Client*)ntyRBTreeInterfaceSearch(pRBTree, toId);
		if (toClient == NULL) { //no Exist
			return ;
		} 

		buffer[NTY_PROTO_MULTICAST_TYPE_IDX] = NTY_PROTO_DATAPACKET_REQ;
		ntySendBuffer(toClient, buffer, length);
		//ntyMulticastSend();
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
	}
}


static const ProtocolFilter ntyMutlcastReqFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyMulticastReqPacketHandleRequest,
};


void ntyMulticastAckPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	const Client *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_MULTICAST_ACK && client->clientType == PROTO_TYPE_MULTICAST) {
		C_DEVID fromId = 0;
			
		ntyU8ArrayToU64(buffer+NTY_PROTO_DEST_DEVID_IDX, &fromId);

	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
	}
}


static const ProtocolFilter ntyMutlcastAckFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyMulticastAckPacketHandleRequest,
};


void ntyICCIDReqPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	const Client *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_ICCID_REQ && buffer[NEY_PROTO_VERSION_IDX] == NTY_PROTO_DEVICE_VERSION) {
		C_DEVID selfId = 0;
		
#define NTY_ICCID_LENGTH				20
#define NTY_ICCID_HEADER_LENGTH			6
#define NTY_ICCID_PHNUM_LENGTH			16
		U8 iccid[NTY_ICCID_LENGTH] = {0}, *pICCID; 
		U8 phnum[NTY_ICCID_PHNUM_LENGTH] = {0};
		U16 len = 0;
		
		ntyU8ArrayToU64(buffer+NTY_PROTO_ICCIDREQ_SLFID_IDX, &selfId);
		len = *(U16*)(buffer+NTY_PROTO_ICCIDREQ_CONTENT_COUNT_IDX);
		if (len != NTY_ICCID_LENGTH) {
			ntylog(" ntyICCIDReqPacketHandleRequest --> iccid is Error %d, %s\n", len, buffer+NTY_PROTO_ICCIDREQ_CONTENT_IDX);
			return ;
		} 
		memcpy(iccid, buffer+NTY_PROTO_ICCIDREQ_CONTENT_IDX, NTY_ICCID_LENGTH);
#if 1 //iccid parse
		iccid[NTY_ICCID_LENGTH-1] = 0x0; //Chsum Cancel
		pICCID = iccid + NTY_ICCID_HEADER_LENGTH; //iccid header
#endif
		ntyQueryPhNumSelectHandle(selfId, pICCID, phnum);
		ntylog(" ntyICCIDReqPacketHandleRequest --> phnum:%s, %ld\n", phnum, strlen(phnum));

		ntyProtoICCIDAck(selfId, phnum, strlen(phnum));
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
	}
}


static const ProtocolFilter ntyICCIDReqFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyICCIDReqPacketHandleRequest,
};

#if 1
U8 u8VoicePacket[NTY_VOICEREQ_COUNT_LENGTH*NTY_VOICEREQ_PACKET_LENGTH] = {0};
#endif

void ntyVoiceReqPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	const Client *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_VOICE_REQ) {
#if 1
		int i = 0;
		U16 index = *(U16*)(buffer+NTY_PROTO_VOICEREQ_PKTINDEX_IDX);
		U16 Count = *(U16*)(&buffer[NTY_PROTO_VOICEREQ_PKTTOTLE_IDX]);
		U32 pktLength = *(U32*)(&buffer[NTY_PROTO_VOICEREQ_PKTLENGTH_IDX]);

		ntylog(" Count:%d, index:%d, pktLength:%d, length:%d, pktLength%d\n", 
			Count, index, pktLength, length, NTY_PROTO_VOICEREQ_DESTID_IDX);

		C_DEVID toId = 0, fromId = 0;
		ntyU8ArrayToU64(buffer+NTY_PROTO_VOICEREQ_SELFID_IDX, &fromId);
		ntyU8ArrayToU64(buffer+NTY_PROTO_VOICEREQ_DESTID_IDX, &toId);

		for (i = 0;i < sizeof(C_DEVID);i ++) {
			ntylog(" %2x", *(buffer+NTY_PROTO_VOICEREQ_DESTID_IDX+i));
		}
		ntylog("\n toId %lld, fromId %lld\n", toId, fromId);

		void *pRBTree = ntyRBTreeInstance();
		Client *toClient = (Client*)ntyRBTreeInterfaceSearch(pRBTree, toId);
		if (toClient == NULL) { //no Exist
			ntylog(" toClient is not Exist\n");

			int online = 0;
			int result = 0;
#if 1 //select device Login status, this operator should use redis, select in redis 
			if (buffer[NEY_PROTO_VERSION_IDX] == NTY_PROTO_DEVICE_VERSION) {
				ntyQueryDeviceOnlineStatusHandle(toId, &online);
			} else if (buffer[NEY_PROTO_VERSION_IDX] == NTY_PROTO_APP_VERSION) {
				ntyQueryAppOnlineStatusHandle(toId, &online);
			} else {
				ntylog(" ntyUserDataPacketHandleRequest --> Packet : %x\n", buffer[NEY_PROTO_VERSION_IDX]);
				assert(0);
			}
#endif
#if ENABLE_MULTICAST_SYNC //multicast 
			//buffer[NTY_PROTO_TYPE_IDX] = NTY_PROTO_MULTICAST_REQ;
			if (online == 1) { //no exist
				result = ntyMulticastSend(buffer, length);
			} else if (online == 0){
				result = ntySendBuffer(client, buffer, length);
			} else {
				assert(0);
			}
#else
			result = ntySendBuffer(client, buffer, length);
#endif


			return ;
		} 
		ntySendBuffer(toClient, buffer, length);
#else
		int i = 0;
		
		U16 index = *(U16*)(buffer+NTY_PROTO_VOICEREQ_PKTINDEX_IDX);
		U16 Count = *(U16*)(&buffer[NTY_PROTO_VOICEREQ_PKTTOTLE_IDX]);
		U32 pktLength = *(U32*)(&buffer[NTY_PROTO_VOICEREQ_PKTLENGTH_IDX]);

		ntylog(" Count:%d, index:%d, pktLength:%d, length:%d, pktLength%d\n", 
			Count, index, pktLength, length, NTY_PROTO_VOICEREQ_PKTLENGTH_IDX);

		ntySendBuffer(client, buffer, length);
		
		for (i = 0;i < pktLength;i ++) {
			u8VoicePacket[index * NTY_VOICEREQ_DATA_LENGTH + i] = buffer[NTY_VOICEREQ_HEADER_LENGTH + i];
		}
		if (index == Count-1) {
			U32 dataLength = NTY_VOICEREQ_DATA_LENGTH*(Count-1) + pktLength;
			ntyWriteDat("./rVoice.amr", u8VoicePacket, dataLength);

		}
#endif
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
	}
}

static const ProtocolFilter ntyVoiceReqFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyVoiceReqPacketHandleRequest,
};

void ntyVoiceAckPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	const Client *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_VOICE_ACK) {
		C_DEVID toId = 0, fromId = 0;
		ntyU8ArrayToU64(buffer+NTY_PROTO_VOICEACK_SELFID_IDX, &fromId);
		ntyU8ArrayToU64(buffer+NTY_PROTO_VOICEACK_DESTID_IDX, &toId);

		ntylog(" ntyVoiceAckPacketHandleRequest --> toId:%lld, fromId:%lld\n", toId, fromId);

		void *pRBTree = ntyRBTreeInstance();
		Client *toClient = (Client*)ntyRBTreeInterfaceSearch(pRBTree, toId);
		if (toClient == NULL) { //no Exist
			return ;
		} 

		ntySendBuffer(toClient, buffer, length);
		
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
	}
}

static const ProtocolFilter ntyVoiceAckFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyVoiceAckPacketHandleRequest,
};

#if 0
void ntyEFenceDataReqPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	const Client *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_EFENCE_DATA_REQ) {
		
		
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
	}
}

static const ProtocolFilter ntyEFenceDataReqFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyEFenceDataReqPacketHandleRequest,
};



void ntyEFenceDataAckPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	const Client *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_EFENCE_DATA_ACK) {
		
		
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
	}
}

static const ProtocolFilter ntyEFenceDataAckFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyEFenceDataAckPacketHandleRequest,
};
#endif

//Set EFence 1:4:lng1|lat1;lng2|lat2;lng3|lat3;lng4|lat4:Efence1
//Set EFenceList 1:4:lng1|lat1;lng2|lat2;lng3|lat3;lng4|lat4:Efence1#2:4:lng1|lat1;lng2|lat2;lng3|lat3;lng4|lat4:Efence1:appid:startTime:endTime
void ntyEFenceReqPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	const Client *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_EFENCE_REQ) {
		C_DEVID toId = 0, fromId = 0;
		
		ntyU8ArrayToU64(buffer+NTY_PROTO_EFENCE_SLEFID_IDX, &fromId);
		ntyU8ArrayToU64(buffer+NTY_PROTO_EFENCE_DESTID_IDX, &toId);

		if (toId == NTY_PROTO_MULTICAST_DEVID) {
			U16 recByteCount = *(U16*)(&buffer[NTY_PROTO_EFENCE_CONTENT_COUNT_IDX]);
			U8 *data = (U8*)malloc((recByteCount+1)*sizeof(U8));
			
			memset(data, 0, (recByteCount+1));
			memcpy(data, buffer+NTY_PROTO_EFENCE_CONTENT_IDX, recByteCount);
			ntylog("data : %s\n", data);

			ntyBoardcastAllFriends(client, data, recByteCount); //mulcast all client

			free(data);
			return ;
		}

		void *pRBTree = ntyRBTreeInstance();
		Client *toClient = (Client*)ntyRBTreeInterfaceSearch(pRBTree, toId);
		if (toClient == NULL) { //no Exist
			//device is not exist
			ntySendBuffer(client, buffer, length);
			return ;
		} 

		ntySendBuffer(toClient, buffer, length);
		
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
	}
}

static const ProtocolFilter ntyEFenceReqFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyEFenceReqPacketHandleRequest,
};


void ntyEFenceAckPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	const Client *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_EFENCE_ACK) {
		C_DEVID toId = 0, fromId = 0;
		
		ntyU8ArrayToU64(buffer+NTY_PROTO_EFENCE_SLEFID_IDX, &fromId);
		ntyU8ArrayToU64(buffer+NTY_PROTO_EFENCE_DESTID_IDX, &toId);

		void *pRBTree = ntyRBTreeInstance();
		Client *toClient = (Client*)ntyRBTreeInterfaceSearch(pRBTree, toId);
		if (toClient == NULL) { //no Exist
			//device is not exist
			ntySendBuffer(client, buffer, length);
			return ;
		} 

		ntySendBuffer(toClient, buffer, length);
		
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
	}
}

static const ProtocolFilter ntyEFenceAckFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyEFenceAckPacketHandleRequest,
};



static void ntySetSuccessor(void *_filter, void *_succ) {
	ProtocolFilter **filter = _filter;
	if (_filter && (*filter) && (*filter)->setSuccessor) {
		(*filter)->setSuccessor(_filter, _succ);
	}
}

static void ntyHandleRequest(void *_filter, unsigned char *buffer, U32 length, const void *obj) {
	ProtocolFilter **filter = _filter;
	if (_filter && (*filter) && (*filter)->handleRequest) {
		(*filter)->handleRequest(_filter, buffer, length, obj);
	}
}



const void *pNtyLoginFilter = &ntyLoginFilter;
const void *pNtyHeartBeatFilter = &ntyHeartBeatFilter;
const void *pNtyLogoutFilter = &ntyLogoutFilter;
const void *pNtyP2PAddrReqFilter = &ntyP2PAddrFilter;
const void *pNtyUserDataPacketFilter = &ntyUserDataPacketFilter;
const void *pNtyUserDataPacketAckFilter = &ntyUserDataPacketAckFilter;
const void *pNtyP2PConnectNotifyPacketFilter = &ntyP2PConnectNotifyPacketFilter;
const void *pNtyP2PConnectNotifyPacketAckFilter = &ntyP2PConnectNotifyPacketAckFilter;
const void *pNtyTimeCheckFilter = &ntyTimeCheckFilter;
const void *pNtyUnBindDeviceFilter = &ntyUnBindDeviceFilter;
const void *pNtyBindDeviceFilter = &ntyBindDeviceFilter;
const void *pNtyMutlcastReqFilter = &ntyMutlcastReqFilter;
const void *pNtyMutlcastAckFilter = &ntyMutlcastAckFilter;
const void *pNtyICCIDReqFilter = &ntyICCIDReqFilter;
const void *pNtyVoiceReqFilter = &ntyVoiceReqFilter;
const void *pNtyVoiceAckFilter = &ntyVoiceAckFilter;

//const void *pNtyEFenceDataReqFilter = &ntyEFenceDataReqFilter;
//const void *pNtyEFenceDataAckFilter = &ntyEFenceDataAckFilter;

const void *pNtyEFenceReqFilter = &ntyEFenceReqFilter;
const void *pNtyEFenceAckFilter = &ntyEFenceAckFilter;


void* ntyProtocolFilterInit(void) {
	void *pHeartBeatFilter = New(pNtyHeartBeatFilter);
	void *pLoginFilter = New(pNtyLoginFilter);
	void *pLogoutFilter = New(pNtyLogoutFilter);
	
	void *pP2PAddrReqFilter = New(pNtyP2PAddrReqFilter);
	
	void *pUserDataPacketFilter = New(pNtyUserDataPacketFilter);
	void *pUserDataPacketAckFilter = New(pNtyUserDataPacketAckFilter);
	
	void *pP2PConnectNotifyPacketFilter = New(pNtyP2PConnectNotifyPacketFilter);
	void *pP2PConnectNotifyPacketAckFilter = New(pNtyP2PConnectNotifyPacketAckFilter);
	void *pTimeCheckFilter = New(pNtyTimeCheckFilter);
	
	void *pUnBindDeviceFilter = New(pNtyUnBindDeviceFilter);
	void *pBindDeviceFilter = New(pNtyBindDeviceFilter);
	
	void *pMutlcastReqFilter = New(pNtyMutlcastReqFilter);
	void *pMutlcastAckFilter = New(pNtyMutlcastAckFilter);
	void *pICCIDReqFilter = New(pNtyICCIDReqFilter);
	
	void *pVoiceReqFilter = New(pNtyVoiceReqFilter);
	void *pVoiceAckFilter = New(pNtyVoiceAckFilter);

	//void *pEFenceDataReqFilter = New(pNtyEFenceDataReqFilter);
	//void *pEFenceDataAckFilter = New(pNtyEFenceDataAckFilter);

	void *pEFenceReqFilter = New(pNtyEFenceReqFilter);
	void *pEFenceAckFilter = New(pNtyEFenceAckFilter);

	ntySetSuccessor(pHeartBeatFilter, pLoginFilter);
	ntySetSuccessor(pLoginFilter, pLogoutFilter);
	ntySetSuccessor(pLogoutFilter, pP2PAddrReqFilter);
	ntySetSuccessor(pP2PAddrReqFilter, pUserDataPacketFilter);
	
	ntySetSuccessor(pUserDataPacketFilter, pUserDataPacketAckFilter);
	ntySetSuccessor(pUserDataPacketAckFilter, pP2PConnectNotifyPacketFilter);
	ntySetSuccessor(pP2PConnectNotifyPacketFilter, pTimeCheckFilter);
	ntySetSuccessor(pTimeCheckFilter, pUnBindDeviceFilter);
	ntySetSuccessor(pUnBindDeviceFilter, pBindDeviceFilter);
	
	ntySetSuccessor(pBindDeviceFilter, pMutlcastReqFilter);
	ntySetSuccessor(pMutlcastReqFilter, pMutlcastAckFilter);
	ntySetSuccessor(pMutlcastAckFilter, pICCIDReqFilter);
	
	ntySetSuccessor(pICCIDReqFilter, pVoiceReqFilter);
	ntySetSuccessor(pVoiceReqFilter, pVoiceAckFilter);
	
	ntySetSuccessor(pVoiceAckFilter, pEFenceReqFilter);
	ntySetSuccessor(pEFenceReqFilter, pEFenceAckFilter);
	ntySetSuccessor(pEFenceAckFilter, NULL);

	
	/*
	 * add your Filter
	 * for example:
	 * void *pFilter = New(NtyFilter);
	 * ntySetSuccessor(pLogoutFilter, pFilter);
	 */

	//Gen Crc Table
	ntyGenCrcTable();

#if ENABLE_NATTY_TIME_STAMP
	pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;
	memcpy(&time_mutex, &blank_mutex, sizeof(blank_mutex));
	memcpy(&loop_mutex, &blank_mutex, sizeof(blank_mutex));
#endif

	return pHeartBeatFilter;
}

static void ntyClientBufferRelease(Client *client) {
	pthread_mutex_lock(&client->buffer_mutex);
#if 0
	free(client->recvBuffer);
	client->recvBuffer = NULL;
	client->rLength = 0;
#else
	memset(client->recvBuffer, 0, PACKET_BUFFER_SIZE);
	client->rLength = 0;
#endif
	pthread_mutex_unlock(&client->buffer_mutex);
}

void ntyProtocolFilterProcess(void *_filter, unsigned char *buffer, U32 length, const void *obj) {
	
	//data crc is right, and encryto
	U32 u32Crc = ntyGenCrcValue(buffer, length-4);
	U32 u32ClientCrc = *((U32*)(buffer+length-4));
	const Client *client = obj;
	struct sockaddr_in addr;
	memcpy(&addr, &client->addr, sizeof(struct sockaddr_in));

	C_DEVID id = ntySearchDevIdFromHashTable(&addr);
	ntylog("ntyProtocolFilterProcess :%lld\n", id);
	if (id == 0) return ;
	
	void *pRBTree = ntyRBTreeInstance();
	Client *pClient = (Client*)ntyRBTreeInterfaceSearch(pRBTree, id);
	
	//ntydbg(" client:%x, server:%x, length:%d", u32ClientCrc, u32Crc, length);
	if (u32Crc != u32ClientCrc || length < NTY_PROTO_MIN_LENGTH) {
#if 1 
		ntylog(" ntyProtocolFilterProcess --> client:%x, server:%x, length:%d\n", u32ClientCrc, u32Crc, length);
		//const Client *client = obj;
		if (pClient == NULL) return ;
		if (client->clientType == PROTO_TYPE_TCP) {
			//Hash table have no Node that client addr
			//struct sockaddr_in addr;
			//memcpy(&addr, &client->addr, sizeof(struct sockaddr_in));
			
			ntylog(" ntyProtocolFilterProcess --> Data Format is Error : %d, DeId:%lld\n", length, id);
			
			if(id == 0) { //have no client id
				ntylog(" ntyProtocolFilterProcess --> Release Client\n");
				struct ev_loop *loop = ntyTcpServerGetMainloop();
				ntyReleaseClientNodeSocket(loop, client->watcher, client->watcher->fd);

				return ;
			} else { //have part data
				//void *pRBTree = ntyRBTreeInstance();
				//Client *pClient = (Client*)ntyRBTreeInterfaceSearch(pRBTree, id);
				
				ntylog("111111111 pClient->rLength:%d, length:%d\n", pClient->rLength, length);

				int bCopy = 0;
				int bIndex = 0, bLength = pClient->rLength;
				U8 bBuffer[PACKET_BUFFER_SIZE] = {0};

				do {
					bCopy = (length > PACKET_BUFFER_SIZE ? PACKET_BUFFER_SIZE : length);					
					bCopy = (((bLength + bCopy) > PACKET_BUFFER_SIZE) ? (PACKET_BUFFER_SIZE - bLength) : bCopy);
					
					pthread_mutex_lock(&pClient->buffer_mutex);
					memcpy(pClient->recvBuffer+pClient->rLength, buffer+bIndex, bCopy);
					pClient->rLength %= PACKET_BUFFER_SIZE;
					pClient->rLength += bCopy;

					memcpy(bBuffer, pClient->recvBuffer, pClient->rLength);
					bLength = pClient->rLength;
					pthread_mutex_unlock(&pClient->buffer_mutex);

					U32 uCrc = ntyGenCrcValue(bBuffer, bLength-4);
					U32 uClientCrc = *((U32*)(bBuffer+bLength-4));

					if (uCrc == uClientCrc)	{
						ntylog(" CMD:%x, Version:[%c]\n", bBuffer[NTY_PROTO_TYPE_IDX], bBuffer[NEY_PROTO_VERSION_IDX]);
						ntyHandleRequest(_filter, bBuffer, bLength, obj);

						ntyClientBufferRelease(pClient);
					} 
					
					length -= bCopy;
					bIndex += bCopy;
					
				} while (length);

				return ;

			}

		}
#endif
		
	}

	if (pClient != NULL) {
		pClient->rLength = 0;
	}
	return ntyHandleRequest(_filter, buffer, length, obj);
}

void ntyProtocolFilterRelease(void *_filter) {
	Packet *self = _filter;
	if (ntyPacketGetSuccessor(self) != NULL) {
		ntyProtocolFilterRelease(self->succ);
	}
	Delete(self);
}

static void *ntyProtocolFilter = NULL;

void* ntyProtocolFilterInstance(void) {
	if (ntyProtocolFilter == NULL) {
		void* pFilter = ntyProtocolFilterInit();
		if ((unsigned long)NULL != cmpxchg((void*)(&ntyProtocolFilter), (unsigned long)NULL, (unsigned long)pFilter, WORD_WIDTH)) {
			Delete(pFilter);
		}
	}
	return ntyProtocolFilter;
}

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

#if 1


int ntyReleaseClientNodeByNode(struct ev_loop *loop, void *node) {
	Client *client = node;
	if (client == NULL) return -4;
#if 0
	ntylog("ntyDeleteNodeFromHashTable Start --> %d.%d.%d.%d:%d \n", *(unsigned char*)(&client->addr.sin_addr.s_addr), *((unsigned char*)(&client->addr.sin_addr.s_addr)+1),													
				*((unsigned char*)(&client->addr.sin_addr.s_addr)+2), *((unsigned char*)(&client->addr.sin_addr.s_addr)+3),													
				client.addr->sin_port);
#endif
	int ret = ntyDeleteNodeFromHashTable(&client->addr, client->devId);
	ASSERT(ret == 0);

#if 1 //
	if (client->deviceType == DEVICE_TYPE_WATCH) {
		ret = ntyExecuteDeviceLogoutUpdateHandle(client->devId);
		ntylog(" ntyReleaseClientNodeByNode --> Logout ret : %d\n", ret);
	}
#endif

	ret = ntyReleaseClientNodeSocket(loop, client->watcher, client->watcher->fd);
	ASSERT(ret == 0);

	void *tree = ntyRBTreeInstance();
	ret = ntyRBTreeInterfaceDelete(tree, client->devId);
	ASSERT(ret == 0);
		
	return 0;
}

int ntyReleaseClientNodeByDevID(struct ev_loop *loop, struct ev_io *watcher, C_DEVID devid) {
	int ret = -1;
	void *tree = ntyRBTreeInstance();

	Client *client = ntyRBTreeInterfaceSearch(tree, devid);
	if (client == NULL) {
		ntylog(" ntyReleaseClientNodeByDevID --> Client == NULL\n");
		return -1;
	}
	ntyReleaseClientNodeSocket(loop, client->watcher, client->watcher->fd);
	
	ret = ntyRBTreeInterfaceDelete(tree, devid);
	ASSERT(ret == 0);
	
	return 0;
}

int ntyReleaseClientNodeByAddr(struct ev_loop *loop, struct sockaddr_in *addr, struct ev_io *watcher) {
	ntylog(" ntyReleaseClientNodeByAddr --> Start \n");
	
	C_DEVID devid = ntySearchDevIdFromHashTable(addr);
	int ret = ntyDeleteNodeFromHashTable(addr, devid);
	if (ret == -1) {
		ntylog("Delete Node From Hash Table Parameter is Error\n");
		return -1;
	} else if (ret == -2) {
		ntylog("Hash Table Node is not Exist\n");
	}

#if 1 //Update Login info
	ntyExecuteDeviceLogoutUpdateHandle(devid);
#endif

	ntylog("Delete Node Success devid : %lld\n", devid);
	ntyReleaseClientNodeSocket(loop, watcher, watcher->fd);
	
	void *tree = ntyRBTreeInstance();
	//ntyRBTreeOperaDelete
	// delete rb-tree node
	ret = ntyRBTreeInterfaceDelete(tree, devid);
	if (ret == 1) {
		ntylog("RBTree Node Not Exist\n");
		return -2;
	} else if (ret == -1) {
		ntylog("RBTree Node Parameter is Error\n");
		return -3;
	}

	ntylog("RBTree Delete Node Success\n");

	return 0;
}

int ntyReleaseClientNodeSocket(struct ev_loop *loop, struct ev_io *watcher, int sockfd) {
	if (watcher == NULL) { 
		ntylog(" ntyReleaseClientNodeSocket --> watcher == NULL\n");
		return -1;
	}
	if (watcher->fd != sockfd) {
		ntylog(" ntyReleaseClientNodeSocket --> watcher->fd :%d, sockfd :%d\n", watcher->fd, sockfd);
		return -1;
	}
	if (sockfd < 0) return -2;
	
#if ENABLE_MAINLOOP_MUTEX //TIME Stamp 	
	pthread_mutex_lock(&loop_mutex);
	ev_io_stop(loop, watcher);
	pthread_mutex_unlock(&loop_mutex);
#endif

	close(sockfd);
	sockfd = -1;
	free(watcher);

	return 0;
}


int ntyReleaseClientNodeHashTable(struct sockaddr_in *addr) {
	C_DEVID devid = ntySearchDevIdFromHashTable(addr);

	ntylog(" ntyReleaseClientNodeHashTable --> %lld\n", devid);
	int ret = ntyDeleteNodeFromHashTable(addr, devid);
	if (ret == -1) {
		ntylog("Delete Node From Hash Table Parameter is Error\n");
		return -1;
	} else if (ret == -2) {
		ntylog("Hash Table Node is not Exist\n");
	}
	return 0;
}

#endif

