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
/*
 * Login Packet
 * Login Packet, HeartBeatPacket, LogoutPacket etc. should use templete designer pattern
 */
void ntyLoginPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	const UdpClient *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_LOGIN_REQ) {
		int i = 0;
		void *pRBTree = ntyRBTreeInstance();
#if 0
		C_DEVID key = *(C_DEVID*)(buffer+NTY_PROTO_LOGIN_REQ_DEVID_IDX);
		U32 ackNum = *(U32*)(buffer+NTY_PROTO_LOGIN_REQ_ACKNUM_IDX)+1;
#else
		C_DEVID key = client->devId;
		U32 ackNum = client->ackNum;
#endif
		U8 ack[NTY_LOGIN_ACK_LENGTH] = {0};

		UdpClient *cliValue = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, key);
		if (cliValue == NULL) {		
			
			UdpClient *pClient = (UdpClient*)malloc(sizeof(UdpClient));
			pClient->sockfd = client->sockfd;
			pClient->clientType = client->clientType;
			pClient->addr.sin_addr.s_addr = client->addr.sin_addr.s_addr;
			pClient->addr.sin_port = client->addr.sin_port;
			pClient->ackNum = ackNum;
			pClient->devId = key;

#if ENABLE_NATTY_TIME_STAMP //TIME Stamp 	
			pthread_mutex_lock(&time_mutex);
			pClient->stamp = time(NULL);
			pthread_mutex_unlock(&time_mutex);
			if (client->clientType == PROTO_TYPE_TCP) {
				pClient->watcher = client->watcher;
			} else {
				pClient->watcher = NULL;
			}
#endif
			//new firends tree			
			pClient->friends = ntyFriendsTreeInstance();
			//read from disk friends and store in rb-tree
#if 1	//just for debug		
			ntylog(" key : %lld\n", key);
			if (key == 1) {
				ntyFriendsTreeInsert(pClient->friends, 2);
				ntyFriendsTreeInsert(pClient->friends, 3);
			} else if (key == 2) {
				ntyFriendsTreeInsert(pClient->friends, 1);
				//ntyFriendsTreeInsert(pClient->friends, 3);
			} else if (key == 3) {
				ntyFriendsTreeInsert(pClient->friends, 1);
			}
#endif

#if ENABLE_CONNECTION_POOL
			if (buffer[NEY_PROTO_VERSION_IDX] == NTY_PROTO_APP_VERSION) { //App
				if(-1 == ntyQueryWatchIDListSelectHandle(key, pClient->friends)) {
					ntylog(" ntyQueryWatchIDListSelectHandle Failed \n");
				}
			} else if (buffer[NEY_PROTO_VERSION_IDX] == NTY_PROTO_DEVICE_VERSION) { //Device
				if (-1 == ntyQueryAppIDListSelectHandle(key, pClient->friends)) {
					ntylog(" ntyQueryAppIDListSelectHandle Failed \n");
				}
			} else {
				ntylog(" Protocol Version is Error : %c\n", buffer[NEY_PROTO_VERSION_IDX]);
				//free(pClient);
				//return ;
			}
#endif
			//insert rb-tree
			ntylog(" New Client RBTREENODE --> %lld\n", key);
			if (ntyRBTreeInterfaceInsert(pRBTree, key, pClient)) {
				ntylog("Client is Exist\n");
				free(pClient);
				return ;
			}

			//notify friends 	
			// this step hive off
			//ntyFriendsTreeTraversalNotify(pClient->friends, key, ntyNotifyFriendMessage);
			//send friends list to this client.
			//ntyFriendsTreeGetAllNodeList
			ntySendFriendsTreeIpAddr(pClient, 1);
#if 1 //Insert Hash table
			ntylog("Insert Hash Table\n");
			int ret = ntyInsertNodeToHashTable(&pClient->addr, key);
			if (ret == 0) {				
				ntylog("Hash Table Node Insert Success\n");
			}
#endif
			ntySendDeviceTimeCheckAck(pClient, ackNum+1);
			return ;
		} else if ((cliValue != NULL) && (!ntyClientCompare (client, cliValue))) {
			UdpClient *pClient = cliValue;//(UdpClient*)malloc(sizeof(UdpClient));
#if 0
			ntylog(" ntyLoginPacketHandleRequest --> client: %d.%d.%d.%d:%d --> cliValue: %d.%d.%d.%d:%d, ip have changed\n", 
				*(unsigned char*)(&client->addr.sin_addr.s_addr), *((unsigned char*)(&client->addr.sin_addr.s_addr)+1),													
				*((unsigned char*)(&client->addr.sin_addr.s_addr)+2), *((unsigned char*)(&client->addr.sin_addr.s_addr)+3),													
				client->addr.sin_port, *(unsigned char*)(&cliValue->addr.sin_addr.s_addr), *((unsigned char*)(&cliValue->addr.sin_addr.s_addr)+1),													
				*((unsigned char*)(&cliValue->addr.sin_addr.s_addr)+2), *((unsigned char*)(&cliValue->addr.sin_addr.s_addr)+3),													
				cliValue->addr.sin_port);
#endif
			struct ev_loop *loop = ntyTcpServerGetMainloop();
#if 1 //Release Before Node
			if(0 == ntyReleaseClientNodeHashTable(&pClient->addr)) { //release before hash node
				ntylog(" ntyLoginPacketHandleRequest --> Ip have changed and release hash table\n");
			}
			if(0 == ntyReleaseClientNodeSocket(loop, pClient->watcher, pClient->sockfd)) {
				ntylog(" ntyLoginPacketHandleRequest --> Ip have changed and relase client socket\n");
			}
#endif

			pClient->sockfd = client->sockfd;
			pClient->clientType = client->clientType;
			pClient->addr.sin_addr.s_addr = client->addr.sin_addr.s_addr;
			pClient->addr.sin_port = client->addr.sin_port;
			pClient->ackNum = ackNum;
			pClient->devId = key;
			
#if ENABLE_NATTY_TIME_STAMP //TIME Stamp 	
			pthread_mutex_lock(&time_mutex);
			pClient->stamp = time(NULL);
			pthread_mutex_unlock(&time_mutex);
			if (client->clientType == PROTO_TYPE_TCP) {
				pClient->watcher = client->watcher;
			} else {
				pClient->watcher = NULL;
			}
#endif
			
			//ntyClientFriendsList(pClient, ack);

			//friends is null and update friends tree
			if (pClient->friends != NULL) {
				if (ntyFriendsTreeIsEmpty(pClient->friends)) {
#if 1	//just for debug		
					if (key == 1) {
						ntyFriendsTreeInsert(pClient->friends, 2);
						ntyFriendsTreeInsert(pClient->friends, 3);
					} else if (key == 2) {
						ntyFriendsTreeInsert(pClient->friends, 1);
					} else if (key == 3) {
						ntyFriendsTreeInsert(pClient->friends, 1);
					}
#endif				
				}	
			} else {
				pClient->friends = ntyFriendsTreeInstance();
#if 1	//just for debug		
				if (key == 1) {
					ntyFriendsTreeInsert(pClient->friends, 2);
					ntyFriendsTreeInsert(pClient->friends, 3);
				} else if (key == 2) {
					ntyFriendsTreeInsert(pClient->friends, 1);
				} else if (key == 3) {
					ntyFriendsTreeInsert(pClient->friends, 1);
				}
#endif

#if ENABLE_CONNECTION_POOL
				if (buffer[NEY_PROTO_VERSION_IDX] == NTY_PROTO_APP_VERSION) { //App
					if(-1 == ntyQueryWatchIDListSelectHandle(key, pClient->friends)) {
						ntylog(" ntyQueryWatchIDListSelectHandle Failed \n");
					}
				} else if (buffer[NEY_PROTO_VERSION_IDX] == NTY_PROTO_DEVICE_VERSION) { //Device
					if (-1 == ntyQueryAppIDListSelectHandle(key, pClient->friends)) {
						ntylog(" ntyQueryAppIDListSelectHandle Failed \n");
					}
				} else {
					ntylog(" Protocol Version is Error : %c\n", buffer[NEY_PROTO_VERSION_IDX]);
					//free(pClient);
					//return ;
				}
#endif

			}

#if 1 //Insert Hash table
			ntylog("Insert Hash Table\n");
			int ret = ntyInsertNodeToHashTable(&pClient->addr, key);
			if (ret == 0) { 			
				ntylog("Hash Table Node Insert Success");
			}
#endif

			//ntylog(" keys : %d");
			//notify friends 
			//ntyFriendsTreeTraversalNotify(pClient->friends, key, ntyNotifyFriendMessage);
			//send friends list to client
			ntySendFriendsTreeIpAddr(pClient, 1);

			ntySendDeviceTimeCheckAck(pClient, ackNum+1);

			return ;
		}
		//ntylog("Login deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);		
		//send login ack
#if 0
		if (client->clientType == PROTO_TYPE_TCP) {
			ack[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
			ack[NTY_PROTO_MESSAGE_TYPE] = (U8)MSG_ACK;
			ack[NTY_PROTO_TYPE_IDX] = NTY_PROTO_LOGIN_ACK;
			//memcpy(ack+NTY_PROTO_LOGIN_ACK_ACKNUM_IDX, &ackNum, NTY_ACKNUM_LENGTH);
			*(U32*)(&ack[NTY_PROTO_ACKNUM_IDX]) = ackNum;
			ntySendBuffer(client, ack, length);
		}
#endif
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

		void *pRBTree = ntyRBTreeInstance();
		C_DEVID key = *(C_DEVID*)(buffer+NTY_PROTO_HEARTBEAT_DEVID_IDX);
		U32 ackNum = *(U32*)(buffer+NTY_PROTO_HEARTBEAT_ACKNUM_IDX)+1;
		const UdpClient *client = obj;
		U8 ack[NTY_HEARTBEAT_ACK_LENGTH] = {0};
		
		UdpClient *cliValue = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, key);
		if (cliValue == NULL) {			
			UdpClient *pClient = (UdpClient*)malloc(sizeof(UdpClient));
			pClient->sockfd = client->sockfd;
			pClient->clientType = client->clientType;
			pClient->addr.sin_addr.s_addr = client->addr.sin_addr.s_addr;
			pClient->addr.sin_port = client->addr.sin_port;
			pClient->ackNum = ackNum;
			pClient->devId = key;
			
#if ENABLE_NATTY_TIME_STAMP //TIME Stamp 	
			pthread_mutex_lock(&time_mutex);
			pClient->stamp = time(NULL);
			pthread_mutex_unlock(&time_mutex);
			if (client->clientType == PROTO_TYPE_TCP) {
				pClient->watcher = client->watcher;
			} else {
				pClient->watcher = NULL;
			}
#endif

			//new firends tree			
			pClient->friends = ntyFriendsTreeInstance();
			//determine this key is available and read from disk friends 

#if 1	//just for debug		
			//if (key != 1 || key != 2) return ;
			if (key == 1) {
				ntyFriendsTreeInsert(pClient->friends, 2);
				ntyFriendsTreeInsert(pClient->friends, 3);
			} else if (key == 2) {
				ntyFriendsTreeInsert(pClient->friends, 1);
			} else if (key == 3) {
				ntyFriendsTreeInsert(pClient->friends, 1);
			}
#endif	

#if ENABLE_CONNECTION_POOL
			if (buffer[NEY_PROTO_VERSION_IDX] == NTY_PROTO_APP_VERSION) { //App
				if(-1 == ntyQueryWatchIDListSelectHandle(key, pClient->friends)) {
					ntylog(" ntyQueryWatchIDListSelectHandle Failed \n");
				}
			} else if (buffer[NEY_PROTO_VERSION_IDX] == NTY_PROTO_DEVICE_VERSION) { //Device
				if (-1 == ntyQueryAppIDListSelectHandle(key, pClient->friends)) {
					ntylog(" ntyQueryAppIDListSelectHandle Failed \n");
				}
			} else {
				ntylog(" Protocol Version is Error : %c\n", buffer[NEY_PROTO_VERSION_IDX]);
				//free(pClient);
				//return ;
			}
#endif

			ntylog("ntyHeartBeatPacketHandleRequest --> Insert New Client Node\n");
			//insert rb-tree
			if (ntyRBTreeInterfaceInsert(pRBTree, key, pClient)) {
				ntylog("Client is Exist\n");
				free(pClient);
				return ;
			}
			
			//notify friends 
			//ntyFriendsTreeTraversal(pClient->friends, ntyNotifyFriendConnect);
			//ntyFriendsTreeTraversalNotify(pClient->friends, key, ntyNotifyFriendMessage);
#if 1 //Insert Hash table
			ntylog("Insert Hash Table\n");
			int ret = ntyInsertNodeToHashTable(&pClient->addr, key);
			if (ret == 0) { 			
				ntylog("Hash Table Node Insert Success");
			}
#endif

			//send friends list to client 
			//ntyFriendsTreeTraversal(pClient->friends, ntySendFriendIpAddr);
			ntySendFriendsTreeIpAddr(pClient, 0);
			
			return ;
		} else if ((cliValue != NULL) && !ntyClientCompare(client, cliValue)) {
			int i = 0, length = 0;
			UdpClient *pClient = cliValue;//(UdpClient*)malloc(sizeof(UdpClient));
#if 0
			ntylog(" ntyHeartBeatPacketHandleRequest --> client: %d.%d.%d.%d:%d --> cliValue: %d.%d.%d.%d:%d, ip have changed\n", 
				*(unsigned char*)(&client->addr.sin_addr.s_addr), *((unsigned char*)(&client->addr.sin_addr.s_addr)+1),													
				*((unsigned char*)(&client->addr.sin_addr.s_addr)+2), *((unsigned char*)(&client->addr.sin_addr.s_addr)+3),													
				client->addr.sin_port, *(unsigned char*)(&cliValue->addr.sin_addr.s_addr), *((unsigned char*)(&cliValue->addr.sin_addr.s_addr)+1),													
				*((unsigned char*)(&cliValue->addr.sin_addr.s_addr)+2), *((unsigned char*)(&cliValue->addr.sin_addr.s_addr)+3),													
				cliValue->addr.sin_port);
#endif			
#if 1 //Release Before Node
			struct ev_loop *loop = ntyTcpServerGetMainloop();
			if(0 == ntyReleaseClientNodeHashTable(&pClient->addr)) { //release before hash node
				ntylog(" ntyHeartBeatPacketHandleRequest --> Ip have changed and release hash table\n");
			}
			if(0 == ntyReleaseClientNodeSocket(loop, pClient->watcher, pClient->sockfd)) {
				ntylog(" ntyHeartBeatPacketHandleRequest --> Ip have changed and relase client socket\n");
			}
#endif

			pClient->sockfd = client->sockfd;
			pClient->clientType = client->clientType;
			pClient->addr.sin_addr.s_addr = client->addr.sin_addr.s_addr;
			pClient->addr.sin_port = client->addr.sin_port;
			pClient->ackNum = ackNum;
			pClient->devId = key;
			
#if ENABLE_NATTY_TIME_STAMP //TIME Stamp 	
			pthread_mutex_lock(&time_mutex);
			pClient->stamp = time(NULL);
			pthread_mutex_unlock(&time_mutex);
			if (client->clientType == PROTO_TYPE_TCP) {
				pClient->watcher = client->watcher;
			} else {
				pClient->watcher = NULL;
			}
#endif
			
			ntylog("HeartBeat Update Info: %d\n", buffer[NTY_PROTO_TYPE_IDX]);

			//friends is null and update friends tree
			if (pClient->friends != NULL) {
				if (ntyFriendsTreeIsEmpty(pClient->friends)) {
#if 1	//just for debug		
					if (key == 1) {
						ntyFriendsTreeInsert(pClient->friends, 2);
						ntyFriendsTreeInsert(pClient->friends, 3);
					} else if (key == 2) {
						ntyFriendsTreeInsert(pClient->friends, 1);
					} else if (key == 3) {
						ntyFriendsTreeInsert(pClient->friends, 1);
					}
#endif				
				}	
			} else {
				pClient->friends = ntyFriendsTreeInstance();
#if 1	//just for debug		
				if (key == 1) {
					ntyFriendsTreeInsert(pClient->friends, 2);
					ntyFriendsTreeInsert(pClient->friends, 3);
				} else if (key == 2) {
					ntyFriendsTreeInsert(pClient->friends, 1);
				} else if (key == 3) {
					ntyFriendsTreeInsert(pClient->friends, 1);
				}
#endif

#if ENABLE_CONNECTION_POOL
				if (buffer[NEY_PROTO_VERSION_IDX] == NTY_PROTO_APP_VERSION) { //App
					if(-1 == ntyQueryWatchIDListSelectHandle(key, pClient->friends)) {
						ntylog(" ntyQueryWatchIDListSelectHandle Failed \n");
					}
				} else if (buffer[NEY_PROTO_VERSION_IDX] == NTY_PROTO_DEVICE_VERSION) { //Device
					if (-1 == ntyQueryAppIDListSelectHandle(key, pClient->friends)) {
						ntylog(" ntyQueryAppIDListSelectHandle Failed \n");
					}
				} else {
					ntylog(" Protocol Version is Error : %c\n", buffer[NEY_PROTO_VERSION_IDX]);
					//free(pClient);
					//return ;
				}
#endif


			}

#if 1 //Insert Hash table
			ntylog("Insert Hash Table\n");
			int ret = ntyInsertNodeToHashTable(&pClient->addr, key);
			if (ret == 0) { 			
				ntylog("Hash Table Node Insert Success");
			}
#endif


			//client dev ipaddr is changed
			//notify its friends list, reset p2p
			//ntyFriendsTreeTraversalNotify(pClient->friends, key, ntyNotifyFriendMessage);
			//
			//send friends list to client 
			//ntyFriendsTreeTraversal(pClient->friends, ntySendFriendIpAddr);
			ntySendFriendsTreeIpAddr(pClient, 0);
			return ;
		}
		
		ntylog("HeartBeat deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
		
#if ENABLE_NATTY_TIME_STAMP //TIME Stamp 	
		pthread_mutex_lock(&time_mutex);
		cliValue->stamp = time(NULL);
		pthread_mutex_unlock(&time_mutex);
		if (client->clientType == PROTO_TYPE_TCP) {
			cliValue->watcher = client->watcher;
		} else {
			cliValue->watcher = NULL;
		}
#endif
		
		//send heartbeat ack
		if (client->clientType == PROTO_TYPE_UDP) {
			ack[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
			ack[NTY_PROTO_MESSAGE_TYPE] = (U8)MSG_ACK;
			ack[NTY_PROTO_TYPE_IDX] = NTY_PROTO_HEARTBEAT_ACK;
			*(U32*)(&ack[NTY_PROTO_ACKNUM_IDX]) = ackNum;
			*(U32*)(&ack[NTY_PROTO_LOGIN_REQ_CRC_IDX]) = ntyGenCrcValue(ack, NTY_PROTO_LOGIN_REQ_CRC_IDX);
			ntySendBuffer(client, ack, NTY_PROTO_LOGIN_REQ_CRC_IDX+sizeof(U32));
		}
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
		void *pRBTree = ntyRBTreeInstance();
		C_DEVID key = *(C_DEVID*)(buffer+NTY_PROTO_HEARTBEAT_DEVID_IDX);
		U32 ackNum = *(U32*)(buffer+NTY_PROTO_HEARTBEAT_ACKNUM_IDX)+1;
		const UdpClient *client = obj;
		unsigned char ack[NTY_HEARTBEAT_ACK_LENGTH] = {0};


		UdpClient *cliValue = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, key);
		if (cliValue != NULL) {
			Delete(cliValue->friends);
		}
		
		ntyRBTreeInterfaceDelete(pRBTree, key);
		ntylog("Logout deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);

		//send logout ack
		ack[NTY_PROTO_TYPE_IDX] = NTY_PROTO_LOGOUT_ACK;
		memcpy(ack+1, &ackNum, NTY_ACKNUM_LENGTH);
		ntySendBuffer(client, ack, NTY_LOGOUT_ACK_LENGTH);
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
#if 0
		U8 data[RECV_BUFFER_SIZE] = {0};
#else
		U8 *data = (U8*)malloc(RECV_BUFFER_SIZE*sizeof(U8));
#endif		
		C_DEVID destDevId = *(C_DEVID*)(&buffer[NTY_PROTO_DATAPACKET_DEST_DEVID_IDX]);
		void *pRBTree = ntyRBTreeInstance();
		UdpClient *destClient = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, destDevId);
		const UdpClient *client = obj;
		
		U16 recByteCount = *(U16*)(&buffer[NTY_PROTO_DATAPACKET_CONTENT_COUNT_IDX]);
#if 1
		Client *selfNode = (Client*)ntyRBTreeInterfaceSearch(pRBTree, client->devId);
		if (selfNode == NULL) { //devid not exist
			ntylog(" ntyUserDataPacketHandleRequest --> selfNode:%lld is not Exist\n", client->devId);

			free(data);
			return ;
		}
#if ENABLE_NATTY_TIME_STAMP //TIME Stamp 	
		pthread_mutex_lock(&time_mutex);
		selfNode->stamp = time(NULL);
		pthread_mutex_unlock(&time_mutex);
		//pClient->watcher = client->watcher;
#endif
#endif
		memcpy(data, buffer+NTY_PROTO_DATAPACKET_CONTENT_IDX, recByteCount);
		ntylog("data : %s\n", data);

		if (destClient == NULL) {
			if (destDevId == 0x0) { //destDevId == 0x0 boardcast all client
				int ret = -1;
				ntylog("destClient broadcast :%d, id:%lld\n", length, client->devId);
				//C_DEVID selfDevId = *(C_DEVID*)(&buffer[NTY_PROTO_DEVID_IDX]);
				buffer[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_RET;
				if (0 > (ret = ntyClassifyMessageType(client->devId, 0x0, data, recByteCount))) {
					ntylog(" ret : %d\n", ret);
					#if 0 //boardcast all client
					ntySendBuffer(client, buffer, length);
					#else
					ntyBoardcastAllFriends(client, data, recByteCount);
					#endif
				}
			} else {
				ntylog("destClient is no exist :%d, id:%lld\n", length, client->devId);
				ntySendBuffer(client, buffer, length);
			}
		} else {
			int ret = -1;
			ntylog(" Proxy data: %s, destDevId:%lld %d.%d.%d.%d:%d\n", data, destDevId, *(unsigned char*)(&destClient->addr.sin_addr.s_addr),
				*((unsigned char*)(&destClient->addr.sin_addr.s_addr)+1), *((unsigned char*)(&destClient->addr.sin_addr.s_addr)+2), 
				*((unsigned char*)(&destClient->addr.sin_addr.s_addr)+3), destClient->addr.sin_port);
		
			if (0 > (ret = ntyClassifyMessageType(client->devId, destClient->devId, data, recByteCount))) {
				ntylog(" ret : %d\n", ret);
				ntySendBuffer(destClient, buffer, length);
			}

		}

#if ENABLE_NODE_AGENT_SAVE //devId , data, recByteCount, token is set
		ntyNodeAgentProcess(data, recByteCount, selfNode->devId);
#endif

		free(data);
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
	}
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

	ntySetSuccessor(pHeartBeatFilter, pLoginFilter);
	ntySetSuccessor(pLoginFilter, pLogoutFilter);
	ntySetSuccessor(pLogoutFilter, pP2PAddrReqFilter);
	ntySetSuccessor(pP2PAddrReqFilter, pUserDataPacketFilter);
	
	ntySetSuccessor(pUserDataPacketFilter, pUserDataPacketAckFilter);
	ntySetSuccessor(pUserDataPacketAckFilter, pP2PConnectNotifyPacketFilter);
	ntySetSuccessor(pP2PConnectNotifyPacketFilter, pTimeCheckFilter);
	ntySetSuccessor(pTimeCheckFilter, pUnBindDeviceFilter);
	ntySetSuccessor(pUnBindDeviceFilter, pBindDeviceFilter);
	
	ntySetSuccessor(pBindDeviceFilter, NULL);

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

void ntyProtocolFilterProcess(void *_filter, unsigned char *buffer, U32 length, const void *obj) {
	
	//data crc is right, and encryto
	U32 u32Crc = ntyGenCrcValue(buffer, length-4);
	U32 u32ClientCrc = *((U32*)(buffer+length-4));

	//ntydbg(" client:%x, server:%x, length:%d", u32ClientCrc, u32Crc, length);
	if (u32Crc != u32ClientCrc) {
#if 1 
		ntylog(" ntyProtocolFilterProcess --> client:%x, server:%x, length:%d\n", u32ClientCrc, u32Crc, length);
		const Client *client = obj;
		if (client->clientType == PROTO_TYPE_TCP) {
			struct ev_loop *loop = ntyTcpServerGetMainloop();

			ntydbg(" ntyProtocolFilterProcess --> Data Format is Error\n");
			ntyReleaseClientNodeSocket(loop, client->watcher, client->sockfd);
		}
#endif
		return ;
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

int ntyReleaseClientNodeNyNode(struct ev_loop *loop, void *node) {
	Client *client = node;

	int ret = ntyDeleteNodeFromHashTable(&client->addr, client->devId);
	if (ret == -1) {
		ntylog("Delete Node From Hash Table Parameter is Error\n");
		//return -1;
	} else if (ret == -2) {
		ntylog("Hash Table Node is not Exist\n");
	}

	ntyReleaseClientNodeSocket(loop, client->watcher, client->sockfd);

	void *tree = ntyRBTreeInstance();
	ret = ntyRBTreeInterfaceDelete(tree, client->devId);
	if (ret == 1) {
		ntylog(" ntyReleaseClientNodeNyNode --> RBTree Node Not Exist\n");
		return -2;
	} else if (ret == -1) {
		ntylog(" ntyReleaseClientNodeNyNode --> RBTree Node Parameter is Error\n");
		return -3;
	}

	return 0;
}

int ntyReleaseClientNodeByDevID(struct ev_loop *loop, struct sockaddr_in *addr, struct ev_io *watcher, C_DEVID devid) {
	C_DEVID devHashId = ntySearchDevIdFromHashTable(addr);
	if (devHashId != devid) {
		ntylog(" ntyReleaseClientNodeByDevID --> devHashId:%lld, devid:%lld\n", devHashId, devid);
	}

	int ret = ntyDeleteNodeFromHashTable(addr, devid);
	if (ret == -1) {
		ntylog("Delete Node From Hash Table Parameter is Error\n");
		//return -1;
	} else if (ret == -2) {
		ntylog("Hash Table Node is not Exist\n");
	}

	ntyReleaseClientNodeSocket(loop, watcher, watcher->fd);

	void *tree = ntyRBTreeInstance();
	//ntyRBTreeOperaDelete
	// delete rb-tree node
	ret = ntyRBTreeInterfaceDelete(tree, devid);
	if (ret == 1) {
		ntylog(" ntyReleaseClientNodeByDevID --> RBTree Node Not Exist\n");
		return -2;
	} else if (ret == -1) {
		ntylog(" ntyReleaseClientNodeByDevID --> RBTree Node Parameter is Error\n");
		return -3;
	}

	ntylog(" ntyReleaseClientNodeByDevID --> RBTree Delete Node Success\n");

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

