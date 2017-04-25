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
#include "NattyHBD.h"
#include "NattyVector.h"
#include "NattyJson.h"
#include "time.h"
#include "NattyServAction.h"
#include "NattyMessage.h"
#include "NattyDaveMQ.h"
#include "NattyUdpServer.h"

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

extern void *ntyTcpServerGetMainloop(void);
extern int ntyReleaseSocket(struct ev_loop *loop, struct ev_io *watcher);


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
			//*(C_DEVID*)(&ackNotify[NTY_PROTO_P2P_NOTIFY_DEVID_IDX]) = key;
			//*(U32*)(&ackNotify[NTY_PROTO_P2P_NOTIFY_ACKNUM_IDX]) = ackNum;
#if 0			
			*(U32*)(&ackNotify[NTY_PROTO_P2P_NOTIFY_IPADDR_IDX]) = client->addr.sin_addr.s_addr;
			*(U16*)(&ackNotify[NTY_PROTO_P2P_NOTIFY_IPPORT_IDX]) = client->addr.sin_port;
#endif
			//ntyP2PNotifyClient(cv, ackNotify);
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



/*
 * add rbtree <key, value> -- <UserId, sockfd>
 * add B+tree <key, value> -- <UserId, UserInfo>
 */
Client* ntyAddClientHeap(const void * obj, int *result) {
	const Client *client = obj;
	int ret = -1;

	//pClient->
	BPTreeHeap *heap = ntyBHeapInstance();
	if (heap == NULL) return NULL;
	NRecord *record = ntyBHeapSelect(heap, client->devId);
	if (record == NULL) {
		Client *pClient = (Client*)malloc(sizeof(Client));
		if (pClient == NULL) {
			*result = NTY_RESULT_ERROR;
			return NULL;
		}
		memset(pClient, 0, sizeof(Client));		
		memcpy(pClient, client, sizeof(Client));
		ntylog("ntyAddClientHeap is not exist %lld\n", client->devId);

		//insert bheap
		ret = ntyBHeapInsert(heap, client->devId, pClient);
#if 0
		if (ret == NTY_RESULT_EXIST || ret == NTY_RESULT_FAILED) {
			ntylog("ret : %d\n", ret);
			free(pClient);
			
			*result = ret;
			return NULL;
		} else if (ret == NTY_RESULT_BUSY) {
			ntylog("ret : %d\n", ret);
			free(pClient);

			*result = ret;
			return NULL;
		}
#else
		if (ret != NTY_RESULT_SUCCESS) {
			ntylog("ret : %d\n", ret);
			free(pClient);

			*result = ret;
			return NULL;
		}
		ntyPrintTree(heap->root);
#endif
		pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;
		memcpy(&pClient->bMutex, &blank_mutex, sizeof(pClient->bMutex));
		
		pClient->rLength = 0;
		pClient->recvBuffer = malloc(PACKET_BUFFER_SIZE);
		if (pClient->recvBuffer == NULL) {
			free(pClient);
			return NULL;
		}
		memset(pClient->recvBuffer, 0, PACKET_BUFFER_SIZE);

		//voice buffer
		pClient->rBuffer = NULL;
		pClient->sBuffer = NULL;

#if ENABLE_NATTY_TIME_STAMP //TIME Stamp 	
		pClient->stamp = ntyTimeStampGenrator();
#endif
		
		ntylog(" ntyAddClientHeap --> ntyVectorCreator \n");
		if (pClient->friends == NULL) {
			pClient->friends = ntyVectorCreator();
			if (pClient->friends == NULL) {
				*result = NTY_RESULT_ERROR;
				return pClient;
			}

			ntylog("ntyAddClientHeap --> friend addr:%llx\n", (C_DEVID)pClient->friends);
#if ENABLE_CONNECTION_POOL
			if (pClient->deviceType == NTY_PROTO_CLIENT_ANDROID 
				|| pClient->deviceType == NTY_PROTO_CLIENT_IOS) { //App
				if(-1 == ntyQueryWatchIDListSelectHandle(pClient->devId, pClient->friends)) {
					ntylog(" ntyQueryWatchIDListSelectHandle Failed \n");
					
				}
			} else if (pClient->deviceType == NTY_PROTO_CLIENT_WATCH) { //Device
				if (-1 == ntyQueryAppIDListSelectHandle(pClient->devId, pClient->friends)) {
					ntylog(" ntyQueryAppIDListSelectHandle Failed \n");
					
				}
			} else {
				ntylog(" Protocol Device Type is Error : %c\n", pClient->deviceType);
				//free(pClient);
				//return ;
			}
#endif
		}

		if (pClient->group == NULL) {
#if 1 //Add Groups
#endif
		}
#if 0 //cancel timer
		//start timer,
		NWTimer* nwTimer = ntyTimerInstance();
		unsigned long addr = (unsigned long)pClient;
		void* timer = ntyTimerAdd(nwTimer, 60, ntyCheckOnlineAlarmNotify, (void*)&addr, sizeof(unsigned long));
		pClient->hbdTimer = timer;
#endif
		*result = NTY_RESULT_SUCCESS;
		return pClient;
	} else {
	
		ntylog("ntyAddClientHeap exist %lld\n", client->devId);
	
		Client *pClient = record->value;
		if (pClient == NULL) {
			ntylog(" ntyAddClientHeap pClient is not Exist : %lld\n", client->devId);

			*result = NTY_RESULT_NOEXIST;
			return NULL;
		}
#if ENABLE_NATTY_TIME_STAMP //TIME Stamp 	
		pClient->stamp = ntyTimeStampGenrator();
#endif
		pClient->online = 1;

		return pClient;
	}

	return NULL;
}

int ntyDelClientHeap(C_DEVID clientId) {
	int ret = -1;

	void *heap = ntyBHeapInstance();
	NRecord *record = ntyBHeapSelect(heap, clientId);
	if (record != NULL) {
		Client *pClient = record->value;
		if (pClient == NULL) {
			ntylog("ntyDelClientHeap pClient == NULL : %lld\n", clientId);
			return NTY_RESULT_NOEXIST;
		}

		pClient->rLength = 0;
		if (pClient->recvBuffer != NULL) {
			free(pClient->recvBuffer);
		}
#if 0 //cancel timer
		NWTimer* nwTimer = ntyTimerInstance();
		if (pClient->hbdTimer != NULL) {
			ntyTimerDel(nwTimer, pClient->hbdTimer);
			pClient->hbdTimer = NULL;
		}
#endif
		if (pClient->friends != NULL) {
			ntyVectorDestory(pClient->friends);
		}
#if 0 //release  groups
		pClient->group
#endif

		ret = ntyBHeapDelete(heap, clientId);
		if (ret == NTY_RESULT_FAILED) {
			ASSERT(0);
		} else if (ret == NTY_RESULT_NOEXIST) {
			ntylog("ntyDelClientHeap Delete Error\n");
		}
		free(pClient);
	} else {
		return NTY_RESULT_NOEXIST;
	}

	return NTY_RESULT_SUCCESS;
}

/*
 * Release Client
 * 1. HashTable	Hash
 * 2. HashMap  	RBTree
 * 3. BHeap 	BPlusTree
 */

int ntyClientCleanup(ClientSocket *client) { //
	if (client == NULL) return NTY_RESULT_ERROR;
	if (client->watcher == NULL) return NTY_RESULT_ERROR;
	int sockfd = client->watcher->fd;

	void *hash = ntyHashTableInstance();
	Payload *load = ntyHashTableSearch(hash, sockfd);
	if (load == NULL) return NTY_RESULT_NOEXIST;

	C_DEVID Id = load->id;
	ntylog(" ntyClientCleanup --> %lld\n", Id);
	//delete hash key 
	int ret = ntyHashTableDelete(hash, sockfd);
	if (ret == NTY_RESULT_SUCCESS) {
		//release client socket map
		ret = ntyDelRelationMap(Id);
		ntylog(" ntyMapDelete --> ret : %d\n", ret);
		
		//release HashMap
#if 0 //no delete from BHeap
		ret = ntyDelClientHeap(Id);
		ntylog(" ntyMapDelete --> ret : %d\n", ret);
#endif
	} else {
		ntylog(" ntyHashDelete ret : %d\n", ret);
	}

}


int ntyOfflineClientHeap(C_DEVID clientId) {
	BPTreeHeap *heap = ntyBHeapInstance();
	//ASSERT(heap != NULL);
	NRecord *record = ntyBHeapSelect(heap, clientId);

	if (record == NULL) {
		ntylog("Error OfflineClientHeap is not Exist %lld\n", clientId);
		ntyPrintTree(heap->root);
		return NTY_RESULT_NOEXIST;
	}

	Client* pClient = (Client*)record->value;
	if (pClient == NULL) {
		ntylog("Error OfflineClientHeap record->value == NULL %lld\n", clientId);
		ntyPrintTree(heap->root);
		return NTY_RESULT_NOEXIST;
	}

	pClient->online = 0;
	return NTY_RESULT_SUCCESS;
}

int ntyOnlineClientHeap(C_DEVID clientId) {
	BPTreeHeap *heap = ntyBHeapInstance();
	//ASSERT(heap != NULL);
	NRecord *record = ntyBHeapSelect(heap, clientId);

	if (record == NULL) {
		ntylog(" Error OfflineClientHeap is not Exist : %lld\n", clientId);
		ntyPrintTree(heap->root);
		return NTY_RESULT_NOEXIST;
	}

	Client* pClient = (Client*)record->value;
	if (pClient == NULL) {
		ntylog(" Error OfflineClientHeap record->value == NULL %lld\n", clientId);
		ntyPrintTree(heap->root);
		return NTY_RESULT_NOEXIST;
	}
	
#if ENABLE_NATTY_TIME_STAMP //TIME Stamp 	
	pClient->stamp = ntyTimeStampGenrator();
#endif
	pClient->online = 1;

	return NTY_RESULT_SUCCESS;
}

//int ntyUpdateClientInfo()

/*
 * Login Packet
 * Login Packet, HeartBeatPacket, LogoutPacket etc. should use templete designer pattern
 */
void ntyLoginPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_LOGIN_REQ) {
		
		ntylog("====================begin ntyLoginPacketHandleRequest action ==========================\n");

#if 0
		int i = 0;
		void *pRBTree = ntyRBTreeInstance();
		
		ntyAddClientNodeToRBTree(buffer, length, obj);
		Client *pClient = (Client*)ntyRBTreeInterfaceSearch(pRBTree, client->devId);
		if (pClient != NULL) {
			ntySendFriendsTreeIpAddr(pClient, 1);

			ntylog(" Buffer Version : %x\n", buffer[NEY_PROTO_VERSION_IDX]);
			if (buffer[NEY_PROTO_VERSION_IDX] == NTY_PROTO_DEVICE_VERSION)
				ntySendDeviceTimeCheckAck(pClient->devId, client->ackNum+1);
		}
#else

		const MessagePacket *msg = (const MessagePacket*)obj;
		if (msg == NULL) return ;
		const Client *client = msg->client;

		MessagePacket *pMsg = (MessagePacket *)malloc(sizeof(MessagePacket));
		if (pMsg == NULL) return ;
		
		memset(pMsg, 0, sizeof(MessagePacket));		
		memcpy(pMsg, msg, sizeof(MessagePacket));
		
		ntyAddRelationMap(pMsg);
		free(pMsg);

		int ret = NTY_RESULT_SUCCESS;
		Client *pClient = ntyAddClientHeap(client, &ret);
		if (pClient != NULL) {
			//ntySendFriendsTreeIpAddr(pClient, 1);

			if (pClient->deviceType == NTY_PROTO_CLIENT_WATCH) {
#if 0
				ntySendDeviceTimeCheckAck(pClient, client->ackNum+1);
#elif 0
				ntySendDeviceTimeCheckAck(pClient, 1);
#else	

				//将此处的添加到队列里面
				ntyExecuteChangeDeviceOnlineStatusHandle(pClient->devId);

				//ntySendLoginAckResult(pClient->devId, "", 0, 200);
				ntySendDeviceTimeCheckAck(pClient->devId, 1);

				VALUE_TYPE *tag = malloc(sizeof(VALUE_TYPE));
				if (tag == NULL) return ;

				memset(tag, 0, sizeof(VALUE_TYPE));
				tag->fromId = pClient->devId;
				tag->Type = MSG_TYPE_OFFLINE_MSG_REQ_HANDLE;
				tag->cb = ntyDeviceOfflineMsgReqHandle;

				ntyDaveMqPushMessage(tag);
#endif
			} else {

				if (pClient->deviceType == NTY_PROTO_CLIENT_IOS) {
					U16 tokenLen = *(U16*)(buffer+NTY_PROTO_LOGIN_REQ_JSON_LENGTH_IDX);
					U8 *token = buffer+NTY_PROTO_LOGIN_REQ_JSON_CONTENT_IDX;

					if (pClient->token == NULL) {
						pClient->token = malloc(tokenLen + 1);
					}

					if (pClient->token != NULL) {
						memset(pClient->token, 0, tokenLen + 1);

						memcpy(pClient->token, token, tokenLen);
						ntylog(" LOGIN --> %s\n", pClient->token);
					}
				}

				VALUE_TYPE *tag = malloc(sizeof(VALUE_TYPE));
				if (tag == NULL) return ;

				memset(tag, 0, sizeof(VALUE_TYPE));
				tag->fromId = pClient->devId;
				tag->Type = MSG_TYPE_OFFLINE_MSG_REQ_HANDLE;
				tag->cb = ntyOfflineMsgReqHandle;

				ntyDaveMqPushMessage(tag);
				
			}
		} else {	
//			ASSERT(0);
			ntylog(" BHeap in the Processs\n");
			if (ret == NTY_RESULT_BUSY) {
				ntyJsonCommonResult(client->devId, NATTY_RESULT_CODE_BUSY);
			}
		}
#endif
		//ntylog("Login deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);		
		//send login ack
		
		ntylog("====================end ntyLoginPacketHandleRequest action ==========================\n");
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);
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
	
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_HEARTBEAT_REQ) {
		ntylog("====================begin ntyHeartBeatPacketHandleRequest action ==========================\n");
		const MessagePacket *msg = (const MessagePacket*)obj;
		if (msg == NULL) return ;
		const Client *client = msg->client;

		MessagePacket *pMsg = (MessagePacket *)malloc(sizeof(MessagePacket));
		if (pMsg == NULL) {
			ntylog(" %s --> malloc failed MessagePacket. \n", __func__);
			return;
		}
		memset(pMsg, 0, sizeof(MessagePacket));
		memcpy(pMsg, msg, sizeof(MessagePacket));
		
		ntyAddRelationMap(pMsg);
		free(pMsg);
		

		C_DEVID fromId = *(C_DEVID*)(buffer+NTY_PROTO_HEARTBEAT_REQ_DEVID_IDX);
#if 1
		int ret = NTY_RESULT_SUCCESS;

		Client *pClient = ntyAddClientHeap(client, &ret);
		if (pClient == NULL) {
			ntylog(" ntyHeartBeatPacketHandleRequest --> Error\n");
		}
#else
		ntyOnlineClientHeap(client->devId);
#endif
		ntySendHeartBeatResult(fromId);
		
		ntylog("====================end ntyHeartBeatPacketHandleRequest action ==========================\n");
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);
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
	
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_LOGOUT_REQ) {
		ntylog("====================begin ntyLogoutPacketHandleRequest action ==========================\n");
		const MessagePacket *msg = (const MessagePacket*)obj;
		if (msg == NULL) return ;
		const Client *client = msg->client;
#if 0 //offline
		ntyDelClientHeap(client->devId);
#else
		ntyOfflineClientHeap(client->devId);
#endif
		ntylog("====================end ntyLogoutPacketHandleRequest action ==========================\n");
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);
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


void ntyTimeCheckHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_TIME_CHECK_REQ) {
		ntylog("====================begin ntyTimeCheckHandleRequest action ==========================\n");
		const MessagePacket *msg = (const MessagePacket*)obj;
		if (msg == NULL) return ;
		const Client *client = msg->client;
#if 0
		C_DEVID key = *(C_DEVID*)(buffer+NTY_PROTO_LOGIN_REQ_DEVID_IDX);
		U32 ackNum = *(U32*)(buffer+NTY_PROTO_LOGIN_REQ_ACKNUM_IDX)+1;
		ntySendDeviceTimeCheckAck(client, ackNum);
#else

		ntySendDeviceTimeCheckAck(client->devId, 1);
#endif
		ntylog("====================end ntyTimeCheckHandleRequest action ==========================\n");
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);
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


void ntyICCIDReqPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_ICCID_REQ) {
		ntylog("====================begin ntyICCIDReqPacketHandleRequest action ==========================\n");

		const MessagePacket *msg = (const MessagePacket*)obj;
		if (msg == NULL) return ;
		const Client *client = msg->client;

		C_DEVID fromId = *(C_DEVID*)(buffer+NTY_PROTO_ICCID_REQ_DEVID_IDX);

		U16 jsonlen = 0;
		memcpy(&jsonlen, buffer+NTY_PROTO_ICCID_REQ_JSON_LENGTH_IDX, NTY_JSON_COUNT_LENGTH);
		char *jsonstring = malloc(jsonlen+1);
		if (jsonstring == NULL) {
			ntylog(" %s --> malloc failed jsonstring. \n", __func__);
			return;
		}
		
		memset(jsonstring, 0, jsonlen+1);
		memcpy(jsonstring, buffer+NTY_PROTO_ICCID_REQ_JSON_CONTENT_IDX, jsonlen);

		JSON_Value *json = ntyMallocJsonValue(jsonstring);
		if (json == NULL) {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_JSON_FORMAT);
		} else {
			size_t len_ActionParam = sizeof(ActionParam);
			ActionParam *pActionParam = malloc(len_ActionParam);
			if (pActionParam == NULL) {
				ntylog(" %s --> malloc failed ActionParam. \n", __func__);
				return;
			}
			memset(pActionParam, 0, len_ActionParam);
			
			pActionParam->fromId = fromId;
			pActionParam->toId = fromId;
			pActionParam->json = json;
			pActionParam->jsonstring = jsonstring;
			pActionParam->jsonlen = jsonlen;
			pActionParam->index = 0;
			ntyJsonICCIDAction(pActionParam);
			
			free(pActionParam);
		}
		ntyFreeJsonValue(json);
		free(jsonstring);

		ntylog("====================end ntyICCIDReqPacketHandleRequest action ==========================\n");
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);
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

void ntyVoiceReqPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_VOICE_REQ) {
		ntylog("====================end ntyVoiceReqPacketHandleRequest action ==========================\n");

		const MessagePacket *msg = (const MessagePacket*)obj;
		if (msg == NULL) return ;
		const Client *client = msg->client;
		
		C_DEVID fromId = *(C_DEVID*)(buffer+NTY_PROTO_VOICE_REQ_DEVID_IDX);
		U32 msgId = *(U32*)(buffer+NTY_PROTO_VOICE_REQ_MSGID_IDX);

		ntylog(" ntyVoiceReqPacketHandleRequest --> %lld, msgId:%d\n", fromId, msgId);
#if 0
		ntyVoiceReqAction(fromId, msgId);
#else
		VALUE_TYPE *tag = malloc(sizeof(VALUE_TYPE));
		if (tag == NULL) return ;

		memset(tag, 0, sizeof(VALUE_TYPE));
		tag->fromId = fromId;
		tag->arg = msgId;
		tag->Type = MSG_TYPE_VOICE_REQ_HANDLE;
		tag->cb = ntyVoiceReqHandle;

		ntyDaveMqPushMessage(tag);
#endif
		ntylog("====================end ntyVoiceReqPacketHandleRequest action ==========================\n");
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);
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
	
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_VOICE_ACK) {
		ntylog("====================begin ntyVoiceAckPacketHandleRequest action ==========================\n");

		const MessagePacket *msg = (const MessagePacket*)obj;
		if (msg == NULL) return ;
		const Client *client = msg->client;

		C_DEVID fromId = *(C_DEVID*)(buffer+NTY_PROTO_VOICE_ACK_DEVID_IDX);
		U32 msgId = *(U32*)(buffer+NTY_PROTO_VOICE_ACK_MSGID_IDX);

		ntylog(" ntyVoiceAckPacketHandleRequest --> %lld, msgId:%d\n", fromId, msgId);

		ntyVoiceAckAction(fromId, msgId);
		ntylog("====================end ntyVoiceAckPacketHandleRequest action ==========================\n");
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);
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


void ntyCommonReqPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_COMMON_REQ) {
		ntylog("====================begin ntyCommonReqPacketHandleRequest action ==========================\n");

		const MessagePacket *msg = (const MessagePacket*)obj;
		if (msg == NULL) return ;
		const Client *client = msg->client;

		C_DEVID fromId = *(C_DEVID*)(buffer+NTY_PROTO_COMMON_REQ_DEVID_IDX);
		C_DEVID toId = *(C_DEVID*)(buffer+NTY_PROTO_COMMON_REQ_RECVID_IDX);

		U16 jsonlen = 0;
		memcpy(&jsonlen, buffer+NTY_PROTO_COMMON_REQ_JSON_LENGTH_IDX, NTY_JSON_COUNT_LENGTH);
		char *jsonstring = malloc(jsonlen+1);
		if (jsonstring == NULL) {
			ntylog(" %s --> malloc failed jsonstring\n", __func__);
			
			return ;
		}
		
		memset(jsonstring, 0, jsonlen+1);
		memcpy(jsonstring, buffer+NTY_PROTO_COMMON_REQ_JSON_CONTENT_IDX, jsonlen);

		ntylog("ntyCommonReqPacketHandleRequest --> fromId:%lld, toId:%lld, json : %s  %d\n", fromId, toId, jsonstring, jsonlen);
		
		JSON_Value *json = ntyMallocJsonValue(jsonstring);
		if (json == NULL) { //JSON Error and send Code to FromId Device
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_JSON_FORMAT);
		} else {
			size_t len_ActionParam = sizeof(ActionParam);
			ActionParam *pActionParam = malloc(len_ActionParam);
			if (pActionParam == NULL) {
				ntylog(" %s --> malloc failed ActionParam", __func__);
				free(jsonstring);

				return ;
			}
			memset(pActionParam, 0, len_ActionParam);
			 
			pActionParam->fromId = fromId;
			pActionParam->toId = toId;
			pActionParam->json = json;
			pActionParam->jsonstring = jsonstring;
			pActionParam->jsonlen = jsonlen;
			pActionParam->index = 0;
			
			ntyCommonReqAction(pActionParam);
			free(pActionParam);
		}
		free(jsonstring);
		ntyFreeJsonValue(json);
		
		ntylog("====================end ntyCommonReqPacketHandleRequest action ==========================\n");
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);
	}
	
}

static const ProtocolFilter ntyCommonReqFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyCommonReqPacketHandleRequest,
};

void ntyCommonAckPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_COMMON_ACK) {
		ntylog("====================begin ntyCommonAckPacketHandleRequest action ==========================\n");

		const MessagePacket *msg = (const MessagePacket*)obj;
		if (msg == NULL) return ;
		const Client *client = msg->client;

		C_DEVID fromId = *(C_DEVID*)(buffer+NTY_PROTO_COMMON_ACK_DEVID_IDX);
		U32 msgId = *(U32*)(buffer+NTY_PROTO_COMMON_ACK_MSGID_IDX);
		ntylog("ntyCommonAckPacketHandleRequest fromId:%lld, msgId:%d\n", fromId, msgId);
#if 0

		int ret = ntyExecuteCommonOfflineMsgDeleteHandle(msgId, fromId);
		if (ret == NTY_RESULT_FAILED) {
			ntylog("ntyExecuteCommonOfflineMsgDeleteHandle DB Error \n");
		}
#else	
		VALUE_TYPE *tag = malloc(sizeof(VALUE_TYPE));
		if (tag == NULL) {
			ntylog(" %s --> malloc failed VALUE_TYPE. \n", __func__);
			return;
		}
		
		memset(tag, 0, sizeof(VALUE_TYPE));

		tag->fromId = fromId;
		tag->arg = msgId;
		if (client->deviceType == NTY_PROTO_CLIENT_WATCH) {
			tag->Type = MSG_TYPE_DEVICE_COMMON_ACK_HANDLE;
		} else {
			tag->Type = MSG_TYPE_APP_COMMON_ACK_HANDLE;
		}
		tag->cb = ntyCommonAckHandle;

		ntyDaveMqPushMessage(tag);
#endif
		ntylog("====================end ntyCommonAckPacketHandleRequest action ==========================\n");
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);
	}
}

static const ProtocolFilter ntyCommonAckFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyCommonAckPacketHandleRequest,
};


#if 1
U8 u8VoicePacket[NTY_VOICEREQ_COUNT_LENGTH*NTY_VOICEREQ_PACKET_LENGTH] = {0};
#endif

void ntyVoiceDataReqPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_VOICE_DATA_REQ) {
		ntylog("====================begin ntyVoiceDataReqPacketHandleRequest action ==========================\n");

		const MessagePacket *msg = (const MessagePacket*)obj;
		if (msg == NULL) return ;
		const Client *client = msg->client;

		int i = 0;
		
		U16 index = *(U16*)(buffer+NTY_PROTO_VOICE_DATA_REQ_PKTINDEX_IDX);
		U16 Count = *(U16*)(&buffer[NTY_PROTO_VOICE_DATA_REQ_PKTTOTLE_IDX]);
		U32 pktLength = *(U32*)(buffer+NTY_PROTO_VOICE_DATA_REQ_PKTLENGTH_IDX);

		ntylog(" Count:%d, index:%d, pktLength:%d, length:%d, pktLength%d\n", 
			Count, index, pktLength, length, NTY_PROTO_VOICE_DATA_REQ_PKTLENGTH_IDX);

		void *heap = ntyBHeapInstance();
		NRecord *record = ntyBHeapSelect(heap, client->devId);
		if (record == NULL) {
			ntylog(" ntyBHeapSelect is not exist %lld\n", client->devId);
			return ;
		}
		Client *pClient = (Client*)record->value;

		if (pClient->rBuffer == NULL) {
			pClient->rBuffer = malloc(NTY_VOICEREQ_COUNT_LENGTH*NTY_VOICEREQ_PACKET_LENGTH);
			
		}
		if (index == 0) { //start voice packet
			memset(pClient->rBuffer, 0, NTY_VOICEREQ_COUNT_LENGTH*NTY_VOICEREQ_PACKET_LENGTH);
		}

		for (i = 0;i < pktLength;i ++) {
			pClient->rBuffer[index * NTY_VOICEREQ_DATA_LENGTH + i] = buffer[NTY_VOICEREQ_HEADER_LENGTH + i];
		}
		if (index == Count-1) { //save voice data
			long stamp = 0;
			C_DEVID gId = 0, fromId = 0;
			U8 filename[NTY_VOICE_FILENAME_LENGTH] = {0};
			
			ntyU8ArrayToU64(buffer+NTY_PROTO_VOICE_DATA_REQ_DEVID_IDX, &fromId);
			ntyU8ArrayToU64(buffer+NTY_PROTO_VOICE_DATA_REQ_GROUP_IDX, &gId);
			
#if ENABLE_NATTY_TIME_STAMP //TIME Stamp 	
			stamp = ntyTimeStampGenrator();
#endif
			sprintf(filename, NTY_VOICE_FILENAME_FORMAT, gId, fromId, stamp);

			U32 dataLength = NTY_VOICEREQ_DATA_LENGTH*(Count-1) + pktLength;
			ntylog(" Voice FileName : %s, %d\n", filename, dataLength);
			ntyWriteDat(filename, pClient->rBuffer, dataLength);

			//release rBuffer
			free(pClient->rBuffer);
			pClient->rBuffer = NULL;

/* enter to SrvAction
 * VoiceBroadCast to gId
 * 0. send data result to from Id 
 * 1. save to DB
 * 2. prepare to offline voice data
 * 
 */			

 			ntyVoiceDataReqAction(fromId, gId, filename);


		}

		ntylog("====================end ntyVoiceDataReqPacketHandleRequest action ==========================\n");
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);
	}
}

static const ProtocolFilter ntyVoiceDataReqFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyVoiceDataReqPacketHandleRequest,
};

void ntyVoiceDataAckPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_VOICE_ACK) {
		ntylog("====================begin ntyVoiceDataAckPacketHandleRequest action ==========================\n");

		const MessagePacket *msg = (const MessagePacket*)obj;
		if (msg == NULL) return ;
		const Client *client = msg->client;

#if 0
		C_DEVID toId = 0, fromId = 0;
		ntyU8ArrayToU64(buffer+NTY_PROTO_VOICEACK_SELFID_IDX, &fromId);
		ntyU8ArrayToU64(buffer+NTY_PROTO_VOICEACK_DESTID_IDX, &toId);

		ntylog(" ntyVoiceAckPacketHandleRequest --> toId:%lld, fromId:%lld\n", toId, fromId);

#if 0
		void *pRBTree = ntyRBTreeInstance();
		Client *toClient = (Client*)ntyRBTreeInterfaceSearch(pRBTree, toId);
#else
		void *map = ntyMapInstance();
		ClientSocket *toClient = ntyMapSearch(map, toId);
#endif
		if (toClient == NULL) { //no Exist
			return ;
		} 

		ntySendBuffer(toClient, buffer, length);
#else
		ntylog(" ntyVoiceDataAckPacketHandleRequest is dispatch\n");
#endif
		ntylog("====================end ntyVoiceDataAckPacketHandleRequest action ==========================\n");
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);
	}
}

static const ProtocolFilter ntyVoiceDataAckFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyVoiceDataAckPacketHandleRequest,
};

void ntyOfflineMsgReqPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_OFFLINE_MSG_REQ) {
		ntylog("====================begin ntyOfflineMsgReqPacketHandleRequest action ==========================\n");

		const MessagePacket *msg = (const MessagePacket*)obj;
		if (msg == NULL) return ;
		const Client *client = msg->client;

		/*
		C_DEVID fromId = *(C_DEVID*)(buffer+NTY_PROTO_OFFLINE_MSG_REQ_DEVICEID_IDX);
		U16 jsonlen = 0;
		memcpy(&jsonlen, buffer+NTY_PROTO_OFFLINE_MSG_REQ_JSON_LENGTH_IDX, NTY_JSON_COUNT_LENGTH);
		char *jsonstring = malloc(jsonlen);
		memset(jsonstring, 0, jsonlen);
		memcpy(jsonstring, buffer+NTY_PROTO_OFFLINE_MSG_REQ_JSON_CONTENT_IDX, jsonlen);

		ntylog("ntyOfflineMsgReqPacketHandleRequest --> json : %s  %d\n", jsonstring, jsonlen);

		JSON_Value *json = ntyMallocJsonValue(jsonstring);
		if (json == NULL) {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_JSON_FORMAT);
			return;
		}
		ntyJsonOfflineMsgReqAction(fromId, fromId, json, jsonstring, jsonlen);
		ntyFreeJsonValue(json);
		free(jsonstring);
		*/
		ntylog("====================end ntyOfflineMsgReqPacketHandleRequest action ==========================\n");
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);
	}
}

static const ProtocolFilter ntyOfflineMsgReqFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyOfflineMsgReqPacketHandleRequest,
};

void ntyOfflineMsgAckPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_OFFLINE_MSG_ACK) {
		ntylog("====================begin ntyOfflineMsgAckPacketHandleRequest action ==========================\n");

		const MessagePacket *msg = (const MessagePacket*)obj;
		if (msg == NULL) return ;
		const Client *client = msg->client;
		
		ntylog("====================end ntyOfflineMsgAckPacketHandleRequest action ==========================\n");
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);
	}
}

static const ProtocolFilter ntyOfflineMsgAckFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyOfflineMsgAckPacketHandleRequest,
};


void ntyUnBindDevicePacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_UNBIND_REQ) {
		ntylog("====================begin ntyUnBindDevicePacketHandleRequest action ==========================\n");

		const MessagePacket *msg = (const MessagePacket*)obj;
		if (msg == NULL) return ;
		const Client *client = msg->client;
		
		C_DEVID AppId = *(C_DEVID*)(buffer+NTY_PROTO_UNBIND_APPID_IDX);
		C_DEVID DeviceId = *(C_DEVID*)(buffer+NTY_PROTO_UNBIND_DEVICEID_IDX);

#if ENABLE_CONNECTION_POOL
#if 0
		int ret = ntyExecuteDevAppRelationDeleteHandle(AppId, DeviceId);
#else
		int ret = ntyExecuteDevAppGroupDeleteHandle(AppId, DeviceId);
#endif
		if (ret == -1) {
			ntylog(" ntyUnBindDevicePacketHandleRequest --> DB Exception\n");
			ret = 4;
		} else if (ret == 0) {
#if 0
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
#else
			//void *map = ntyMapInstance();
			void *heap = ntyBHeapInstance();
			NRecord *record = ntyBHeapSelect(heap, AppId);
			if (record != NULL) {
				Client *aclient = (Client *)record->value;
				ASSERT(aclient != NULL);

				ntylog("ntyUnBindDevicePacketHandleRequest -> record != NULL AppId\n");

				ret = ntyVectorDelete(aclient->friends, &DeviceId);

				ntylog("ntyVectorDelete AppId:%lld ret : %d\n", AppId, ret);
			}

			record = ntyBHeapSelect(heap, DeviceId);
			if (record != NULL) {
				Client *dclient = (Client *)record->value;
				ASSERT(dclient != NULL);
				ntylog("ntyUnBindDevicePacketHandleRequest -> record != NULL DeviceId\n");
				
				ret = ntyVectorDelete(dclient->friends, &AppId);

				ntylog("ntyVectorDelete DeviceId:%lld ret : %d\n", DeviceId, ret);
			}
#endif		
			ret = NTY_RESULT_SUCCESS;
		}
		ntyProtoUnBindAck(AppId, DeviceId, ret);
#endif		
		
		ntylog("====================end ntyUnBindDevicePacketHandleRequest action ==========================\n");
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);
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

void ntyBindConfirmReqPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_BIND_CONFIRM_REQ) {
		ntylog("====================begin ntyBindConfirmReqPacketHandleRequest action ==========================\n");

		const MessagePacket *msg = (const MessagePacket*)obj;
		if (msg == NULL) return ;
		const Client *client = msg->client;
		
		C_DEVID fromId = *(C_DEVID*)(buffer+NTY_PROTO_BIND_CONFIRM_REQ_ADMIN_SELFID_IDX);
		C_DEVID AppId = *(C_DEVID*)(buffer+NTY_PROTO_BIND_CONFIRM_REQ_PROPOSER_IDX);
		C_DEVID DeviceId = *(C_DEVID*)(buffer+NTY_PROTO_BIND_CONFIRM_REQ_DEVICEID_IDX);

		U32 msgId = *(U32*)(buffer+NTY_PROTO_BIND_CONFIRM_REQ_MSGID_IDX);
		U8 *jsonstring = buffer+NTY_PROTO_BIND_CONFIRM_REQ_JSON_CONTENT_IDX;
		U16 jsonLen = *(U16*)(buffer+NTY_PROTO_BIND_CONFIRM_REQ_JSON_LENGTH_IDX);

		ntylog("ntyBindConfirmReqPacketHandleRequest --> json : %s  %d\n", jsonstring, jsonLen);

		JSON_Value *json = ntyMallocJsonValue(jsonstring);
		if (json == NULL) { //JSON Error and send Code to FromId Device
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_JSON_FORMAT);
		} else {
			BindConfirmReq *pBindConfirmReq = (BindConfirmReq*)malloc(sizeof(BindConfirmReq));
			if (pBindConfirmReq == NULL) {
				ntylog(" %s --> malloc failed BindConfirmReq. \n", __func__);
				return ;
			}
			memset(pBindConfirmReq, 0, sizeof(BindConfirmReq));
			ntyJsonBindConfirmReq(json, pBindConfirmReq);

			if (strcmp(pBindConfirmReq->category, NATTY_USER_PROTOCOL_BINDCONFIRMREQ) == 0) {
				VALUE_TYPE *tag = malloc(sizeof(VALUE_TYPE));
				if (tag == NULL) {
					free(pBindConfirmReq);
					return ;
				}
				memset(tag, 0, sizeof(VALUE_TYPE));
				
				tag->fromId = fromId;
				tag->toId = AppId;
				tag->gId = DeviceId;
				tag->cb = ntyBindConfirmReqHandle;
				tag->arg = msgId;
				tag->Type = MSG_TYPE_BIND_CONFIRM_REQ_HANDLE;
				
				if (strcmp(pBindConfirmReq->answer, NATTY_USER_PROTOCOL_AGREE) == 0) {
					tag->u8LocationType = 1;
				} else if (strcmp(pBindConfirmReq->answer, NATTY_USER_PROTOCOL_REJECT) == 0) {
					tag->u8LocationType = 0;
				} else {
					ntylog("Can't find answer with: %s\n", pBindConfirmReq->answer);
				}
				
				ntydbg("tag->answer:%s  \n", pBindConfirmReq->answer);
				ntydbg("tag->u8LocationType:%d  \n", tag->u8LocationType);
				ntyDaveMqPushMessage(tag);
				free(pBindConfirmReq);
			
			}

			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);
		}

		ntyFreeJsonValue(json);
		
#if ENABLE_CONNECTION_POOL

#if 1 //Need to Recode

#if 0 //juge agree and reject
#define NTY_TOKEN_AGREE			"Agree"
#define NTY_TOKEN_AGREE_LENGTH	5

#define NTY_TOKEN_REJECT		"Reject"
#define NTY_TOKEN_REJECT_LENGTH	6
		U32 match[6] = {0};

		VALUE_TYPE *tag = malloc(sizeof(VALUE_TYPE));

		if (ntyKMP(jsonstring, jsonLen, NTY_TOKEN_AGREE, NTY_TOKEN_AGREE_LENGTH, match)) {
			tag->arg = 1;
		} else if (ntyKMP(jsonstring, jsonLen, NTY_TOKEN_REJECT, NTY_TOKEN_REJECT_LENGTH, match)) {
			tag->arg = 0;
		} else {
			tag->arg = 1;
		}
#endif
		
#endif
#endif

		ntylog("====================end ntyBindConfirmReqPacketHandleRequest action ==========================\n");
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);
	}
}

static const ProtocolFilter ntyBindConfirmReqPacketFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyBindConfirmReqPacketHandleRequest,
};


void ntyBindDevicePacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_BIND_REQ) {
		ntylog("====================begin ntyBindDevicePacketHandleRequest action ==========================\n");

		const MessagePacket *msg = (const MessagePacket*)obj;
		if (msg == NULL) return ;
		const Client *client = msg->client;
		
		C_DEVID fromId = *(C_DEVID*)(buffer+NTY_PROTO_BIND_APPID_IDX);
		C_DEVID toId =  *(C_DEVID*)(buffer+NTY_PROTO_BIND_DEVICEID_IDX);
		
#if 1	//New Version need implement
		U16 jsonlen = *(U16*)(buffer+NTY_PROTO_BIND_JSON_LENGTH_IDX);
		//memcpy(&jsonlen, buffer+NTY_PROTO_BIND_JSON_LENGTH_IDX, NTY_JSON_COUNT_LENGTH);
		char *jsonstring = malloc(jsonlen+1);
		if (jsonstring == NULL) {
			ntylog(" %s --> malloc failed jsonstring. \n", __func__);
			return;
		}
		memset(jsonstring, 0, jsonlen+1);
		memcpy(jsonstring, buffer+NTY_PROTO_BIND_JSON_CONTENT_IDX, jsonlen);

		ntylog("ntyBindDevicePacketHandleRequest --> json : %s  %d\n", jsonstring, jsonlen);
#if 0		
		JSON_Value *json = ntyMallocJsonValue(jsonstring);
		if (json == NULL) { //JSON Error and send Code to FromId Device
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_JSON_FORMAT);
		} else {

			ActionParam *pActionParam = malloc(sizeof(ActionParam));
			if (pActionParam == NULL) {
				ntylog(" %s --> malloc failed ActionParam. \n", __func__);
				free(jsonstring);
				return;
			}
			pActionParam->fromId = fromId;
			pActionParam->toId = toId;
			pActionParam->json = json;
			pActionParam->jsonstring= jsonstring;
			pActionParam->jsonlen = jsonlen;
			pActionParam->index = 0;
			
			ntyBindReqAction(pActionParam);
			
			free(pActionParam);

		}
		free(jsonstring);
		ntyFreeJsonValue(json);
#else
		JSON_Value *json = ntyMallocJsonValue(jsonstring);
		if (json == NULL) { //JSON Error and send Code to FromId Device
			return ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_JSON_FORMAT);
		}

		VALUE_TYPE *tag = malloc(sizeof(VALUE_TYPE));
		if (tag == NULL) {
			ntylog(" %s --> malloc VALUE_TYPE error. \n", __func__);
			free(jsonstring);
			return;
		}
		
		tag->fromId = fromId;
		tag->gId = toId;
		
		tag->Tag = (U8*)json;		
		tag->length = jsonlen;
		
		tag->cb = ntyBindDeviceCheckStatusReqHandle;
		tag->Type = MSG_TYPE_BIND_CONFIRM_REQ_HANDLE;
		ntyDaveMqPushMessage(tag);
		
		free(jsonstring);
#endif



#else
		ntyProtoBindAck(fromId, toId, 5);
#endif		
		ntylog("====================end ntyBindDevicePacketHandleRequest action ==========================\n");
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);
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
	
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_MULTICAST_REQ) {
		ntylog("====================begin ntyMulticastReqPacketHandleRequest action ==========================\n");

		const MessagePacket *msg = (const MessagePacket*)obj;
		if (msg == NULL) return ;
		const Client *client = msg->client;
		
		C_DEVID toId = 0;
		ntyU8ArrayToU64(buffer+NTY_PROTO_DEST_DEVID_IDX, &toId);

		void *pRBTree = ntyRBTreeInstance();
		Client *toClient = (Client*)ntyRBTreeInterfaceSearch(pRBTree, toId);
		if (toClient == NULL) { //no Exist
			return;
		}

		buffer[NTY_PROTO_MULTICAST_TYPE_IDX] = NTY_PROTO_DATAPACKET_REQ;
		//ntySendBuffer(toClient, buffer, length);
		//ntyMulticastSend();

		ntylog("====================end ntyMulticastReqPacketHandleRequest action ==========================\n");
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);
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
	
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_MULTICAST_ACK) {
		ntylog("====================begin ntyMulticastAckPacketHandleRequest action ==========================\n");

		const MessagePacket *msg = (const MessagePacket*)obj;
		if (msg == NULL) return ;
		const Client *client = msg->client;
		
		C_DEVID fromId = 0;
			
		ntyU8ArrayToU64(buffer+NTY_PROTO_DEST_DEVID_IDX, &fromId);

		ntylog("====================end ntyMulticastAckPacketHandleRequest action ==========================\n");
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);
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

void ntyLocationAsyncReqPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_LOCATION_ASYNCREQ) {
		ntylog("====================begin ntyLocationAsyncReqPacketHandleRequest action ==========================\n");

		const MessagePacket *msg = (const MessagePacket*)obj;
		if (msg == NULL) return ;
		const Client *client = msg->client;

		U16 jsonlen = 0;
		memcpy(&jsonlen, buffer+NTY_PROTO_LOCATION_ASYNC_REQ_JSON_LENGTH_IDX, NTY_JSON_COUNT_LENGTH);
		char *jsonstring = malloc(jsonlen+1);
		if (jsonstring == NULL) {
			ntylog(" %s --> malloc failed jsonstring. \n", __func__);
			return;
		}
		memset(jsonstring, 0, jsonlen+1);
		memcpy(jsonstring, buffer+NTY_PROTO_LOCATION_ASYNC_REQ_JSON_CONTENT_IDX, jsonlen);

		C_DEVID deviceId = *(C_DEVID*)(buffer+NTY_PROTO_LOCATION_ASYNC_REQ_DEVID_IDX);

		JSON_Value *json = ntyMallocJsonValue(jsonstring);
		if (json == NULL) {
			ntyJsonCommonResult(deviceId, NATTY_RESULT_CODE_ERR_JSON_FORMAT);
		} else {
			size_t len_ActionParam = sizeof(ActionParam);
			ActionParam *pActionParam = malloc(len_ActionParam);
			if (pActionParam == NULL) {
				ntylog(" %s --> malloc failed ActionParam. \n", __func__);
				goto exit;
			}
			memset(pActionParam, 0, len_ActionParam);

			pActionParam->fromId = client->devId;
			pActionParam->toId = deviceId;
			pActionParam->json = json;
			pActionParam->jsonstring= jsonstring;
			pActionParam->jsonlen = jsonlen;
			pActionParam->index = 0;
			const char *category = ntyJsonAppCategory(json);
			if (strcmp(category, NATTY_USER_PROTOCOL_WIFI) == 0) {
				ntyJsonLocationWIFIAction(pActionParam);
			} else if (strcmp(category, NATTY_USER_PROTOCOL_LAB) == 0) {
				ntyJsonLocationLabAction(pActionParam);
			} else {
				ntylog("Can't find category with: %s\n", category);
			}
			free(pActionParam);
		}
		
		ntyFreeJsonValue(json);

exit:
		free(jsonstring);
		ntylog("====================end ntyLocationAsyncReqPacketHandleRequest action ==========================\n");
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);
	}
}

static const ProtocolFilter ntyLocationAsyncReqFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyLocationAsyncReqPacketHandleRequest,
};


void ntyWeatherAsyncReqPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_WEATHER_ASYNCREQ) {
		ntylog("====================begin ntyWeatherAsyncReqPacketHandleRequest action ==========================\n");

		const MessagePacket *msg = (const MessagePacket*)obj;
		if (msg == NULL) return ;
		const Client *client = msg->client;

		U16 jsonlen = 0;
		memcpy(&jsonlen, buffer+NTY_PROTO_WEATHER_ASYNC_REQ_JSON_LENGTH_IDX, NTY_JSON_COUNT_LENGTH);
		char *jsonstring = malloc(jsonlen+1);
		if (jsonstring == NULL) {
			ntylog(" %s --> malloc failed jsonstring. \n", __func__);
			return;
		}

		memset(jsonstring, 0, jsonlen+1);
		memcpy(jsonstring, buffer+NTY_PROTO_WEATHER_ASYNC_REQ_JSON_CONTENT_IDX, jsonlen);
		C_DEVID fromId = *(C_DEVID*)(buffer+NTY_PROTO_WEATHER_ASYNC_REQ_DEVID_IDX);

		ntylog("ntyWeatherAsyncReqPacketHandleRequest --> json : %s  %d\n", jsonstring, jsonlen);
		
		JSON_Value *json = ntyMallocJsonValue(jsonstring);
		if (json == NULL) {
			ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_JSON_FORMAT);
		} else {
			size_t len_ActionParam = sizeof(ActionParam);
			ActionParam *pActionParam = malloc(len_ActionParam);
			if (pActionParam == NULL) {
				ntylog(" %s --> malloc failed ActionParam. \n", __func__);
				goto exit;
			}
			memset(pActionParam, 0, len_ActionParam);
			
			pActionParam->fromId = fromId;
			pActionParam->toId = client->devId;
			pActionParam->json = json;
			pActionParam->jsonstring= jsonstring;
			pActionParam->jsonlen = jsonlen;
			pActionParam->index = 0;
			ntyJsonWeatherAction(pActionParam);
			free(pActionParam);
		}
		ntyFreeJsonValue(json);

exit:
		free(jsonstring);
		ntylog("====================end ntyWeatherAsyncReqPacketHandleRequest action ==========================\n");
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);
	}
}

static const ProtocolFilter ntyWeatherAsyncReqFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyWeatherAsyncReqPacketHandleRequest,
};


/*
 * Server Proxy Data Transport
 * VERSION					1			 BYTE
 * MESSAGE TYPE			 	1			 BYTE (req, ack)
 * TYPE				 		1			 BYTE 
 * DEVID					8			 BYTE (self devid)
 * ACKNUM					4			 BYTE (Network Module Set Value)
 * DEST DEVID				8			 BYTE (friend devid)
 * CONTENT COUNT			2			 BYTE 
 * CONTENT					*(CONTENT COUNT)	 BYTE 
 * CRC 				 		4			 BYTE (Network Module Set Value)
 * 
 * send to server addr, proxy to send one client
 * 
 */
void ntyRoutePacketHandleRequest(const void *_self, unsigned char *buffer, int length,const void* obj) {
	
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_DATA_ROUTE) {
		ntylog("====================begin ntyRoutePacketHandleRequest action ==========================\n");

		const MessagePacket *msg = (const MessagePacket*)obj;
		if (msg == NULL) return ;
		const Client *client = msg->client;

		C_DEVID fromId = *(C_DEVID*)(buffer+NTY_PROTO_DATA_ROUTE_DEVID_IDX);
		C_DEVID toId = *(C_DEVID*)(buffer+NTY_PROTO_DATA_ROUTE_RECVID_IDX);

		ntylog("ntyRoutePacketHandleRequest --> fromId:%lld, toId:%lld\n", fromId, toId);
		
		int len = ntySendDataRoute(toId, (U8*)buffer, length);
		if (len>=0) {
			ntydbg("ntySendDataRoute success \n");
			if (1){//client->deviceType == NTY_PROTO_CLIENT_WATCH) {
				ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);
			} else {

			}
		} else {
			ntydbg("ntySendDataRoute no exist \n");
			if (1){//client->deviceType == NTY_PROTO_CLIENT_WATCH) {
				ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
			} else {
			
			}
		}
		ntylog("====================end ntyRoutePacketHandleRequest action ==========================\n");
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);
	}
}


static const ProtocolFilter ntyRoutePacketFilter = {
	sizeof(Packet),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyRoutePacketHandleRequest,
};
#if 0
void ntyUserDataPacketAckHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	const UdpClient *client = obj;
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_DATAPACKET_ACK) {


	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, length, obj);
	} else {
		ntylog("Can't deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);
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
#endif



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
const void *pNtyTimeCheckFilter = &ntyTimeCheckFilter;
const void *pNtyICCIDReqFilter = &ntyICCIDReqFilter;

const void *pNtyVoiceReqFilter = &ntyVoiceReqFilter;
const void *pNtyVoiceAckFilter = &ntyVoiceAckFilter;

const void *pNtyCommonReqFilter = &ntyCommonReqFilter;
const void *pNtyCommonAckFilter = &ntyCommonAckFilter;

const void *pNtyVoiceDataReqFilter = &ntyVoiceDataReqFilter;
const void *pNtyVoiceDataAckFilter = &ntyVoiceDataAckFilter;

const void *pNtyOfflineMsgReqFilter = &ntyOfflineMsgReqFilter;
const void *pNtyOfflineMsgAckFilter = &ntyOfflineMsgAckFilter;

const void *pNtyRoutePacketFilter = &ntyRoutePacketFilter;
const void *pNtyUnBindDeviceFilter = &ntyUnBindDeviceFilter;

const void *pNtyBindDeviceFilter = &ntyBindDeviceFilter;
const void *pNtyBindConfirmReqPacketFilter = &ntyBindConfirmReqPacketFilter;

const void *pNtyMutlcastReqFilter = &ntyMutlcastReqFilter;
const void *pNtyMutlcastAckFilter = &ntyMutlcastAckFilter;

const void *pNtyLocationAsyncReqFilter = &ntyLocationAsyncReqFilter;
const void *pNtyWeatherAsyncReqFilter = &ntyWeatherAsyncReqFilter;


//ntyVoiceReqPacketHandleRequest

void* ntyProtocolFilterInit(void) {
	void *pLoginFilter = New(pNtyLoginFilter);
	void *pHeartBeatFilter = New(pNtyHeartBeatFilter);
	void *pLogoutFilter = New(pNtyLogoutFilter);
	
	void *pTimeCheckFilter = New(pNtyTimeCheckFilter);
	void *pICCIDReqFilter = New(pNtyICCIDReqFilter);
	
	void *pVoiceReqFilter = New(pNtyVoiceReqFilter);
	void *pVoiceAckFilter = New(pNtyVoiceAckFilter);

	void *pCommonReqFilter = New(pNtyCommonReqFilter);
	void *pCommonAckFilter = New(pNtyCommonAckFilter);

	void *pVoiceDataReqFilter = New(pNtyVoiceDataReqFilter);
	void *pVoiceDataAckFilter = New(pNtyVoiceDataAckFilter);

	void *pOfflineMsgReqFilter = New(pNtyOfflineMsgReqFilter);
	void *pOfflineMsgAckFilter = New(pNtyOfflineMsgAckFilter);

	void *pRoutePacketFilter = New(pNtyRoutePacketFilter);
	void *pUnBindDeviceFilter = New(pNtyUnBindDeviceFilter);
	
	void *pBindDeviceFilter = New(pNtyBindDeviceFilter);
	void *pBindConfirmReqPacketFilter = New(pNtyBindConfirmReqPacketFilter);
	
	void *pMutlcastReqFilter = New(pNtyMutlcastReqFilter);
	void *pMutlcastAckFilter = New(pNtyMutlcastAckFilter);
	
	void *pLocationAsyncReqFilter = New(pNtyLocationAsyncReqFilter);
	void *pWeatherAsyncReqFilter = New(pNtyWeatherAsyncReqFilter);

	ntySetSuccessor(pLoginFilter, pHeartBeatFilter);
	ntySetSuccessor(pHeartBeatFilter, pLogoutFilter);
	ntySetSuccessor(pLogoutFilter, pTimeCheckFilter);
	ntySetSuccessor(pTimeCheckFilter, pICCIDReqFilter);
	
	ntySetSuccessor(pICCIDReqFilter, pVoiceReqFilter);
	ntySetSuccessor(pVoiceReqFilter, pVoiceAckFilter);
	
	ntySetSuccessor(pVoiceAckFilter, pCommonReqFilter);
	ntySetSuccessor(pCommonReqFilter, pCommonAckFilter);
	ntySetSuccessor(pCommonAckFilter, pVoiceDataReqFilter);

	ntySetSuccessor(pVoiceDataReqFilter, pVoiceDataAckFilter);
	ntySetSuccessor(pVoiceDataAckFilter, pOfflineMsgReqFilter);
	
	ntySetSuccessor(pOfflineMsgReqFilter, pOfflineMsgAckFilter);
	ntySetSuccessor(pOfflineMsgAckFilter, pRoutePacketFilter);
	
	ntySetSuccessor(pRoutePacketFilter, pUnBindDeviceFilter);
	ntySetSuccessor(pUnBindDeviceFilter, pBindDeviceFilter);
	
	ntySetSuccessor(pBindDeviceFilter, pBindConfirmReqPacketFilter);
	ntySetSuccessor(pBindConfirmReqPacketFilter, pMutlcastReqFilter);

	ntySetSuccessor(pMutlcastReqFilter, pMutlcastAckFilter);
	ntySetSuccessor(pMutlcastAckFilter, pLocationAsyncReqFilter);
	
	ntySetSuccessor(pLocationAsyncReqFilter, pWeatherAsyncReqFilter);
	ntySetSuccessor(pWeatherAsyncReqFilter, NULL);

	
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

	return pLoginFilter;
}

static void ntyClientBufferRelease(Client *client) {
	pthread_mutex_lock(&client->bMutex);
#if 0
	free(client->recvBuffer);
	client->recvBuffer = NULL;
	client->rLength = 0;
#else
	memset(client->recvBuffer, 0, PACKET_BUFFER_SIZE);
	client->rLength = 0;
#endif
	pthread_mutex_unlock(&client->bMutex);
}

void ntyProtocolFilterProcess(void *_filter, unsigned char *buffer, U32 length, const void *obj) {
#if 1
	//data crc is right, and encryto
	U32 u32Crc = ntyGenCrcValue(buffer, length-4);
	U32 u32ClientCrc = *((U32*)(buffer+length-4));

	const MessagePacket *msg = (const MessagePacket*)obj;
	if (msg == NULL) return ;
	const Client *client = msg->client;
	if (client == NULL) return ;
#if 0	
	struct sockaddr_in addr;
	memcpy(&addr, &client->addr, sizeof(struct sockaddr_in));

	C_DEVID id = ntySearchDevIdFromHashTable(&addr);
	ntylog("ntyProtocolFilterProcess :%lld\n", id);
	if (id == 0) return ;
#endif	
	void *pBHeap = ntyBHeapInstance();
	NRecord *Record = (NRecord*)ntyBHeapSelect(pBHeap, client->devId);
	//if (Record == NULL) return ;
	
	//ntydbg(" client:%x, server:%x, length:%d", u32ClientCrc, u32Crc, length);
	if (u32Crc != u32ClientCrc || length < NTY_PROTO_MIN_LENGTH) {
#if 1 
		ntylog(" ntyProtocolFilterProcess --> client:%x, server:%x, length:%d\n", u32ClientCrc, u32Crc, length);
		//const Client *client = obj;
		if (Record == NULL) return ; //illeges record
		Client* pClient = Record->value;
		if (pClient == NULL) return ;
		
		if (1 /*&& client->connectType == PROTO_TYPE_TCP*/) { //have part data
			
			int bCopy = 0;
			int bIndex = 0, bLength = pClient->rLength;
			U8 bBuffer[PACKET_BUFFER_SIZE] = {0};

			do {
				
				bCopy = (length > PACKET_BUFFER_SIZE ? PACKET_BUFFER_SIZE : length);					
				bCopy = (((bLength + bCopy) > PACKET_BUFFER_SIZE) ? (PACKET_BUFFER_SIZE - bLength) : bCopy);
				
				pthread_mutex_lock(&pClient->bMutex);
				memcpy(pClient->recvBuffer+pClient->rLength, buffer+bIndex, bCopy);
				pClient->rLength %= PACKET_BUFFER_SIZE;
				pClient->rLength += bCopy;

				memset(bBuffer, 0, PACKET_BUFFER_SIZE);
				memcpy(bBuffer, pClient->recvBuffer, pClient->rLength);
				bLength = pClient->rLength;
				pthread_mutex_unlock(&pClient->bMutex);

				U32 uCrc = ntyGenCrcValue(bBuffer, bLength-4);
				U32 uClientCrc = *((U32*)(bBuffer+bLength-4));

				if (uCrc == uClientCrc)	{
					ntylog(" CMD:%x, Version:[%c]\n", bBuffer[NTY_PROTO_MSGTYPE_IDX], bBuffer[NTY_PROTO_DEVTYPE_IDX]);
					ntyHandleRequest(_filter, bBuffer, bLength, obj);

					ntyClientBufferRelease(pClient);
				} 
				
				length -= bCopy;
				bIndex += bCopy;
				
			} while (length);

			return ;

		}

		
#endif
		
	}

	if (Record != NULL) {
		//if (Record->value != NULL) {
		Client* pClient = Record->value;
		if (pClient != NULL) {
			pClient->rLength = 0;
		}
		//}
	}
#endif	
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
#define NTY_CRC_KEY					0x04c11db7ul

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

