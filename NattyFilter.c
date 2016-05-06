

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
void ntyLoginPacketHandleRequest(const void *_self, unsigned char type, const void* obj) {
	const UdpClient *client = obj;
	if (type == NTY_PROTO_LOGIN_REQ) {
		fprintf(stdout, "Login deal with: %d\n", type);

		//send login ack
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, type, obj);
	} else {
		fprintf(stderr, "Can't deal with: %d\n", type);
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
void ntyHeartBeatPacketHandleRequest(const void *_self, unsigned char type, const void* obj) {
	const UdpClient *client = obj;
	if (type == NTY_PROTO_HEARTBEAT_REQ) {
		fprintf(stdout, "HeartBeat deal with: %d\n", type);

		//send heartbeat ack
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, type, obj);
	} else {
		fprintf(stderr, "Can't deal with: %d\n", type);
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
void ntyLogoutPacketHandleRequest(const void *_self, unsigned char type, const void* obj) {
	const UdpClient *client = obj;
	if (type == NTY_PROTO_LOGOUT_REQ) {
		fprintf(stdout, "Logout deal with: %d\n", type);

		//send logout ack
		
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, type, obj);
	} else {
		fprintf(stderr, "Can't deal with: %d\n", type);
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
void ntyP2PAddrReqPacketHandleRequest(const void *_self, unsigned char type, const void* obj) {
	const UdpClient *client = obj;
	if (type == NTY_PROTO_P2P_ADDR_REQ) {
		fprintf(stdout, "P2P addr Req deal with: %d\n", type);

		//send logout ack
		
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const ProtocolFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, type, obj);
	} else {
		fprintf(stderr, "Can't deal with: %d\n", type);
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

static void ntyHandleRequest(void *_filter, unsigned char type, const void *obj) {
	ProtocolFilter **filter = _filter;
	if (_filter && (*filter) && (*filter)->handleRequest) {
		(*filter)->handleRequest(_filter, type, obj);
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
	return ntyHandleRequest(_filter, buffer[NTY_PROTO_TYPE_IDX], obj);
}

void ntyProtocolFilterRelease(void *_filter) {
	Packet *self = _filter;
	if (ntyPacketGetSuccessor(self) != NULL) {
		ntyProtocolFilterRelease(self->succ);
	}
	Delete(self);
}

