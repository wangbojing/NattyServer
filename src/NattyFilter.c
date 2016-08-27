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


/*
 * Login Packet
 * Login Packet, HeartBeatPacket, LogoutPacket etc. should use templete designer pattern
 */
void ntyLoginPacketHandleRequest(const void *_self, unsigned char *buffer, int length, const void* obj) {
	const UdpClient *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_LOGIN_REQ) {
		int i = 0;
		void *pRBTree = ntyRBTreeInstance();
		C_DEVID key = *(C_DEVID*)(buffer+NTY_PROTO_LOGIN_REQ_DEVID_IDX);
		U32 ackNum = *(U32*)(buffer+NTY_PROTO_LOGIN_REQ_ACKNUM_IDX)+1;
		
		U8 ack[NTY_LOGIN_ACK_LENGTH] = {0};

		UdpClient *cliValue = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, key);
		if (cliValue == NULL) {			
			UdpClient *pClient = (UdpClient*)malloc(sizeof(UdpClient));
			pClient->sockfd = client->sockfd;
			pClient->clientType = client->clientType;
			pClient->addr.sin_addr.s_addr = client->addr.sin_addr.s_addr;
			pClient->addr.sin_port = client->addr.sin_port;
			pClient->ackNum = ackNum;

			//new firends tree			
			pClient->friends = ntyFriendsTreeInstance();
			//read from disk friends and store in rb-tree
#if 1	//just for debug		
			ntylog(" key : %lld\n", key);
			if (key == 1) {
				ntyFriendsTreeInsert(pClient->friends, 2);
				//ntyFriendsTreeInsert(pClient->friends, 3);
			} else if (key == 2) {
				ntyFriendsTreeInsert(pClient->friends, 1);
				//ntyFriendsTreeInsert(pClient->friends, 3);
			}
#else
#endif
			//insert rb-tree
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
			
			return ;
		} else if ((cliValue != NULL) && (!ntyClientCompare (client, cliValue))) {
			UdpClient *pClient = cliValue;//(UdpClient*)malloc(sizeof(UdpClient));
			pClient->sockfd = client->sockfd;
			pClient->clientType = client->clientType;
			pClient->addr.sin_addr.s_addr = client->addr.sin_addr.s_addr;
			pClient->addr.sin_port = client->addr.sin_port;
			pClient->ackNum = ackNum;
			
			//ntyClientFriendsList(pClient, ack);

			//friends is null and update friends tree
			if (pClient->friends != NULL) {
				if (ntyFriendsTreeIsEmpty(pClient->friends)) {
#if 1	//just for debug		
					if (key == 1) {
						ntyFriendsTreeInsert(pClient->friends, 2);
						//ntyFriendsTreeInsert(pClient->friends, 3);
					} else if (key == 2) {
						ntyFriendsTreeInsert(pClient->friends, 1);
						//ntyFriendsTreeInsert(pClient->friends, 3);
					}
#else
#endif				
				}	
			} else {
				pClient->friends = ntyFriendsTreeInstance();
#if 1	//just for debug		
				if (key == 1) {
					ntyFriendsTreeInsert(pClient->friends, 2);
					//ntyFriendsTreeInsert(pClient->friends, 3);
				} else if (key == 2) {
					ntyFriendsTreeInsert(pClient->friends, 1);
					//ntyFriendsTreeInsert(pClient->friends, 3);
				}
#else
#endif
			}

			//ntylog(" keys : %d");
			//notify friends 
			//ntyFriendsTreeTraversalNotify(pClient->friends, key, ntyNotifyFriendMessage);
			//send friends list to client
			ntySendFriendsTreeIpAddr(pClient, 1);

			return ;
		}
		//ntylog("Login deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);		
		//send login ack
		
		ack[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
		ack[NTY_PROTO_MESSAGE_TYPE] = (U8)MSG_ACK;
		ack[NTY_PROTO_TYPE_IDX] = NTY_PROTO_LOGIN_ACK;
		//memcpy(ack+NTY_PROTO_LOGIN_ACK_ACKNUM_IDX, &ackNum, NTY_ACKNUM_LENGTH);
		*(U32*)(&ack[NTY_PROTO_ACKNUM_IDX]) = ackNum;
		ntySendBuffer(client, ack, length);

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
			//new firends tree			
			pClient->friends = ntyFriendsTreeInstance();
			//determine this key is available and read from disk friends 

#if 1	//just for debug		
			//if (key != 1 || key != 2) return ;
			if (key == 1) {
				ntyFriendsTreeInsert(pClient->friends, 2);
				//ntyFriendsTreeInsert(pClient->friends, 3);
			} else if (key == 2) {
				ntyFriendsTreeInsert(pClient->friends, 1);
				//ntyFriendsTreeInsert(pClient->friends, 3);
			}
#else
#endif	
			ntylog("Insert New Client Node\n");
			//insert rb-tree
			if (ntyRBTreeInterfaceInsert(pRBTree, key, pClient)) {
				ntylog("Client is Exist\n");
				free(pClient);
				return ;
			}
			
			//notify friends 
			//ntyFriendsTreeTraversal(pClient->friends, ntyNotifyFriendConnect);
			//ntyFriendsTreeTraversalNotify(pClient->friends, key, ntyNotifyFriendMessage);

			//send friends list to client 
			//ntyFriendsTreeTraversal(pClient->friends, ntySendFriendIpAddr);
			ntySendFriendsTreeIpAddr(pClient, 0);
			
			return ;
		} else if ((cliValue != NULL) && !ntyClientCompare(client, cliValue)) {
			int i = 0, length = 0;
			UdpClient *pClient = cliValue;//(UdpClient*)malloc(sizeof(UdpClient));
			pClient->sockfd = client->sockfd;
			pClient->clientType = client->clientType;
			pClient->addr.sin_addr.s_addr = client->addr.sin_addr.s_addr;
			pClient->addr.sin_port = client->addr.sin_port;
			pClient->ackNum = ackNum;
			
			ntylog("HeartBeat Update Info: %d\n", buffer[NTY_PROTO_TYPE_IDX]);

			//friends is null and update friends tree
			if (pClient->friends != NULL) {
				if (ntyFriendsTreeIsEmpty(pClient->friends)) {
#if 1	//just for debug		
					if (key == 1) {
						ntyFriendsTreeInsert(pClient->friends, 2);
						//ntyFriendsTreeInsert(pClient->friends, 3);
					} else if (key == 2) {
						ntyFriendsTreeInsert(pClient->friends, 1);
						//ntyFriendsTreeInsert(pClient->friends, 3);
					}
#else
#endif				
				}	
			} else {
				pClient->friends = ntyFriendsTreeInstance();
#if 1	//just for debug		
				if (key == 1) {
					ntyFriendsTreeInsert(pClient->friends, 2);
					//ntyFriendsTreeInsert(pClient->friends, 3);
				} else if (key == 2) {
					ntyFriendsTreeInsert(pClient->friends, 1);
					//ntyFriendsTreeInsert(pClient->friends, 3);
				}
#else
#endif
			}

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
 * CONTENT COUNT				2			 BYTE 
 * CONTENT					*(CONTENT COUNT)	 BYTE 
 * CRC 				 	4			 BYTE (Network Module Set Value)
 * 
 * send to server addr, proxy to send one client
 * 
 */

void ntyUserDataPacketHandleRequest(const void *_self, unsigned char *buffer, int length,const void* obj) {
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_DATAPACKET_REQ) {
		U8 data[RECV_BUFFER_SIZE] = {0};
		
		C_DEVID destDevId = *(C_DEVID*)(&buffer[NTY_PROTO_DATAPACKET_DEST_DEVID_IDX]);
		void *pRBTree = ntyRBTreeInstance();
		UdpClient *destClient = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, destDevId);
		const UdpClient *client = obj;
		
		U16 recByteCount = *(U16*)(&buffer[NTY_PROTO_DATAPACKET_CONTENT_COUNT_IDX]);

		memcpy(data, buffer+NTY_PROTO_DATAPACKET_CONTENT_IDX, recByteCount);
#if 1
		if (destClient == NULL) {
			ntylog("destClient is no exist\n");
			//C_DEVID selfDevId = *(C_DEVID*)(&buffer[NTY_PROTO_DEVID_IDX]);
			buffer[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_RET;
			ntySendBuffer(client, buffer, length);
		} else {
			ntylog(" Proxy data: %s, destDevId:%lld %d.%d.%d.%d:%d\n", data, destDevId, *(unsigned char*)(&destClient->addr.sin_addr.s_addr),
				*((unsigned char*)(&destClient->addr.sin_addr.s_addr)+1), *((unsigned char*)(&destClient->addr.sin_addr.s_addr)+2), 
				*((unsigned char*)(&destClient->addr.sin_addr.s_addr)+3), destClient->addr.sin_port);
		
#if 0
			ntySendBuffer(destClient, buffer, length);
#else
			if (0 >= ntyClassifyMessageType(client->devId, destClient->devId, data, recByteCount)) {
				ntySendBuffer(destClient, buffer, length);
			}
#endif
		}
#else
		int ret = ntyClassifyMessageType(client->devId, data, recByteCount);
		ntydbg("ret : %d\n", ret);
#endif
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

	ntySetSuccessor(pHeartBeatFilter, pLoginFilter);
	ntySetSuccessor(pLoginFilter, pLogoutFilter);
	ntySetSuccessor(pLogoutFilter, pP2PAddrReqFilter);
	ntySetSuccessor(pP2PAddrReqFilter, pUserDataPacketFilter);
	
	ntySetSuccessor(pUserDataPacketFilter, pUserDataPacketAckFilter);
	ntySetSuccessor(pUserDataPacketAckFilter, pP2PConnectNotifyPacketFilter);
	ntySetSuccessor(pP2PConnectNotifyPacketFilter, pTimeCheckFilter);
	ntySetSuccessor(pTimeCheckFilter, NULL);

	/*
	 * add your Filter
	 * for example:
	 * void *pFilter = New(NtyFilter);
	 * ntySetSuccessor(pLogoutFilter, pFilter);
	 */

	//Gen Crc Table
	ntyGenCrcTable();

	return pHeartBeatFilter;
}

void ntyProtocolFilterProcess(void *_filter, unsigned char *buffer, U32 length, const void *obj) {
	
	//data crc is right, and encryto
	U32 u32Crc = ntyGenCrcValue(buffer, length-4);
	U32 u32ClientCrc = *((U32*)(buffer+length-4));

	//ntydbg(" client:%x, server:%x, length:%d", u32ClientCrc, u32Crc, length);
	if (u32Crc != u32ClientCrc) {
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
void ntyProtoHttpProxyTransform(C_DEVID fromId, C_DEVID toId, U8 *buf, int length) {
	int n = 0;
	void *pRBTree = ntyRBTreeInstance();
	UdpClient *destClient = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, toId);
	if (destClient == NULL) {
		ntylog(" destClient is not exist\n");
		return ;
	}

	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_REQ;	
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_DATAPACKET_REQ;
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEVID_IDX]) = fromId;
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEST_DEVID_IDX]) = toId;
	
	*(U16*)(&buf[NTY_PROTO_DATAPACKET_CONTENT_COUNT_IDX]) = (U16)length;
	length += NTY_PROTO_DATAPACKET_CONTENT_IDX;

	*(U32*)(&buf[length]) = ntyGenCrcValue(buf, length);
	length += sizeof(U32);

	ntySendBuffer(destClient, buf, length);
	
}

#endif



