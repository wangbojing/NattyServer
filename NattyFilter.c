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
 ****		*****
   ***	  *
   ***        * 					    *			    *
   * ** 	  * 					    *			    *
   * ** 	  * 					    *			    *
   *  **	  * 					   **			   **
   *  **	  * 					  ***			  ***
   *   **	  * 	    ******	          ***********	   ***********	    *****         *****
   *   **	  * 	  **	    **		   **			   **			**		 **
   *	  **	  *    **		**		   **			   **			 **		 *
   *	  **	  *    **		**		   **			   **			  *		*
   *	   **   *    **		**		   **			   **			   ** 	*
   *	   **   * 		     ***		   **			   **			    *       *
   *	    **  * 	      ***** **		   **			   **			    **     *
   *	    **  * 	  *** 	**		   **			   **			    **     *
   *	     ** *    **		**		   **			   **			     *    *
   *	     ** *   **		**		   **			   **			     **  *
   *		***   **		**		   **			   **			       **
   *		***   **		**		   **	    * 	   **	    * 		 **
   *		 **   **		**	*	   **	    * 	   **	    * 		 **
   *		 **    **	  ****	*	    **   *		    **   *			 *
 *****		  *******	 ***		     ****		     ****			 *
														 *
														 *
													   *****
 *													   ****
 *
 *
 */


#include "NattyRBTree.h"
#include "NattyFilter.h"




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

static U32* ntyClientListIterator(const void *_self) {
	const SingleList *self = _self;
	Node **p = &self->head->next;
	U32* pClientList = (U32*)malloc(self->count * sizeof(U32));

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
		printf(" %d ", (*p)->clientId);
		p = &(*p)->next;
	}
	printf("\n");
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

U32* Iterator(void *self) {
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
	*(U32*)(&notify[NTY_PROTO_P2P_NOTIFY_IPADDR_IDX]) = client->addr.sin_addr.s_addr;
	*(U16*)(&notify[NTY_PROTO_P2P_NOTIFY_IPPORT_IDX]) = client->addr.sin_port;

	length = NTY_PROTO_P2P_NOTIFY_CRC_IDX;
	*(U32*)(&notify[length]) = ntyGenCrcValue(notify, length);
	length += sizeof(U32);

	notify[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2P_NOTIFY_REQ;
	
	ntySendBuffer(client, notify, length);

	return ;
}

static int ntyUpdateClientListIpAddr(UdpClient *client, U32 key, U32 ackNum) {
	int i = 0;
	UdpClient *pClient = client;
	void *pRBTree = ntyRBTreeInstance();

	U32* friendsList = Iterator(pClient->clientList);
	int count = ((SingleList*)pClient->clientList)->count;
	for (i = 0;i < count;i ++) {
		UdpClient *cv = ntyRBTreeInterfaceSearch(pRBTree, *(friendsList+i));
		if (cv != NULL) {
			
			U8 ackNotify[NTY_P2P_NOTIFY_ACK_LENGTH] = {0};
			*(U32*)(&ackNotify[NTY_PROTO_P2P_NOTIFY_DEVID_IDX]) = key;
			*(U32*)(&ackNotify[NTY_PROTO_P2P_NOTIFY_ACKNUM_IDX]) = ackNum;

			ntyP2PNotifyClient(cv, ackNotify);
		}

	}
	free(friendsList);
}

static void ntyClientFriendsList(UdpClient *client, U8 *ack) {
	int i = 0, length;
	void *pRBTree = ntyRBTreeInstance();
	UdpClient *pClient = client;

	U32* friendsList = Iterator(pClient->clientList);
	int count = ((SingleList*)pClient->clientList)->count;
	for (i = 0;i < count;i ++) {
		UdpClient *cv = ntyRBTreeInterfaceSearch(pRBTree, *(friendsList+i));
		if (cv != NULL) {			
			*(U32*)(&ack[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_ADDR_IDX(i)]) = (U32)(cv->addr.sin_addr.s_addr);
			*(U16*)(&ack[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_PORT_IDX(i)]) = (U16)(cv->addr.sin_port);
		}
		*(U16*)(&ack[NTY_PROTO_LOGIN_ACK_FRIENDS_COUNT_IDX]) = (U16)count;
		*(U32*)(&ack[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_DEVID_IDX(i)]) = *(friendsList+i);
				
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
void ntyLoginPacketHandleRequest(const void *_self, unsigned char *buffer, const void* obj) {
	const UdpClient *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_LOGIN_REQ) {
		int i = 0, length;
		void *pRBTree = ntyRBTreeInstance();
		unsigned int key = *(unsigned int*)(buffer+NTY_PROTO_LOGIN_REQ_DEVID_IDX);
		unsigned int ackNum = *(unsigned int*)(buffer+NTY_PROTO_LOGIN_REQ_ACKNUM_IDX)+1;
		const UdpClient *client = obj;
		unsigned char ack[NTY_LOGIN_ACK_LENGTH] = {0};

		UdpClient *cliValue = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, key);
		if (cliValue == NULL) {			
			UdpClient *pClient = (UdpClient*)malloc(sizeof(UdpClient));
			pClient->sockfd = client->sockfd;
			pClient->addr.sin_addr.s_addr = client->addr.sin_addr.s_addr;
			pClient->addr.sin_port = client->addr.sin_port;
			
			pClient->clientList = New(pNtyClientList);
			//set devid friends list
#if 1 //Debug
			//read from disk or mysql by key
			if (key == 1) {
				Insert(pClient->clientList, 2);				
			} else if (key == 2) {
				Insert(pClient->clientList, 1);
			}
#endif
			ack[NTY_PROTO_TYPE_IDX] = NTY_PROTO_LOGIN_ACK;
			*(U32*)(&ack[NTY_PROTO_LOGIN_ACK_ACKNUM_IDX]) = ackNum;
			
			ntyClientFriendsList(pClient, ack);
			ntyUpdateClientListIpAddr(pClient, key, ackNum);

			if (ntyRBTreeInterfaceInsert(pRBTree, key, pClient)) {
				fprintf(stdout, "Client is Exist\n");
				free(pClient);
			}
		} else if ((cliValue != NULL) && (!ntyClientCompare (client, cliValue))) {
			UdpClient *pClient = cliValue;//(UdpClient*)malloc(sizeof(UdpClient));
			pClient->sockfd = client->sockfd;
			pClient->addr.sin_addr.s_addr = client->addr.sin_addr.s_addr;
			pClient->addr.sin_port = client->addr.sin_port;
			
			ntyUpdateClientListIpAddr(pClient, key, ackNum); //notify all friends dev
		}
		
		fprintf(stdout, "Login deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);		
		//send login ack
		ack[NTY_PROTO_TYPE_IDX] = NTY_PROTO_LOGIN_ACK;
		//memcpy(ack+NTY_PROTO_LOGIN_ACK_ACKNUM_IDX, &ackNum, NTY_ACKNUM_LENGTH);
		*(U32*)(&ack[NTY_PROTO_LOGIN_ACK_ACKNUM_IDX]) = ackNum;
		ntySendBuffer(client, ack, length);

	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, obj);
	} else {
		fprintf(stderr, "Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
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
void ntyHeartBeatPacketHandleRequest(const void *_self, unsigned char *buffer, const void* obj) {
	const UdpClient *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_HEARTBEAT_REQ) {

		void *pRBTree = ntyRBTreeInstance();
		unsigned int key = *(unsigned int*)(buffer+NTY_PROTO_HEARTBEAT_DEVID_IDX);
		unsigned int ackNum = *(unsigned int*)(buffer+NTY_PROTO_HEARTBEAT_ACKNUM_IDX)+1;
		const UdpClient *client = obj;
		unsigned char ack[NTY_HEARTBEAT_ACK_LENGTH] = {0};

		UdpClient *cliValue = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, key);
		if (cliValue == NULL) {			
			UdpClient *pClient = (UdpClient*)malloc(sizeof(UdpClient));
			pClient->sockfd = client->sockfd;
			pClient->addr.sin_addr.s_addr = client->addr.sin_addr.s_addr;
			pClient->addr.sin_port = client->addr.sin_port;
			
			pClient->clientList = New(pNtyClientList);
						//set devid friends list
#if 1 //Debug
			//read from disk or mysql by key
			if (key == 1) {
				Insert(pClient->clientList, 2);
			} else if (key == 2) {
				Insert(pClient->clientList, 1);
			}
#endif
			if (ntyRBTreeInterfaceInsert(pRBTree, key, pClient)) {
				fprintf(stdout, "Client is Exist\n");
				free(pClient);
			}
		} else if ((cliValue != NULL) && !ntyClientCompare(client, cliValue)) {
			int i = 0, length = 0;
			UdpClient *pClient = cliValue; //(UdpClient*)malloc(sizeof(UdpClient));
			pClient->sockfd = client->sockfd;
			pClient->addr.sin_addr.s_addr = client->addr.sin_addr.s_addr;
			pClient->addr.sin_port = client->addr.sin_port;

			ntyUpdateClientListIpAddr(pClient, key, ackNum); //notify all friends dev
		}
		
		fprintf(stdout, "HeartBeat deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
		
		//send heartbeat ack
		ack[NTY_PROTO_TYPE_IDX] = NTY_PROTO_HEARTBEAT_ACK;
		memcpy(ack+1, &ackNum, NTY_ACKNUM_LENGTH);
		ntySendBuffer(client, ack, NTY_HEARTBEAT_ACK_LENGTH);

	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, obj);
	} else {
		fprintf(stderr, "Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
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
void ntyLogoutPacketHandleRequest(const void *_self, unsigned char *buffer, const void* obj) {
	const UdpClient *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_LOGOUT_REQ) {
		//delete key
		void *pRBTree = ntyRBTreeInstance();
		unsigned int key = *(unsigned int*)(buffer+NTY_PROTO_HEARTBEAT_DEVID_IDX);
		unsigned int ackNum = *(unsigned int*)(buffer+NTY_PROTO_HEARTBEAT_ACKNUM_IDX)+1;
		const UdpClient *client = obj;
		unsigned char ack[NTY_HEARTBEAT_ACK_LENGTH] = {0};


		UdpClient *cliValue = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, key);
		if (cliValue != NULL) {
			Delete(cliValue->clientList);
		}
		
		ntyRBTreeInterfaceDelete(pRBTree, key);
		fprintf(stdout, "Logout deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);

		//send logout ack
		ack[NTY_PROTO_TYPE_IDX] = NTY_PROTO_LOGOUT_ACK;
		memcpy(ack+1, &ackNum, NTY_ACKNUM_LENGTH);
		ntySendBuffer(client, ack, NTY_LOGOUT_ACK_LENGTH);
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, obj);
	} else {
		fprintf(stderr, "Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
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
void ntyP2PAddrReqPacketHandleRequest(const void *_self, unsigned char *buffer, const void* obj) {
	const UdpClient *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_ADDR_REQ) {
		int i = 0, length;
		void *pRBTree = ntyRBTreeInstance();

		U32 key = *(unsigned int*)(buffer+NTY_PROTO_HEARTBEAT_DEVID_IDX);
		U32 ackNum = *(unsigned int*)(buffer+NTY_PROTO_HEARTBEAT_ACKNUM_IDX)+1;

		const UdpClient *client = obj;
		U16 count = *(U16*)(&buffer[NTY_PROTO_P2PADDR_REQ_FRIENDS_COUNT_IDX]);
		U8 ack[NTY_P2PADDR_ACK_LENGTH] = {0};
		
		//deal with P2P addr Req
		for (i = 0;i < count;i ++) {
			U32 reqKey = *(U32*)(&buffer[NTY_PROTO_P2PADDR_REQ_FRIENDS_DEVID_IDX(i)]);
			UdpClient *cliValue = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, reqKey);
			if (cliValue != NULL) {
				*(U32*)(&ack[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_ADDR_IDX(i)]) = (U32)(cliValue->addr.sin_addr.s_addr);
				*(U16*)(&ack[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_PORT_IDX(i)]) = (U16)(cliValue->addr.sin_port);

				
				U8 ackNotify[NTY_P2P_NOTIFY_ACK_LENGTH] = {0};
				*(U32*)(&ackNotify[NTY_PROTO_P2P_NOTIFY_DEVID_IDX]) = key;
				*(U32*)(&ackNotify[NTY_PROTO_P2P_NOTIFY_ACKNUM_IDX]) = ackNum;

				ntyP2PNotifyClient(cliValue, ackNotify);				
			}
			*(U16*)(&ack[NTY_PROTO_LOGIN_ACK_FRIENDS_COUNT_IDX]) = (U16)count;
			*(U32*)(&ack[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_DEVID_IDX(i)]) = reqKey;				
		}
		//send P2P ack

		if (count == 0) {
			length = NTY_PROTO_P2PADDR_ACK_FRIENDSLIST_START_IDX;
			*(U32*)&ack[length] = ntyGenCrcValue(ack, length+1);				
		} else {
			length = NTY_PROTO_P2PADDR_ACK_CRC_IDX(count);
			*(U32*)&ack[length] = ntyGenCrcValue(ack, length+1);				
		}
		length = length + 1 + sizeof(U32);
		fprintf(stdout, "P2P addr Req deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);

		ack[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2P_ADDR_ACK;
		//memcpy(ack+NTY_PROTO_LOGIN_ACK_ACKNUM_IDX, &ackNum, NTY_ACKNUM_LENGTH);
		*(U32*)(&ack[NTY_PROTO_LOGIN_ACK_ACKNUM_IDX]) = ackNum;
		ntySendBuffer(client, ack, length);
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, buffer, obj);
	} else {
		fprintf(stderr, "Can't deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);
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



static void ntySetSuccessor(void *_filter, void *_succ) {
	ProtocolFilter **filter = _filter;
	if (_filter && (*filter) && (*filter)->setSuccessor) {
		(*filter)->setSuccessor(_filter, _succ);
	}
}

static void ntyHandleRequest(void *_filter, unsigned char *buffer, const void *obj) {
	ProtocolFilter **filter = _filter;
	if (_filter && (*filter) && (*filter)->handleRequest) {
		(*filter)->handleRequest(_filter, buffer, obj);
	}
}



const void *pNtyLoginFilter = &ntyLoginFilter;
const void *pNtyHeartBeatFilter = &ntyHeartBeatFilter;
const void *pNtyLogoutFilter = &ntyLogoutFilter;
const void *pNtyP2PAddrReqFilter = &ntyP2PAddrFilter;



void* ntyProtocolFilterInit(void) {
	void *pHeartBeatFilter = New(pNtyHeartBeatFilter);
	void *pLoginFilter = New(pNtyLoginFilter);
	void *pLogoutFilter = New(pNtyLogoutFilter);
	void *pP2PAddrReqFilter = New(pNtyP2PAddrReqFilter);


	ntySetSuccessor(pHeartBeatFilter, pLoginFilter);
	ntySetSuccessor(pLoginFilter, pLogoutFilter);
	ntySetSuccessor(pLogoutFilter, pP2PAddrReqFilter);

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
	//data length is right

	//data crc is right
	U32 u32Crc = ntyGenCrcValue(buffer, length-4);
	U32 u32ClientCrc = *((U32*)(buffer+length-4));
	if (u32Crc != u32ClientCrc) {
		return ;
	}

	return ntyHandleRequest(_filter, buffer, obj);
}

void ntyProtocolFilterRelease(void *_filter) {
	Packet *self = _filter;
	if (ntyPacketGetSuccessor(self) != NULL) {
		ntyProtocolFilterRelease(self->succ);
	}
	Delete(self);
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



