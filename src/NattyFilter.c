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
#include "NattyBPlusTree.h"
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
#if 0
void ntyP2PNotifyClient(UdpClient *client, U8 *notify) {
	int length = 0;
	
	notify[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_P2P_NOTIFY_REQ;
	
	length = NTY_PROTO_P2P_NOTIFY_CRC_IDX;
	*(U32*)(&notify[length]) = ntyGenCrcValue(notify, length);
	length += sizeof(U32);

	ntySendBuffer(client, notify, length);

	return ;
}
#endif

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


extern void *ntyTcpServerGetMainloop(void);


/*
 * add rbtree <key, value> -- <UserId, sockfd>
 * add B+tree <key, value> -- <UserId, UserInfo>
 */
static int ntyAddClientHeap(const void * obj, RECORDTYPE *value) {
	const Client *client = obj;
	int ret = -1;

	//pClient->
	BPTreeHeap *heap = ntyBHeapInstance();
	//ASSERT(heap != NULL);
	NRecord *record = ntyBHeapSelect(heap, client->devId);
	if (record == NULL) {
		Client *pClient = (Client*)malloc(sizeof(Client));
		assert(pClient != NULL);

		memcpy(pClient, client, sizeof(Client));
		
		pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;
		memcpy(&pClient->bMutex, &blank_mutex, sizeof(pClient->bMutex));
		
		pClient->rLength = 0;
		pClient->recvBuffer = malloc(PACKET_BUFFER_SIZE);

#if ENABLE_NATTY_TIME_STAMP //TIME Stamp 	
		pClient->stamp = ntyTimeStampGenrator();
#endif

		if (pClient->friends == NULL) {
			pClient->friends = ntyVectorCreator();
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
	
		//start timer,
		NWTimer* nwTimer = ntyTimerInstance();
		unsigned long addr = (unsigned long)pClient;
		void* timer = ntyTimerAdd(nwTimer, 60, ntyCheckOnlineAlarmNotify, (void*)&addr, sizeof(unsigned long));
		pClient->hbdTimer = timer;

		//insert bheap
		ret = ntyBHeapInsert(heap, client->devId, pClient);
		if (ret == NTY_RESULT_EXIST || ret == NTY_RESULT_FAILED) {
			//ntylog("ret : %d\n", ret);
			free(pClient);
			ASSERT(0);
		}
		*value = pClient;
		
		return NTY_RESULT_NEEDINSERT;
	} else {
		Client *pClient = record->value;
#if ENABLE_NATTY_TIME_STAMP //TIME Stamp 	
		pClient->stamp = ntyTimeStampGenrator();
#endif
		*value = record->value;
	}

	return NTY_RESULT_SUCCESS;
}

static int ntyDelClientHeap(const void * obj) {
	const Client *client = obj;
	int ret = -1;

	void *heap = ntyBHeapInstance();
	NRecord *record = ntyBHeapSelect(heap, client->devId);
	if (record != NULL) {
		Client *pClient = record->value;

		pClient->rLength = 0;
		free(pClient->recvBuffer);

		NWTimer* nwTimer = ntyTimerInstance();
		ntyTimerDel(nwTimer, pClient->hbdTimer);
		pClient->hbdTimer = NULL;

		if (pClient->friends != NULL) {
			ntyVectorDestory(pClient->friends);
		}
#if 0 //release  groups
		pClient->group
#endif

		ret = ntyBHeapDelete(heap, client->devId);
		if (ret == NTY_RESULT_FAILED || ret == NTY_RESULT_NOEXIST) {
			ASSERT(0);
		}
		free(pClient);
	} else {
		ASSERT(1);
	}

	return NTY_RESULT_SUCCESS;
}



/*
 * Login Packet
 * Login Packet, HeartBeatPacket, LogoutPacket etc. should use templete designer pattern
 */
void ntyLoginPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	const UdpClient *client = obj;
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_LOGIN_REQ) {
#if 0
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
#else
		Client *pClient = NULL;
		ntyAddClientHeap(client, (RECORDTYPE *)&pClient);
		if (pClient != NULL) {
			//ntySendFriendsTreeIpAddr(pClient, 1);

			if (pClient->deviceType == NTY_PROTO_CLIENT_WATCH) {
#if 0
				ntySendDeviceTimeCheckAck(pClient, client->ackNum+1);
#elif 0
				ntySendDeviceTimeCheckAck(pClient, 1);
#else	
				ntySendLoginAckResult(pClient->devId, "", 0, 200);
#endif
			}
		} else {	
			ASSERT(0);
		}
#endif
		//ntylog("Login deal with: %d\n", buffer[NTY_PROTO_MSGTYPE_IDX]);		
		//send login ack

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
	const UdpClient *client = obj;
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_HEARTBEAT_REQ) {
		Client *pClient = NULL;
		ntyAddClientHeap(client, (RECORDTYPE *)&pClient);
		
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
	const UdpClient *client = obj;
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_LOGOUT_REQ) {
		//delete key

		ntyDelClientHeap(client);

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
#if 0
		C_DEVID key = *(C_DEVID*)(buffer+NTY_PROTO_LOGIN_REQ_DEVID_IDX);
		U32 ackNum = *(U32*)(buffer+NTY_PROTO_LOGIN_REQ_ACKNUM_IDX)+1;
		ntySendDeviceTimeCheckAck(client, ackNum);
#else
		const Client *client = obj;
		ntySendDeviceTimeCheckAck(client, 1);
#endif
		
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
	const Client *client = obj;
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_ICCID_REQ) {
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
	const Client *client = obj;
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_VOICE_REQ) {
		
		
		
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
	const Client *client = obj;
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_VOICE_ACK) {
		
		
		
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
	const Client *client = obj;
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_COMMON_REQ) {
		U16 jsonlen = 0;
		U8 *jsonstring = NULL;
		memcpy(&jsonlen, buffer+24, sizeof(U16));
		jsonstring = buffer+26;
		buffer[length-4] = '\0';
		U16 jsonlenTemp = strlen(jsonstring);
		if (jsonlen != jsonlenTemp) {
			ntylog("JSON format error: %s\n", jsonstring);
			return;
		}
	
		C_DEVID AppId = *(C_DEVID*)(buffer+NTY_PROTO_BIND_APPID_IDX);
		ntydbg("ntyCommonReqPacketHandleRequest --> json : %s\n", jsonstring);

		ntydbg("====================begin ntyCommonReqPacketHandleRequest action ==========================\n");
		
		JSON_Value *json = ntyMallocJsonValue(jsonstring);
		const char *category = ntyJsonAppCategory(json);
		if (strcmp(category, NATTY_USER_PROTOCOL_CATEGORY_EFENCE) == 0) {
			ntyJsonEfenceAction(AppId, client->devId, json, jsonstring, jsonlen);
		} else if (strcmp(category, NATTY_USER_PROTOCOL_CATEGORY_RUNTIME) == 0) {
			ntyJsonRunTimeAction(AppId, client->devId, json, jsonstring, jsonlen);
		} else if (strcmp(category, NATTY_USER_PROTOCOL_CATEGORY_TURN) == 0) {
			ntyJsonTurnAction(AppId, client->devId, json, jsonstring, jsonlen);
		} else if (strcmp(category, NATTY_USER_PROTOCOL_ICCID) == 0) {
			ntyJsonICCIDAction(AppId, client->devId, json, jsonstring, jsonlen);
		} else if (strcmp(category, NATTY_USER_PROTOCOL_CATEGORY_SCHEDULE) == 0) {
			const char *action = ntyJsonAction(json);
			if (strcmp(action, NATTY_USER_PROTOCOL_CATEGORY_ADD) == 0) {
				ntyJsonAddScheduleAction(AppId, client->devId, json, jsonstring, jsonlen);
			} else if (strcmp(action, NATTY_USER_PROTOCOL_CATEGORY_DELETE) == 0) {
				ntyJsonDelScheduleAction(AppId, client->devId, json, jsonstring, jsonlen);
			} else if (strcmp(action, NATTY_USER_PROTOCOL_CATEGORY_UPDATE) == 0) {
				ntyJsonUpdateScheduleAction(AppId, client->devId, json, jsonstring, jsonlen);
			} else if (strcmp(action, NATTY_USER_PROTOCOL_CATEGORY_SCHEDULE) == 0) {
				ntyJsonSelectScheduleAction(AppId, client->devId, json, jsonstring, jsonlen);
			} else {
				ntylog("Can't find action with: %s\n", action);
			}
		} else if (strcmp(category, NATTY_USER_PROTOCOL_CATEGORY_TIMETABLES) == 0) {
			ntyJsonTimeTablesAction(AppId, client->devId, json, jsonstring, jsonlen);
		} else {
			ntylog("Can't find category with: %s\n", category);
		}
		ntyFreeJsonValue(json);


		ntydbg("====================end ntyCommonReqPacketHandleRequest action ==========================\n");
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
	const Client *client = obj;
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_COMMON_ACK) {
		
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
	const Client *client = obj;
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_VOICE_DATA_REQ) {
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
#if 0
		void *pRBTree = ntyRBTreeInstance();
		Client *toClient = (Client*)ntyRBTreeInterfaceSearch(pRBTree, toId);
#else
		void *map = ntyMapInstance();
		ClientSocket *toClient = ntyMapSearch(map, toId);
#endif
		if (toClient == NULL) { //no Exist
			ntylog(" toClient is not Exist\n");

			int online = 0;
			int result = 0;
#if 1 //select device Login status, this operator should use redis, select in redis 
			if (client->deviceType == NTY_PROTO_CLIENT_WATCH) {
				ntyQueryDeviceOnlineStatusHandle(toId, &online);
			} else if (client->deviceType == NTY_PROTO_CLIENT_ANDROID
				|| client->deviceType == NTY_PROTO_CLIENT_IOS) {
				ntyQueryAppOnlineStatusHandle(toId, &online);
			} else {
				ntylog(" ntyUserDataPacketHandleRequest --> Packet : %x\n", client->deviceType);
				ASSERT(0);
			}
#endif
			ClientSocket *selfClient = ntyMapSearch(map, toId);
#if ENABLE_MULTICAST_SYNC //multicast 
			//buffer[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_MULTICAST_REQ;
			if (online == 1) { //no exist
				result = ntyMulticastSend(buffer, length);
			} else if (online == 0) {
				result = ntySendBuffer(selfClient, buffer, length);
			} else {
				ASSERT(0);
			}
#else
			result = ntySendBuffer(selfClient, buffer, length);
#endif

			return ;
		} 
		ntyProxyBuffer(toClient, buffer, length);
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
	const Client *client = obj;
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_VOICE_ACK) {
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
	const Client *client = obj;
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_OFFLINE_MSG_REQ) {
		
		
		
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
	const Client *client = obj;
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_OFFLINE_MSG_ACK) {
		
		
		
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
	const UdpClient *client = obj;
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_UNBIND_REQ) {
		C_DEVID AppId = *(C_DEVID*)(buffer+NTY_PROTO_UNBIND_APPID_IDX);
		C_DEVID DeviceId = *(C_DEVID*)(buffer+NTY_PROTO_UNBIND_DEVICEID_IDX);

#if ENABLE_CONNECTION_POOL
		int ret = ntyExecuteDevAppRelationDeleteHandle(AppId, DeviceId);
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
				Client *aclient = record->value;
				ASSERT(aclient != NULL);
				ntyVectorDelete(aclient->friends, &DeviceId);
			}

			record = ntyBHeapSelect(heap, DeviceId);
			if (record != NULL) {
				Client *dclient = record->value;
				ASSERT(dclient != NULL);
				ntyVectorDelete(dclient->friends, &DeviceId);
			}
#endif
		}
		ntyProtoUnBindAck(AppId, DeviceId, ret);
#endif		
		
		
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

void ntyBindDevicePacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	const Client *client = obj;
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_BIND_REQ) {
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
#if 0
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
#else

			void *heap = ntyBHeapInstance();
			NRecord *record = ntyBHeapSelect(heap, AppId);
			if (record != NULL) {
				Client *aclient = record->value;
				ASSERT(aclient != NULL);
				ntyVectorAdd(aclient->friends, &DeviceId, sizeof(C_DEVID));
			}

			record = ntyBHeapSelect(heap, DeviceId);
			if (record != NULL) {
				Client *dclient = record->value;
				ASSERT(dclient != NULL);
				ntyVectorAdd(dclient->friends, &DeviceId, sizeof(C_DEVID));
			}

#endif
		}
		ntyProtoBindAck(AppId, DeviceId, ret);
#endif		
		//if ()
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
	const Client *client = obj;
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_MULTICAST_REQ) {
		C_DEVID toId = 0;
		ntyU8ArrayToU64(buffer+NTY_PROTO_DEST_DEVID_IDX, &toId);

		void *pRBTree = ntyRBTreeInstance();
		Client *toClient = (Client*)ntyRBTreeInterfaceSearch(pRBTree, toId);
		if (toClient == NULL) { //no Exist
			return ;
		} 

		buffer[NTY_PROTO_MULTICAST_TYPE_IDX] = NTY_PROTO_DATAPACKET_REQ;
		//ntySendBuffer(toClient, buffer, length);
		//ntyMulticastSend();
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
	const Client *client = obj;
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_MULTICAST_ACK) {
		C_DEVID fromId = 0;
			
		ntyU8ArrayToU64(buffer+NTY_PROTO_DEST_DEVID_IDX, &fromId);

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
	const Client *client = obj;
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_LOCATION_ASYNCREQ) {
		U16 jsonlen = 0;
		U8 *jsonstring = NULL;
		memcpy(&jsonlen, buffer+16, sizeof(U16));
		jsonstring = buffer+18;
		buffer[length-4] = '\0';
		U16 jsonlenTemp = strlen(jsonstring);
		if (jsonlen != jsonlenTemp) {
			ntylog("JSON format error: %s\n", jsonstring);
			return;
		}

		C_DEVID fromId = *(C_DEVID*)(buffer+4);
		ntydbg("ntyLocationAsyncReqPacketHandleRequest --> json : %s\n", jsonstring);

		ntydbg("====================begin ntyLocationAsyncReqPacketHandleRequest action ==========================\n");
		
		JSON_Value *json = ntyMallocJsonValue(jsonstring);
		const char *category = ntyJsonAppCategory(json);
		if (strcmp(category, NATTY_USER_PROTOCOL_WIFI) == 0) {
			ntyJsonLocationWIFIAction(fromId, client->devId, json, jsonstring, jsonlen);
		} else if (strcmp(category, NATTY_USER_PROTOCOL_LAB) == 0) {
			ntyJsonLocationLabAction(fromId, client->devId, json, jsonstring, jsonlen);
		} else {
			ntylog("Can't find category with: %s\n", category);
		}
		ntyFreeJsonValue(json);
		
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
	const Client *client = obj;
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_WEATHER_ASYNCREQ) {
		U16 jsonlen = 0;
		U8 *jsonstring = NULL;
		memcpy(&jsonlen, buffer+12, sizeof(U16));
		jsonstring = buffer+14;
		buffer[length-4] = '\0';
		U16 jsonlenTemp = strlen(jsonstring);
		if (jsonlen != jsonlenTemp) {
			ntylog("JSON format error: %s\n", jsonstring);
			return;
		}

		C_DEVID clientId = *(C_DEVID*)(buffer+4);
		ntydbg("ntyWeatherAsyncReqPacketHandleRequest --> json : %s\n", jsonstring);
		ntydbg("====================begin ntyWeatherAsyncReqPacketHandleRequest action ==========================\n");
		
		JSON_Value *json = ntyMallocJsonValue(jsonstring);
		ntyJsonWeatherAction(clientId, client->devId, json, jsonstring, jsonlen);
		ntyFreeJsonValue(json);

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
	const Client *client = obj;
	if (buffer[NTY_PROTO_MSGTYPE_IDX] == NTY_PROTO_DATA_ROUTE) {
		U16 jsonlen = 0;
		char *jsonstring = NULL;
		memcpy(&jsonlen, buffer+24, 2);

		C_DEVID fromId = *(C_DEVID*)(buffer+NTY_PROTO_DATA_ROUTE_DEVID_IDX);
		C_DEVID toId = *(C_DEVID*)(buffer+NTY_PROTO_DATA_ROUTE_RECVID_IDX);
		
		jsonstring = buffer+26;
		buffer[length-4] = '\0';
		U16 jsonlenTemp = strlen(jsonstring);
		if (jsonlen != jsonlenTemp) {
			ntylog("JSON format error: %s\n", jsonstring);
			return;
		}
		ntydbg("ntyRoutePacketHandleRequest --> json : %s\n", jsonstring);
		ntydbg("====================begin ntyRoutePacketHandleRequest action ==========================\n");
		
		JSON_Value *json = ntyMallocJsonValue(jsonstring);
		const char *app_category = ntyJsonAppCategory(json);
		U16 app_jsonlen = strlen(app_category);
		if (app_jsonlen != 0) {
			ntydbg("Category : %s\n", app_category);
			if (strcmp(app_category, NATTY_USER_PROTOCOL_CATEGORY_CONFIG) == 0) {
				int len = ntySendDataRoute(toId, (U8*)buffer, strlen(buffer));
				if (len>0) {
					ntyJsonSuccessResult(fromId);
				}
			} else if (strcmp(app_category, NATTY_USER_PROTOCOL_CATEGORY_POWER) == 0) {				
				int len = ntySendDataRoute(toId, (U8*)buffer, strlen(buffer));
				if (len>0) {
					ntyJsonSuccessResult(fromId);
				}
			} else if (strcmp(app_category, NATTY_USER_PROTOCOL_CATEGORY_SIGNAL) == 0) {
				int len = ntySendDataRoute(toId, (U8*)buffer, strlen(buffer));
				if (len>0) {
					ntyJsonSuccessResult(fromId);
				}
			} else if (strcmp(app_category, NATTY_USER_PROTOCOL_CATEGORY_LOCATION) == 0) {
				int len = ntySendDataRoute(toId, (U8*)buffer, strlen(buffer));
				if (len>0) {
					ntyJsonSuccessResult(fromId);
				}
			} else if (strcmp(app_category, NATTY_USER_PROTOCOL_CATEGORY_FARE) == 0) {
				int len = ntySendDataRoute(toId, (U8*)buffer, strlen(buffer));
				if (len>0) {
					ntyJsonSuccessResult(fromId);
				}
			} else {
				ntylog("Can't find category with: %s\n", app_category);
			}
		}
		
		ntyFreeJsonValue(json);
		ntydbg("====================end ntyRoutePacketHandleRequest action ==========================\n");

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
	ntySetSuccessor(pBindDeviceFilter, pMutlcastReqFilter);

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
	const Client *client = obj;
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
		if (Record->value != NULL) {
			Client* pClient = Record->value;
			if (pClient != NULL) {
				pClient->rLength = 0;
			}
		}
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

