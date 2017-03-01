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
#include <arpa/inet.h>
#include <netdb.h> 
#include <string.h>
#include <pthread.h>
#include <unistd.h>


#include "NattyProtoClient.h"
#include "NattyTimer.h"
#include "NattyUtils.h"
#include "NattyNetwork.h"
#include "NattyResult.h"
#include "NattyVector.h"


/* ** **** ******** **************** Global Variable **************** ******** **** ** */
static NWTimer *nHeartBeatTimer = NULL;
static NWTimer *nReconnectTimer = NULL;
static NWTimer *nBigBufferSendTimer = NULL;
static NWTimer *nBigBufferRecvTimer = NULL;


static int ntyHeartBeatCb (NITIMER_ID id, void *user_data, int len);

static void ntySetupRecvProcThread(void *self);
static int ntySendLogin(void *self);
static int ntySendLogout(void *self);
static void ntySendTimeout(int len);
static void* ntyRecvProc(void *arg);

void *ntyProtoInstance(void);
void ntyProtoRelease(void);


static int ntySendBigBuffer(void *self, U8 *u8Buffer, int length, C_DEVID gId);


#if 1 //

typedef void* (*RECV_CALLBACK)(void *arg);

typedef enum {
	STATUS_NETWORK_LOGIN,
	STATUS_NETWORK_PROXYDATA,
	STATUS_NETWORK_LOGOUT,
} StatusNetwork;

typedef struct _NATTYPROTOCOL {
	const void *_;
	C_DEVID selfId; //App Or Device Id
	C_DEVID fromId; //store ack devid
#if (NTY_PROTO_SELFTYPE==NTY_PROTO_CLIENT_IOS)
	U8 tokens[NORMAL_BUFFER_SIZE];
	U8 tokenLen;
#endif
	void *friends;
	U8 recvBuffer[RECV_BUFFER_SIZE];
	U16 recvLen;
	RECV_CALLBACK onRecvCallback; //recv
	PROXY_CALLBACK onProxyCallback; //just for java
	PROXY_CALLBACK onProxyFailed; //send data failed
	PROXY_CALLBACK onProxySuccess; //send data success
	PROXY_CALLBACK onProxyDisconnect;
	PROXY_CALLBACK onProxyReconnect;
	PROXY_CALLBACK onBindResult;
	PROXY_CALLBACK onUnBindResult;
	PROXY_CALLBACK onPacketRecv;
	PROXY_CALLBACK onPacketSuccess;
	
#if 1 //Natty Protocol v3.2

	NTY_PARAM_CALLBACK onLoginAckResult; //RECV LOGIN_ACK
	NTY_STATUS_CALLBACK onHeartBeatAckResult; //RECV HEARTBEAT_ACK
	NTY_STATUS_CALLBACK onLogoutAckResult; //RECV LOGOUT_ACK
	NTY_PARAM_CALLBACK onTimeAckResult; //RECV TIME_ACK
	NTY_PARAM_CALLBACK onICCIDAckResult; //RECV ICCID_ACK
	NTY_RETURN_CALLBACK onCommonReqResult; //RECV COMMON_REQ
#if 0 //discard
	NTY_PARAM_CALLBACK onCommonAckResult; //RECV COMMON_ACK
#endif
	NTY_STATUS_CALLBACK onVoiceDataAckResult; //RECV VOICE_DATA_ACK
	NTY_PARAM_CALLBACK onOfflineMsgAckResult; //RECV OFFLINE_MSG_ACK
	NTY_PARAM_CALLBACK onLocationPushResult; //RECV LOCATION_PUSH
	NTY_PARAM_CALLBACK onWeatherPushResult; //RECV WEATHER_PUSH
	NTY_RETURN_CALLBACK onDataRoute; //RECV DATA_RESULT
	NTY_STATUS_CALLBACK onDataResult; //RECV DATA_RESULT
	NTY_RETURN_CALLBACK onVoiceBroadCastResult; //RECV VOICE_BROADCAST
	NTY_RETURN_CALLBACK onLocationBroadCastResult; //RECV LOCATION_BROADCAST
	NTY_RETURN_CALLBACK onCommonBroadCastResult; //RECV COMMON_BROADCAST
	
#endif
	pthread_t recvThreadId;
	U8 u8RecvExitFlag;
	U8 u8ConnectFlag;

} NattyProto;

typedef struct _NATTYPROTO_OPERA {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	int (*login)(void *_self); //argument is optional
	int (*logout)(void *_self); //argument is optional
#if 0
	void (*proxyReq)(void *_self, C_DEVID toId, U8 *buffer, int length);
	void (*proxyAck)(void *_self, C_DEVID retId, U32 ack);
	void (*fenceReq)(void *_self, C_DEVID toId, U8 *buffer, int length);
	void (*fenceAck)(void *_self, C_DEVID friId, U32 ack);
#else
	int (*voiceReq)(void *_self, U8 *json, U16 length);
	int (*voiceAck)(void *_self, U8 *json, U16 length);
	int (*voiceDataReq)(void *_self, C_DEVID gId, U8 *data, int length);
	int (*commonReq)(void *_self, C_DEVID gId, U8 *json, U16 length);
	int (*commonAck)(void *_self, U8 *json, U16 length);
	int (*offlineMsgReq)(void *_self);
	int (*dataRoute)(void *_self, C_DEVID toId, U8 *json, U16 length);
#endif
	int (*bind)(void *_self, C_DEVID did);
	int (*unbind)(void *_self, C_DEVID did);

} NattyProtoOpera;

typedef NattyProtoOpera NattyProtoHandle;

#endif
#if 1
extern DEVID g_devid;
#endif

void* ntyProtoClientCtor(void *_self, va_list *params) {
	NattyProto *proto = _self;

	proto->onRecvCallback = ntyRecvProc;
	proto->selfId = g_devid;
	proto->recvLen = 0;
	memset(proto->recvBuffer, 0, RECV_BUFFER_SIZE);
	//proto->friends = ntyVectorCreator();

	ntyGenCrcTable();
	//Setup Socket Connection
	Network *network = ntyNetworkInstance();
	if (network->sockfd < 0) { //Connect failed
		proto->u8ConnectFlag = 0;
	} else {
		proto->u8ConnectFlag = 1;
	}

	//Create Timer
	void *nTimerList = ntyTimerInstance();
	nHeartBeatTimer = ntyTimerAdd(nTimerList, HEARTBEAT_TIME_TICK, ntyHeartBeatCb, NULL, 0);

	return proto;
}

void* ntyProtoClientDtor(void *_self) {
	NattyProto *proto = _self;

	proto->onRecvCallback = NULL;
	proto->recvLen = 0;

	//Cancel Timer
	void *nTimerList = ntyTimerInstance();
	ntyTimerDel(nTimerList, nHeartBeatTimer);

	//Release Socket Connection
	ntyNetworkRelease();
	
	//ntyVectorDestory(proto->friends);
	proto->u8ConnectFlag = 0;
	proto->u8RecvExitFlag = 0;

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
static int ntyHeartBeatCb (NITIMER_ID id, void *user_data, int len) {
	NattyProto *proto = ntyProtoInstance();
	ClientSocket *nSocket = ntyNetworkInstance();
	int length, n;
	U8 buffer[NTY_HEARTBEAT_ACK_LENGTH] = {0};	

	bzero(buffer, NTY_HEARTBEAT_ACK_LENGTH);
	
	if (proto->selfId == 0) {//set devid
		trace("[%s:%s:%d] selfId == 0\n", __FILE__, __func__, __LINE__);
		return NTY_RESULT_FAILED;
	} 
	if (proto->u8ConnectFlag == 0) {
		trace("[%s:%s:%d] socket don't connect to server\n", __FILE__, __func__, __LINE__);
		return NTY_RESULT_FAILED;
	}
	
	buffer[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;	
	buffer[NTY_PROTO_PROTOTYPE_IDX] = (U8) PROTO_REQ;	
	buffer[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_HEARTBEAT_REQ;	
#if 0
	*(C_DEVID*)(&buffer[NTY_PROTO_DEVID_IDX]) = proto->devid;
#else
	memcpy(buffer+NTY_PROTO_DEVID_IDX, &proto->selfId, sizeof(C_DEVID));
#endif
	length = NTY_PROTO_HEARTBEAT_REQ_CRC_IDX+sizeof(U32);
	
	n = ntySendFrame(nSocket, buffer, length);

	return n;
}

#if (NTY_PROTO_SELFTYPE==NTY_PROTO_CLIENT_IOS)
void ntyProtoClientSetToken(void *_self, U8 *tokens, int length) {
	NattyProto *proto = _self;

	memcpy(proto->tokens, tokens, length);
	proto->tokenLen = (U16)length;
}
#endif

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
int ntyProtoClientLogin(void *_self) {
	NattyProto *proto = _self;
	int len;	
	U8 buffer[RECV_BUFFER_SIZE] = {0};	

	buffer[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;	
	buffer[NTY_PROTO_PROTOTYPE_IDX] = (U8) MSG_REQ;	
	buffer[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_LOGIN_REQ;
#if 0
	*(C_DEVID*)(&buffer[NTY_PROTO_DEVID_IDX]) = proto->devid;
#else
	memcpy(buffer+NTY_PROTO_LOGIN_REQ_DEVID_IDX, &proto->selfId, sizeof(C_DEVID));
#endif

#if (NTY_PROTO_SELFTYPE==NTY_PROTO_CLIENT_IOS)
	memcpy(buffer+NTY_PROTO_LOGIN_REQ_JSON_LENGTH_IDX, &proto->tokenLen, sizeof(U16));
	memcpy(buffer+NTY_PROTO_LOGIN_REQ_JSON_CONTENT_IDX, &proto->tokens, proto->tokenLen);
	len = NTY_PROTO_LOGIN_REQ_JSON_CONTENT_IDX+proto->tokenLen+4;
#else
	len = NTY_PROTO_LOGIN_REQ_CRC_IDX+sizeof(U32);		
#endif
		

	ntydbg(" ntyProtoClientLogin %d\n", __LINE__);
	ClientSocket *nSocket = ntyNetworkInstance();
	return ntySendFrame(nSocket, buffer, len);
}

int ntyProtoClientBind(void *_self, C_DEVID did) {
	NattyProto *proto = _self;
	int len;	

	U8 buf[NORMAL_BUFFER_SIZE] = {0};	

	buf[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;	
	buf[NTY_PROTO_PROTOTYPE_IDX] = (U8) PROTO_REQ;	
	buf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_BIND_REQ;

	memcpy(buf+NTY_PROTO_BIND_APPID_IDX, &proto->selfId, sizeof(C_DEVID));
	*(C_DEVID*)(&buf[NTY_PROTO_BIND_DEVICEID_IDX]) = did;
	len = NTY_PROTO_BIND_CRC_IDX + sizeof(U32);

	ntydbg(" ntyProtoClientBind --> ");

	ClientSocket *nSocket = ntyNetworkInstance();
	return ntySendFrame(nSocket, buf, len);
}

int ntyProtoClientUnBind(void *_self, C_DEVID did) {
	NattyProto *proto = _self;
	int len;	

	U8 buf[NORMAL_BUFFER_SIZE] = {0};	

	buf[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;	
	buf[NTY_PROTO_PROTOTYPE_IDX] = (U8) PROTO_REQ;	
	buf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_UNBIND_REQ;

	memcpy(buf+NTY_PROTO_UNBIND_APPID_IDX, &proto->selfId, sizeof(C_DEVID));
	memcpy(buf+NTY_PROTO_UNBIND_DEVICEID_IDX, &did, sizeof(C_DEVID));
	//*(C_DEVID*)(&buf[NTY_PROTO_UNBIND_DEVICEID_IDX]) = did;
	len = NTY_PROTO_UNBIND_CRC_IDX + sizeof(U32);

	ClientSocket *nSocket = ntyNetworkInstance();
	return ntySendFrame(nSocket, buf, len);
}

int ntyProtoClientLogout(void *_self) {
	NattyProto *proto = _self;
	int len;	
	U8 buf[RECV_BUFFER_SIZE] = {0};	

	buf[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;	
	buf[NTY_PROTO_PROTOTYPE_IDX] = (U8) PROTO_REQ;	
	buf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_LOGOUT_REQ;
	memcpy(buf+NTY_PROTO_LOGOUT_REQ_DEVID_IDX, &proto->selfId, sizeof(C_DEVID));
	
	len = NTY_PROTO_LOGOUT_REQ_CRC_IDX+sizeof(U32);				

	ClientSocket *nSocket = ntyNetworkInstance();
	return ntySendFrame(nSocket, buf, len);
}

/*
 * 
 */
int ntyProtoClientVoiceReq(void *_self, U8 *json, U16 length) {
	NattyProto *proto = _self;
	U8 buf[RECV_BUFFER_SIZE] = {0}; 
	
	buf[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;
	buf[NTY_PROTO_PROTOTYPE_IDX] = (U8) PROTO_REQ; 
	buf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_VOICE_REQ;

	memcpy(&buf[NTY_PROTO_VOICE_REQ_DEVID_IDX], &proto->selfId, sizeof(C_DEVID));
	memcpy(&buf[NTY_PROTO_VOICE_REQ_JSON_LENGTH_IDX], &length, sizeof(U16));
	memcpy(&buf[NTY_PROTO_VOICE_REQ_JSON_CONTENT_IDX], json, length);

	length = NTY_PROTO_VOICE_REQ_JSON_CONTENT_IDX+length+4;

	void *pNetwork = ntyNetworkInstance();
	return ntySendFrame(pNetwork, buf, length);
}

/*
 * @Param: void *_self, NattyProto Instance
 * @Param: U8 *json, json format data
 * @Param: U16 length, json length
 * 
 */
int ntyProtoClientVoiceAck(void *_self, U8 *json, U16 length) {
	NattyProto *proto = _self;
	U8 buf[RECV_BUFFER_SIZE] = {0}; 
	
	buf[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;
	buf[NTY_PROTO_PROTOTYPE_IDX] = (U8) PROTO_REQ; 
	buf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_VOICE_ACK;

	memcpy(&buf[NTY_PROTO_VOICE_ACK_DEVID_IDX], &proto->selfId, sizeof(C_DEVID));
	memcpy(&buf[NTY_PROTO_VOICE_ACK_JSON_LENGTH_IDX], &length, sizeof(U16));
	memcpy(&buf[NTY_PROTO_VOICE_ACK_JSON_CONTENT_IDX], json, length);

	length = NTY_PROTO_VOICE_ACK_JSON_CONTENT_IDX+length+sizeof(U32);

	void *pNetwork = ntyNetworkInstance();
	return ntySendFrame(pNetwork, buf, length);
}

int ntyProtoClientVoiceDataReq(void *_self, C_DEVID gId, U8 *data, int length) {
	NattyProto *proto = _self;

	ntySendBigBuffer(proto, data, length, gId);
}

int ntyProtoClientCommonReq(void *_self, C_DEVID gId, U8 *json, U16 length) {
	NattyProto *proto = _self;
	U8 buf[RECV_BUFFER_SIZE] = {0}; 

	buf[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;
	buf[NTY_PROTO_PROTOTYPE_IDX] = (U8) PROTO_REQ; 
	buf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_COMMON_REQ;
	
	memcpy(&buf[NTY_PROTO_COMMON_REQ_DEVID_IDX], &proto->selfId, sizeof(C_DEVID));
	memcpy(&buf[NTY_PROTO_COMMON_REQ_RECVID_IDX], &gId, sizeof(C_DEVID));

	memcpy(&buf[NTY_PROTO_COMMON_REQ_JSON_LENGTH_IDX], &length, sizeof(U16));
	memcpy(&buf[NTY_PROTO_COMMON_REQ_JSON_CONTENT_IDX], json, length);

	length = NTY_PROTO_COMMON_REQ_JSON_CONTENT_IDX+length+sizeof(U32);
	
	void *pNetwork = ntyNetworkInstance();
	return ntySendFrame(pNetwork, buf, length);
}


int ntyProtoClientCommonAck(void *_self, U8 *json, U16 length) {
	NattyProto *proto = _self;

	U8 buf[RECV_BUFFER_SIZE] = {0};
	buf[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;
	buf[NTY_PROTO_PROTOTYPE_IDX] = (U8) PROTO_REQ; 
	buf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_COMMON_ACK;

	memcpy(&buf[NTY_PROTO_COMMON_ACK_DEVID_IDX], &proto->selfId, sizeof(C_DEVID));
	memcpy(&buf[NTY_PROTO_COMMON_ACK_JSON_LENGTH_IDX], &length, sizeof(U16));

	memcpy(&buf[NTY_PROTO_COMMON_ACK_JSON_CONTENT_IDX], json, length);

	length = NTY_PROTO_COMMON_ACK_JSON_CONTENT_IDX+length+sizeof(U32);

	void *pNetwork = ntyNetworkInstance();
	return ntySendFrame(pNetwork, buf, length);
}

int ntyProtoClientOfflineMsgReq(void *_self) {
	NattyProto *proto = _self;
	int len = 0;
	U8 buf[RECV_BUFFER_SIZE] = {0}; 

	buf[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;
	buf[NTY_PROTO_PROTOTYPE_IDX] = (U8) PROTO_REQ; 
	buf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_OFFLINE_MSG_REQ;

	memcpy(&buf[NTY_PROTO_OFFLINE_MSG_REQ_DEVICEID_IDX], &proto->selfId, sizeof(C_DEVID));
	len = NTY_PROTO_OFFLINE_MSG_REQ_CRC_IDX+sizeof(U32);

	void *pNetwork = ntyNetworkInstance();
	return ntySendFrame(pNetwork, buf, len);	
	
}

int ntyProtoClientDataRoute(void *_self, C_DEVID toId, U8 *json, U16 length) {
	NattyProto *proto = _self;
	U8 buf[RECV_BUFFER_SIZE] = {0}; 

	buf[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;
	buf[NTY_PROTO_PROTOTYPE_IDX] = (U8) PROTO_ROUTE; 
	buf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_DATA_ROUTE;

	memcpy(&buf[NTY_PROTO_DATA_ROUTE_DEVID_IDX], &proto->selfId, sizeof(C_DEVID));
	memcpy(&buf[NTY_PROTO_DATA_ROUTE_RECVID_IDX], &toId, sizeof(C_DEVID));

	memcpy(&buf[NTY_PROTO_DATA_ROUTE_JSON_LENGTH_IDX], &length, sizeof(U16));
	memcpy(&buf[NTY_PROTO_DATA_ROUTE_JSON_CONTENT_IDX], json, length);

	length = NTY_PROTO_COMMON_REQ_JSON_CONTENT_IDX+length+sizeof(U32);

	void *pNetwork = ntyNetworkInstance();
	return ntySendFrame(pNetwork, buf, length);
}


static const NattyProtoHandle ntyProtoOpera = {
	sizeof(NattyProto),
	ntyProtoClientCtor,
	ntyProtoClientDtor,
	ntyProtoClientLogin,
	ntyProtoClientLogout,
#if 0
	ntyProtoClientProxyReq,
	ntyProtoClientProxyAck,
	ntyProtoClientEfenceReq,
	ntyProtoClientEfenceAck,
#else
	ntyProtoClientVoiceReq,
	ntyProtoClientVoiceAck,
	ntyProtoClientVoiceDataReq,
	ntyProtoClientCommonReq,
	ntyProtoClientCommonAck,
	ntyProtoClientOfflineMsgReq,
	ntyProtoClientDataRoute,
#endif
	ntyProtoClientBind,
	ntyProtoClientUnBind,
};

const void *pNattyProtoOpera = &ntyProtoOpera;

static NattyProto *pProtoOpera = NULL;

void *ntyProtoInstance(void) { //Singleton
	if (pProtoOpera == NULL) {
		ntydbg("ntyProtoInstance\n");
		pProtoOpera = New(pNattyProtoOpera);
		if (pProtoOpera->u8ConnectFlag == 0) { //Socket Connect Failed
			Delete(pProtoOpera);
			pProtoOpera = NULL;
		}
	}
	return pProtoOpera;
}

void ntyProtoRelease(void) {
	if (pProtoOpera != NULL) {
		Delete(pProtoOpera);
		pProtoOpera = NULL;
	}
}


static void ntySetupRecvProcThread(void *self) {
	//NattyProtoOpera * const * protoOpera = self;
	NattyProto *proto = self;
	int err;
	//pthread_t recvThread_id;

	if (self && proto && proto->onRecvCallback) {	
		if (proto->recvThreadId != 0) {
			ntydbg(" recv thread is running \n");
			return ;
		}
		
		err = pthread_create(&proto->recvThreadId, NULL, proto->onRecvCallback, self);				
		if (err != 0) { 				
			ntydbg(" can't create thread:%s\n", strerror(err)); 
			return ;	
		}
	}
}

static int ntySendLogin(void *self) {
	NattyProtoOpera * const * protoOpera = self;

	ntydbg(" ntySendLogin %d\n", __LINE__);
	if (self && (*protoOpera) && (*protoOpera)->login) {
		return (*protoOpera)->login(self);
	}
}

static int ntySendLogout(void *self) {
	NattyProtoOpera * const * protoOpera = self;

	if (self && (*protoOpera) && (*protoOpera)->logout) {
		return (*protoOpera)->logout(self);
	}
}




#if 0
int ntySendDataPacket(C_DEVID toId, U8 *data, int length) {
	int n = 0;
	void *self = ntyProtoInstance();
	NattyProtoOpera * const *protoOpera = self;
	NattyProto *proto = self;
	
	U8 buf[NTY_PROXYDATA_PACKET_LENGTH] = {0};
	
#if 0
	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	buf[NTY_PROTO_PROTOTYPE_IDX] = (U8) MSG_REQ;	
	buf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_DATAPACKET_REQ;
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
	ntydbg(" toId : %lld \n", toId);
#if 0
	if (proto && (*protoOpera) && (*protoOpera)->proxyReq) {
		(*protoOpera)->proxyReq(proto, toId, buf, length);
		return 0;
	}
#endif
	return -1;
#endif
	
}

int ntySendMassDataPacket(U8 *data, int length) {	
	void *pTree = ntyRBTreeInstance();
	
	ntyFriendsTreeMass(pTree, ntySendDataPacket, data, length);

	return 0;
}


#endif

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

void ntySetProxyDisconnect(PROXY_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->onProxyDisconnect = cb;
	}
}

void ntySetProxyReconnect(PROXY_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->onProxyReconnect = cb;
	}
}

void ntySetBindResult(PROXY_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->onBindResult = cb;
	}
}

void ntySetUnBindResult(PROXY_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->onUnBindResult = cb;
	}
}

void ntySetPacketRecv(PROXY_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->onPacketRecv = cb;
	}
}

void ntySetPacketSuccess(PROXY_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->onPacketSuccess = cb;
	}
}


void ntySetDevId(C_DEVID id) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->selfId = id;
	}
}

void ntySetLoginAckResult(NTY_PARAM_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->onLoginAckResult = cb;
	}
}

void ntySetHeartBeatAckResult(NTY_STATUS_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->onHeartBeatAckResult = cb;
	}
}

void ntySetLogoutAckResult(NTY_STATUS_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->onLogoutAckResult = cb;
	}
}

void ntySetTimeAckResult(NTY_PARAM_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->onTimeAckResult = cb;
	}
}

void ntySetICCIDAckResult(NTY_PARAM_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->onICCIDAckResult = cb;
	}
}

void ntySetCommonReqResult(NTY_RETURN_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->onCommonReqResult = cb;
	}
}

void ntySetVoiceDataAckResult(NTY_STATUS_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->onVoiceDataAckResult = cb;
	}
}

void ntySetOfflineMsgAckResult(NTY_PARAM_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->onOfflineMsgAckResult = cb;
	}
}

void ntySetLocationPushResult(NTY_PARAM_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->onLocationPushResult = cb;
	}
}

void ntySetWeatherPushResult(NTY_PARAM_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->onWeatherPushResult = cb;
	}
}

void ntySetDataRoute(NTY_RETURN_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->onDataRoute = cb;
	}
}

void ntySetDataResult(NTY_STATUS_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->onDataResult = cb;
	}
}

void ntySetVoiceBroadCastResult(NTY_RETURN_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->onVoiceBroadCastResult = cb;
	}
}

void ntySetLocationBroadCastResult(NTY_RETURN_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->onLocationBroadCastResult = cb;
	}
}

void ntySetCommonBroadCastResult(NTY_RETURN_CALLBACK cb) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->onCommonBroadCastResult = cb;
	}
}


int ntyGetNetworkStatus(void) {
	void *network = ntyNetworkInstance();
	return ntyGetSocket(network);
}


int ntyCheckProtoClientStatus(void) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
#if 0
		if (proto->onProxyCallback == NULL) return -2;
		if (proto->onProxyFailed == NULL) return -3;
		if (proto->onProxySuccess == NULL) return -4;
#endif
		if (proto->onProxyDisconnect == NULL) return -5;
		if (proto->onProxyReconnect == NULL) return -6;
		if (proto->onBindResult == NULL) return -7;
		if (proto->onUnBindResult == NULL) return -8;
		if (proto->onRecvCallback == NULL) return -9;
		if (proto->onPacketRecv == NULL) return -10;
		if (proto->onPacketSuccess == NULL) return -11;
	}
	return 0;
}

void* ntyStartupClient(int *status) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		ntySendLogin(proto);
		ntySetupRecvProcThread(proto); //setup recv proc
	}

	*status = ntyGetNetworkStatus();
	
	return proto;
}

void ntyShutdownClient(void) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		ntySendLogout(proto);
	}
}

#if 1

int ntyBindClient(C_DEVID did) {
	NattyProto* proto = ntyProtoInstance();

	if (proto) {
		return ntyProtoClientBind(proto, did);
	}
	return -1;
}

int ntyUnBindClient(C_DEVID did) {
	NattyProto* proto = ntyProtoInstance();

	if (proto) {
		return ntyProtoClientUnBind(proto, did);
	}
	return -1;
}

int ntyVoiceReqClient(U8 *json, U16 length) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		return ntyProtoClientVoiceReq(proto, json, length);
	}
	return -1;
}

int ntyVoiceAckClient(U8 *json, U16 length) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		return ntyProtoClientVoiceAck(proto, json, length);
	}
	return -1;
}

int ntyVoiceDataReqClient(C_DEVID gId, U8 *data, int length) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		return ntyProtoClientVoiceDataReq(proto, gId, data, length);
	}
	return -1;
}

int ntyCommonReqClient(C_DEVID gId, U8 *json, U16 length) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		return ntyProtoClientCommonReq(proto, gId, json, length);
	}
	return -1;
}


int ntyCommonAckClient(U8 *json, U16 length) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		return ntyProtoClientCommonAck(proto, json, length);
	}
	return -1;
}

int ntyDataRouteClient(C_DEVID toId, U8 *json, U16 length) {
	NattyProto* proto = ntyProtoInstance();

	if (proto) {
		return ntyProtoClientDataRoute(proto, toId, json, length);
	}
	return -1;
}






#endif

U8* ntyGetRecvBuffer(void) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		proto->recvBuffer[NTY_PROTO_DATAPACKET_CONTENT_IDX+proto->recvLen] = 0x0;
		return proto->recvBuffer+NTY_PROTO_DATAPACKET_CONTENT_IDX;
	}
	return NULL;
}

int ntyGetRecvBufferSize(void) {
	NattyProto* proto = ntyProtoInstance();
	if (proto) {
		return proto->recvLen;
	}
	return -1;
}
#if 0
static void ntySendTimeout(int len) {
	NattyProto* proto = ntyProtoInstance();
	if (proto && proto->onProxyFailed) {
		proto->onProxyFailed(STATUS_TIMEOUT);
	}

	//void* pTimer = ntyNetworkTimerInstance();
	//ntyStopTimer(pTimer);
}
#endif

static int ntyReconnectCb(NITIMER_ID id, void *user_data, int len) {
	int status = 0;
	
	trace(" ntyReconnectCb ...\n");
	NattyProto *proto = ntyStartupClient(&status);
	if (status != -1 && (proto != NULL)) {
		//NattyProto *proto = ntyProtoInstance();
		trace(" ntyReconnectCb ... status:%d, flag:%d\n", status, proto->u8ConnectFlag);
		if (proto->u8ConnectFlag) { //Reconnect Success
			if (proto->onProxyReconnect)
				proto->onProxyReconnect(0);
			//Stop Timer
#if 1
			trace(" Stop Timer\n");
			void *nTimerList = ntyTimerInstance();
			ntyTimerDel(nTimerList, nReconnectTimer);
			nReconnectTimer = NULL;
#endif
		}
	}

	return NTY_RESULT_SUCCESS;
}

#if 0

C_DEVID* ntyGetFriendsList(int *Count) {
	NattyProto* proto = ntyProtoInstance();

	//C_DEVID *list = ntyVectorGetNodeList(proto->friends, Count);

	return NULL;
}

void ntyReleaseFriendsList(C_DEVID **list) {
	C_DEVID *pList = *list;
	free(pList);
	pList = NULL;
}
#endif

void ntyStartReconnectTimer(void) {
	void *nTimerList = ntyTimerInstance();
	nReconnectTimer = ntyTimerAdd(nTimerList, RECONNECT_TIME_TICK, ntyReconnectCb, NULL, 0);
}


#if 1


int u32DataLength = 0;
static U8 u8RecvBigBuffer[NTY_BIGBUFFER_SIZE] = {0};
static U8 u8SendBigBuffer[NTY_BIGBUFFER_SIZE] = {0};
void* tBigTimer = NULL;
void* tEfenceTimer = NULL;
void* tProxyTimer = NULL;


int ntySendVoicePacket(void *self, U8 *buffer, int length, C_DEVID toId) {
	U16 Count = length / NTY_VOICEREQ_PACKET_LENGTH + 1 ;
	U32 pktLength = NTY_VOICEREQ_DATA_LENGTH, i;
	U8 *pkt = buffer;
	void *pNetwork = ntyNetworkInstance();
	NattyProto* proto = self;
	int ret = -1;

	LOG(" destId:%d, pktIndex:%d, pktTotal:%d", NTY_PROTO_VOICEREQ_DESTID_IDX,
		NTY_PROTO_VOICEREQ_PKTINDEX_IDX, NTY_PROTO_VOICEREQ_PKTTOTLE_IDX);
	
	for (i = 0;i < Count;i ++) {
		pkt = buffer+(i*NTY_VOICEREQ_PACKET_LENGTH);

		pkt[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;
		pkt[NTY_PROTO_DEVTYPE_IDX] = NTY_PROTO_CLIENT_ANDROID;
		pkt[NTY_PROTO_PROTOTYPE_IDX] = (U8) MSG_REQ;	
		pkt[NTY_PROTO_VOICEREQ_TYPE_IDX] = NTY_PROTO_VOICE_REQ;

		memcpy(pkt+NTY_PROTO_VOICEREQ_SELFID_IDX, &proto->selfId, sizeof(C_DEVID));
		memcpy(pkt+NTY_PROTO_VOICEREQ_DESTID_IDX, &toId, sizeof(C_DEVID));

		memcpy(pkt+NTY_PROTO_VOICEREQ_PKTINDEX_IDX, &i, sizeof(U16));
		memcpy(pkt+NTY_PROTO_VOICEREQ_PKTTOTLE_IDX, &Count , sizeof(U16));

		if (i == Count-1) { //last packet
			pktLength = (length % NTY_VOICEREQ_PACKET_LENGTH) - NTY_VOICEREQ_EXTEND_LENGTH;
		}

		memcpy(pkt+NTY_PROTO_VOICEREQ_PKTLENGTH_IDX, &pktLength, sizeof(U32));
		
		ret = ntySendFrame(pNetwork, pkt, pktLength+NTY_VOICEREQ_EXTEND_LENGTH);

		LOG(" index : %d", i );
		LOG(" pktLength:%d, Count:%d, ret:%d, selfIdx:%d\n",
			pktLength+NTY_VOICEREQ_EXTEND_LENGTH, Count, ret, NTY_PROTO_VOICEREQ_SELFID_IDX);

		usleep(200 * 1000); //Window Send
	}

	return 0;
}

int ntyAudioPacketEncode(U8 *pBuffer, int length) {
	int i = 0, j = 0, k = 0, idx = 0;
	int pktCount = length / NTY_VOICEREQ_DATA_LENGTH;
	int pktLength = pktCount * NTY_VOICEREQ_PACKET_LENGTH + (length % NTY_VOICEREQ_DATA_LENGTH) + NTY_VOICEREQ_EXTEND_LENGTH;
	//U8 *pktIndex = pBuffer + pktCount * NTY_VOICEREQ_PACKET_LENGTH;

	LOG("pktLength :%d, pktCount:%d\n", pktLength, pktCount);
	if (pktCount >= (NTY_VOICEREQ_COUNT_LENGTH-1)) return -1;

	j = pktLength - NTY_CRCNUM_LENGTH;
	k = length;

	LOG("j :%d, k :%d, pktCount:%d, last:%d\n", j, k, pktCount, (length % NTY_VOICEREQ_DATA_LENGTH));
	for (idx = 0;idx < (length % NTY_VOICEREQ_DATA_LENGTH);idx ++) {
		pBuffer[--j] = pBuffer[--k];
	}	
			

	for (i = pktCount;i > 0;i --) {
		j = i * NTY_VOICEREQ_PACKET_LENGTH - NTY_CRCNUM_LENGTH;
		k = i * NTY_VOICEREQ_DATA_LENGTH;
		LOG("j :%d, k :%d\n", j, k);
		for (idx = 0;idx < NTY_VOICEREQ_DATA_LENGTH;idx ++) {
			pBuffer[--j] = pBuffer[--k];
		}
	}

	return pktLength;
}


int ntyGetRecvBigLength(void) {
	return u32DataLength;
}

U8 *ntyGetRecvBigBuffer(void) {
	return u8RecvBigBuffer;
}

U8 *ntyGetSendBigBuffer(void) {
	return u8SendBigBuffer;
}

static int ntySendBigBufferCb(NITIMER_ID id, void *user_data, int len) {
	NattyProto* proto = ntyProtoInstance();
	if (proto && proto->onPacketSuccess) {
		proto->onPacketSuccess(1); //Failed
#if 0
		if (tBigTimer != -1) {
			del_timer(tBigTimer);
			tBigTimer = -1;
		}
#else
		if (nBigBufferSendTimer != NULL) {
			void *nTimerList = ntyTimerInstance();
			
			ntyTimerDel(nTimerList, nBigBufferSendTimer);
			nBigBufferSendTimer = NULL;
		}
#endif
	}

	return 0;
}

static int ntySendBigBuffer(void *self, U8 *u8Buffer, int length, C_DEVID gId) {
	int i = 0;
#if 0
	tBigTimer = add_timer(10, ntySendBigBufferCb, NULL, 0);
#else
	void *nTimerList = ntyTimerInstance();
	nBigBufferSendTimer = ntyTimerAdd(nTimerList, PACKET_SEND_TIME_TICK, ntySendBigBufferCb, NULL, 0);
#endif
	int ret = ntyAudioPacketEncode(u8Buffer, length);
	LOG(" ntySendBigBuffer --> Ret %d, %x", ret, u8Buffer[0]);

	ntySendVoicePacket(self, u8Buffer, length, gId);
#if 0
	C_DEVID tToId = 0;
	memcpy(&tToId, u8Buffer+NTY_PROTO_VOICEREQ_DESTID_IDX, sizeof(C_DEVID));
	LOG(" ntySendBigBuffer --> toId : %lld, %d", tToId, NTY_PROTO_VOICEREQ_DESTID_IDX);
#endif
	return 0;
}

int ntyAudioRecodeDepacket(U8 *buffer, int length) {
	int i = 0;
	U8 *pBuffer = ntyGetRecvBigBuffer();	
	U16 index = ntyU8ArrayToU16(buffer+NTY_PROTO_VOICEREQ_PKTINDEX_IDX);
	U16 Count = ntyU8ArrayToU16(&buffer[NTY_PROTO_VOICEREQ_PKTTOTLE_IDX]);
	U32 pktLength = ntyU8ArrayToU32(&buffer[NTY_PROTO_VOICEREQ_PKTLENGTH_IDX]);

	//nty_printf(" Count:%d, index:%d, pktLength:%d, length:%d, pktLength%d\n", 
	//				Count, index, pktLength, length, NTY_PROTO_VOICEREQ_PKTLENGTH_IDX);

	if (length != pktLength+NTY_VOICEREQ_EXTEND_LENGTH) return 2;

	
	for (i = 0;i < pktLength;i ++) {
		pBuffer[index * NTY_VOICEREQ_DATA_LENGTH + i] = buffer[NTY_VOICEREQ_HEADER_LENGTH + i];
	}

	if (index == Count-1) {
		u32DataLength = NTY_VOICEREQ_DATA_LENGTH*(Count-1) + pktLength;
		return 1;
	}

	return 0;
}



void ntyPacketClassifier(void *arg, U8 *buf, int length) {
	NattyProto *proto = arg;
	NattyProtoOpera * const *protoOpera = arg;
	Network *pNetwork = ntyNetworkInstance();
	U8 MSG = buf[NTY_PROTO_MSGTYPE_IDX];

	switch (MSG) {
		case NTY_PROTO_LOGIN_ACK: {

			U16 status = *(U16*)(buf+NTY_PROTO_LOGIN_ACK_STATUS_IDX);
			U16 jsonLen = *(U16*)(buf+NTY_PROTO_LOGIN_ACK_JSON_LENGTH_IDX);
			U8 *json = buf+NTY_PROTO_LOGIN_ACK_JSON_CONTENT_IDX;

			LOG(" LoginAckResult status:%d\n", status);
			proto->onLoginAckResult(json, jsonLen);
			break;
		} 
		case NTY_PROTO_BIND_ACK: {

			int result = 0;
			memcpy(&result, &buf[NTY_PROTO_BIND_ACK_RESULT_IDX], sizeof(int));

			if (proto->onBindResult) {
				proto->onBindResult(result);
			}
			ntydbg(" NTY_PROTO_BIND_ACK\n");
			break;
		} 
		case NTY_PROTO_UNBIND_ACK: {

			int result = 0;
			memcpy(&result, &buf[NTY_PROTO_BIND_ACK_RESULT_IDX], sizeof(int));

			if (proto->onUnBindResult) {
				proto->onUnBindResult(result);
			}
			ntydbg(" NTY_PROTO_UNBIND_ACK\n");
			break;
		} 
		case NTY_PROTO_HEARTBEAT_ACK: {
			
			break;
		} 
		case NTY_PROTO_LOGOUT_ACK: {
			
			break;
		}
#if (NTY_PROTO_SELFTYPE == NTY_PROTO_CLIENT_WATCH)
		case NTY_PROTO_TIME_CHECK_ACK: {
			//set system time
			break;
		}
		case NTY_PROTO_ICCID_ACK: {
			U16 status = 0;
			U16 length = 0;
			U8 *json = NULL;

			memcpy(&status, buf+NTY_PROTO_ICCID_ACK_STATUS_IDX, sizeof(U16));
			memcpy(&length, buf+NTY_PROTO_ICCID_ACK_JSON_LENGTH_IDX, sizeof(U16));
			json = buf+NTY_PROTO_ICCID_ACK_JSON_CONTENT_IDX;

			if (proto->onICCIDAckResult) {
				proto->onICCIDAckResult(json, length);
			}
			
			break;
		}
#endif
		case NTY_PROTO_COMMON_REQ: {
			C_DEVID fromId = 0;
			U16 length = 0;
			U8 *json = NULL;

			ntyU8ArrayToU64(buf+NTY_PROTO_COMMON_REQ_DEVID_IDX, &fromId);
			memcpy(&length, buf+NTY_PROTO_COMMON_REQ_JSON_LENGTH_IDX, sizeof(U16));

			json = buf+NTY_PROTO_COMMON_REQ_JSON_CONTENT_IDX;

			if (proto->onCommonReqResult) {
				proto->onCommonReqResult(fromId, json, length);
			}
			
			break;
		}
		case NTY_PROTO_VOICE_DATA_REQ: {
			int ret = ntyAudioRecodeDepacket(buf, length);
			if (ret == 1) {
				C_DEVID fromId = 0;
				ntyU8ArrayToU64(buf+NTY_PROTO_VOICE_DATA_REQ_DEVID_IDX, &fromId);

				if (proto->onPacketRecv) {
					proto->onPacketRecv(u32DataLength);
				}
			}
			break;
		}
		case NTY_PROTO_VOICE_DATA_ACK: {
			U16 status = 0;

			memcpy(&status, buf+NTY_PROTO_VOICE_DATA_ACK_STATUS_IDX, sizeof(U16));

			//send voice data success
			if (proto->onVoiceDataAckResult) {
				proto->onVoiceDataAckResult(status);
			}
			break;
		}
		case NTY_PROTO_OFFLINE_MSG_ACK: {

			U16 jsonLen = *(U16*)(buf+NTY_PROTO_OFFLINE_MSG_ACK_JSON_LENGTH_IDX);
			U8 *json = buf+NTY_PROTO_OFFLINE_MSG_ACK_JSON_CONTENT_IDX;

			if (proto->onOfflineMsgAckResult) {
				proto->onOfflineMsgAckResult(json, jsonLen);
			}
			
			break;
		}
		case NTY_PROTO_DATA_ROUTE: {
			C_DEVID fromId = 0;
			
			memcpy(&fromId, buf+NTY_PROTO_DATA_ROUTE_DEVID_IDX, sizeof(C_DEVID));

			U16 jsonLen = *(U16*)(buf+NTY_PROTO_DATA_ROUTE_JSON_LENGTH_IDX);
			U8 *json = buf+NTY_PROTO_DATA_ROUTE_JSON_CONTENT_IDX;

			if (proto->onDataRoute) {
				proto->onDataRoute(fromId, json, jsonLen);
			}
			
			break;
		}
		case NTY_PROTO_DATA_RESULT: {
			U16 status = 0;
			int ackNum = 0;

			memcpy(&status, buf+NTY_PROTO_DATA_RESULT_STATUS_IDX, sizeof(U16));
			memcpy(&ackNum, buf+NTY_PROTO_DATA_RESULT_ACKNUM_IDX, sizeof(U32));
			LOG("Data Result:%d\n", status);
		
			if (proto->onDataResult) {
				proto->onDataResult(ackNum);
			}
			
			break;
		}
		case NTY_PROTO_VOICE_BROADCAST: { //Recv Voice Notify
			C_DEVID fromId = 0;
			U8 *json = NULL;
			U16 length = 0;
			
			memcpy(&fromId, buf+NTY_PROTO_VOICE_BROADCAST_DEVID_IDX, sizeof(DEVID));
			memcpy(&length, buf+NTY_PROTO_VOICE_BROADCAST_JSON_LENGTH_IDX, sizeof(U16));

			json = buf+NTY_PROTO_VOICE_BROADCAST_JSON_CONTENT_IDX;

			// voice data notify
			if (proto->onVoiceBroadCastResult) {
				proto->onVoiceBroadCastResult(fromId, json, length);
			}
			
			break;
		}
		case NTY_PROTO_LOCATION_BROADCAST: {
			C_DEVID fromId = 0;
			U8 *json = NULL;
			U16 length = 0;
			
			memcpy(&fromId, buf+NTY_PROTO_LOCATION_BROADCAST_DEVID_IDX, sizeof(C_DEVID));
			memcpy(&length, buf+NTY_PROTO_LOCATION_BROADCAST_JSON_LENGTH_IDX, sizeof(U16));

			json = buf+NTY_PROTO_LOCATION_BROADCAST_JSON_CONTENT_IDX;
			

			if (proto->onLocationBroadCastResult) {
				proto->onLocationBroadCastResult(fromId, json, length);
			}
			break;
		}
		case NTY_PROTO_COMMON_BROADCAST: {
			DEVID fromId = 0;
			U8 *json = NULL;
			U16 length = 0;
			
			memcpy(&fromId, buf+NTY_PROTO_COMMON_BROADCAST_DEVID_IDX, sizeof(DEVID));
			memcpy(&length, buf+NTY_PROTO_COMMON_BROADCAST_JSON_LENGTH_IDX, sizeof(U16));

			json = buf+NTY_PROTO_COMMON_BROADCAST_JSON_CONTENT_IDX;
			
			if (proto->onCommonBroadCastResult) {
				proto->onCommonBroadCastResult(fromId, json, length);
			}
			
			break;
		}
#if (NTY_PROTO_SELFTYPE == NTY_PROTO_CLIENT_WATCH)		
		case NTY_PROTO_LOCATION_PUSH: {
			U8 *json = NULL;
			U16 length = 0;
			
			memcpy(&length, buf+NTY_PROTO_LOCATION_PUSH_JSON_LENGTH_IDX, sizeof(U16));
			json = buf+NTY_PROTO_LOCATION_PUSH_JSON_CONTENT_IDX;

			if (proto->onLocationPushResult) {
				proto->onLocationPushResult(json, length);
			}

			break;
		}
		case NTY_PROTO_WEATHER_PUSH: {

			U8 *json = NULL;
			U16 length = 0;
			
			memcpy(&length, buf+NTY_PROTO_WEATHER_PUSH_JSON_LENGTH_IDX, sizeof(U16));
			json = buf+NTY_PROTO_WEATHER_PUSH_JSON_CONTENT_IDX;

			if (proto->onWeatherPushResult) {
				proto->onWeatherPushResult(json, length);
			}
			
			break;
		}
#endif		
	}
	
}

static U8 rBuffer[NTY_VOICEREQ_PACKET_LENGTH] = {0};
static U16 rLength = 0;
extern U32 ntyGenCrcValue(U8 *buf, int length);

int ntyPacketValidator(void *self, U8 *buffer, int length) {
	int bCopy = 0, bIndex = 0, ret = -1;
	U32 uCrc = 0, uClientCrc = 0;
	int bLength = length;

	LOG(" rLength :%d, length:%d\n", rLength, length);
	uCrc = ntyGenCrcValue(buffer, length-4);
	uClientCrc = ntyU8ArrayToU32(buffer+length-4);
	if (uCrc != uClientCrc) {
		do {
			bCopy = (bLength > NTY_VOICEREQ_PACKET_LENGTH ? NTY_VOICEREQ_PACKET_LENGTH : bLength);
			bCopy = (((rLength + bCopy) > NTY_VOICEREQ_PACKET_LENGTH) ? (NTY_VOICEREQ_PACKET_LENGTH - rLength) : bCopy);
			
			memcpy(rBuffer+rLength, buffer+bIndex, bCopy);
			rLength += bCopy;
			
			uCrc = ntyGenCrcValue(rBuffer, rLength-4);
			uClientCrc = ntyU8ArrayToU32(rBuffer+rLength-4);

			LOG("uCrc:%x  uClientCrc:%x", uCrc, uClientCrc);
			if (uCrc == uClientCrc)	 {
				LOG(" CMD:%x, Version:[%d]\n", rBuffer[NTY_PROTO_MSGTYPE_IDX], rBuffer[NTY_PROTO_VERSION_IDX]);
				
				ntyPacketClassifier(self, rBuffer, rLength);

				rLength = 0;
				ret = 0;
			} 
			
			bLength -= bCopy;
			bIndex += bCopy;
			rLength %= NTY_VOICEREQ_PACKET_LENGTH;
			
		} while (bLength);
	} else {
		ntyPacketClassifier(self, buffer, length);
		rLength = 0;
		ret = 0;
	}
	return ret;
}

#endif


static void* ntyRecvProc(void *arg) {
	struct sockaddr_in addr;
	int nSize = sizeof(struct sockaddr_in);
	NattyProto *proto = arg;
	NattyProtoOpera * const *protoOpera = arg;
	U8 *buf = proto->recvBuffer;

	int ret;
	void *pNetwork = ntyNetworkInstance();

	struct pollfd fds;
	fds.fd = ntyGetSocket(pNetwork);
	fds.events = POLLIN;

	ntydbg(" ntyRecvProc %d\n", fds.fd);
	while (1) {
		if (proto->u8RecvExitFlag){ 
			ntydbg(" ntyRecvProc Exist\n");
			ntyProtoRelease();

			//reconnect
			ntyStartReconnectTimer();
			break;
		}
		ret = poll(&fds, 1, 5);
		if (ret) {
			bzero(buf, RECV_BUFFER_SIZE);
			proto->recvLen = ntyRecvFrame(pNetwork, buf, RECV_BUFFER_SIZE);
			if (proto->recvLen == 0) { //disconnect
#if 1				
				proto->u8RecvExitFlag = 1;
#endif
				
				ntydbg("Prepare to Reconnect to server\n");
				if (proto->onProxyDisconnect) {
					proto->onProxyDisconnect(0);
				}

				continue;
			}

			int nServFd = ntyGetSocket(pNetwork);
			getpeername(nServFd,(struct sockaddr*)&addr, &nSize);
			ntydbg("\n%d.%d.%d.%d:%d, length:%d --> %x, id:%lld\n", *(unsigned char*)(&addr.sin_addr.s_addr), *((unsigned char*)(&addr.sin_addr.s_addr)+1),													
				*((unsigned char*)(&addr.sin_addr.s_addr)+2), *((unsigned char*)(&addr.sin_addr.s_addr)+3),													
				addr.sin_port, proto->recvLen, buf[NTY_PROTO_MSGTYPE_IDX], *(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]));	

			ntyPacketValidator(arg, buf, proto->recvLen);

		}
	}

#if 0		
	proto->u8RecvExitFlag = 0;
#endif


}






