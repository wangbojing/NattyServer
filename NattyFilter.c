

#include "NattyRBTree.h"
#include "NattyFilter.h"


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

/*
 * Login Packet
 */
void ntyLoginPacketHandleRequest(const void *_self, unsigned char *buffer, const void* obj) {
	const UdpClient *client = obj;
	if (buffer[NTY_PROTO_TYPE_IDX] == NTY_PROTO_LOGIN_REQ) {

		void *pRBTree = ntyRBTreeInstance();
		unsigned int key = *(unsigned int*)(buffer+NTY_PROTO_DEVID_IDX);
		unsigned int ackNum = *(unsigned int*)(buffer+NTY_PROTO_ACKNUM_IDX)+1;
		const UdpClient *client = obj;
		unsigned char ack[NTY_HEARTBEAT_ACK_LENGTH] = {0};

		UdpClient *cliValue = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, key);
		if (cliValue == NULL) {			
			UdpClient *pClient = (UdpClient*)malloc(sizeof(UdpClient));
			pClient->sockfd = client->sockfd;
			pClient->addr.sin_addr.s_addr = client->addr.sin_addr.s_addr;

			if (ntyRBTreeInterfaceInsert(pRBTree, key, pClient)) {
				fprintf(stdout, "Client is Exist\n");
				free(pClient);
			}
		}
		
		fprintf(stdout, "Login deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);		
		//send login ack
		ack[NTY_PROTO_TYPE_IDX] = NTY_PROTO_LOGIN_ACK;
		memcpy(ack+1, &ackNum, NTY_ACKNUM_LENGTH);
		ntySendBuffer(client, ack, NTY_HEARTBEAT_ACK_LENGTH);

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
		unsigned int key = *(unsigned int*)(buffer+NTY_PROTO_DEVID_IDX);
		unsigned int ackNum = *(unsigned int*)(buffer+NTY_PROTO_ACKNUM_IDX)+1;
		const UdpClient *client = obj;
		unsigned char ack[NTY_HEARTBEAT_ACK_LENGTH] = {0};

		UdpClient *cliValue = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, key);
		if (cliValue == NULL) {			
			UdpClient *pClient = (UdpClient*)malloc(sizeof(UdpClient));
			pClient->sockfd = client->sockfd;
			pClient->addr.sin_addr.s_addr = client->addr.sin_addr.s_addr;

			if (ntyRBTreeInterfaceInsert(pRBTree, key, pClient)) {
				fprintf(stdout, "Client is Exist\n");
				free(pClient);
			}
		} else if (!ntyClientCompare(client, cliValue)) {
			UdpClient *pClient = (UdpClient*)malloc(sizeof(UdpClient));
			pClient->sockfd = client->sockfd;
			pClient->addr.sin_addr.s_addr = client->addr.sin_addr.s_addr;

			if (ntyRBTreeInterfaceUpdate(pRBTree, key, pClient)) {
				fprintf(stdout, "Client Update failed\n");
				free(pClient);
			}
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
		unsigned int key = *(unsigned int*)(buffer+NTY_PROTO_DEVID_IDX);
		unsigned int ackNum = *(unsigned int*)(buffer+NTY_PROTO_ACKNUM_IDX)+1;
		const UdpClient *client = obj;
		unsigned char ack[NTY_HEARTBEAT_ACK_LENGTH] = {0};

		ntyRBTreeInterfaceDelete(pRBTree, key);
		fprintf(stdout, "Logout deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);

		//send logout ack
		ack[NTY_PROTO_TYPE_IDX] = NTY_PROTO_LOGOUT_ACK;
		memcpy(ack+1, &ackNum, NTY_ACKNUM_LENGTH);
		ntySendBuffer(client, ack, NTY_HEARTBEAT_ACK_LENGTH);
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
		fprintf(stdout, "P2P addr Req deal with: %d\n", buffer[NTY_PROTO_TYPE_IDX]);

		//send P2P ack
		
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

	return pHeartBeatFilter;
}

void ntyProtocolFilterProcess(void *_filter, unsigned char *buffer, const void *obj) {
	//data length is right

	//data crc is right

	return ntyHandleRequest(_filter, buffer, obj);
}

void ntyProtocolFilterRelease(void *_filter) {
	Packet *self = _filter;
	if (ntyPacketGetSuccessor(self) != NULL) {
		ntyProtocolFilterRelease(self->succ);
	}
	Delete(self);
}

