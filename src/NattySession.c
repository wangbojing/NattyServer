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

 
#include "NattyFilter.h"
#include "NattySession.h"
#include "NattyRBTree.h"
#include "NattyUtils.h"
#include "NattyMulticast.h"

#include "NattyVector.h"
#include "NattyResult.h"

#undef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({                   \
	const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
	(type *)( (char *)__mptr - offsetof(type,member) );})

 

/*
 * send friends list to client
 */

#if 0
int ntyNotifyClient(UdpClient *client, U8 *notify) {
	int length = 0;
	
	notify[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_P2P_NOTIFY_REQ;
	
	length = NTY_PROTO_P2P_NOTIFY_CRC_IDX;
	*(U32*)(&notify[length]) = ntyGenCrcValue(notify, length);
	length += sizeof(U32);

	return ntySendBuffer(client, notify, length);

}

C_DEVID ntyClientGetDevId(void *client) {
	RBTreeNode *pNode = container_of(&client, RBTreeNode, value);
	return pNode->key;
}

/*
 * send client Ip to friend
 */

int ntyNotifyFriendConnect(void* fTree, C_DEVID id) {
	U8 notify[NTY_LOGIN_ACK_LENGTH] = {0};
	
	UdpClient *pClient = container_of(&fTree, UdpClient, friends);	
	void *pRBTree = ntyRBTreeInstance();

	UdpClient *client = ntyRBTreeInterfaceSearch(pRBTree, id);
	if (client == NULL) return -1;


	notify[NTY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	notify[NTY_PROTO_PROTOTYPE_IDX] = (U8)MSG_REQ;
	notify[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_P2P_NOTIFY_REQ;


	*(C_DEVID*)(&notify[NTY_PROTO_P2P_NOTIFY_DEVID_IDX]) = ntyClientGetDevId(pClient);
#if 0
	*(U32*)(&notify[NTY_PROTO_P2P_NOTIFY_ACKNUM_IDX]) = pClient->ackNum;
			
	*(U32*)(&notify[NTY_PROTO_P2P_NOTIFY_IPADDR_IDX]) = pClient->addr.sin_addr.s_addr;
	*(U16*)(&notify[NTY_PROTO_P2P_NOTIFY_IPPORT_IDX]) = pClient->addr.sin_port;
#endif
	return ntyNotifyClient(client, notify);
}

int ntyNotifyFriendMessage(C_DEVID fromId, C_DEVID toId) {
	U8 notify[NTY_LOGIN_ACK_LENGTH] = {0};
	
	void *pRBTree = ntyRBTreeInstance();
	UdpClient *toClient = ntyRBTreeInterfaceSearch(pRBTree, toId);
	if (toClient == NULL) return -1;

	UdpClient *fromClient = ntyRBTreeInterfaceSearch(pRBTree, fromId);
	if (fromClient == NULL) return -1;

	notify[NTY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	notify[NTY_PROTO_PROTOTYPE_IDX] = (U8)MSG_REQ;
	notify[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_P2P_NOTIFY_REQ;

	
	*(C_DEVID*)(&notify[NTY_PROTO_P2P_NOTIFY_DEVID_IDX]) = fromId;
	*(C_DEVID*)(&notify[NTY_PROTO_P2P_NOTIFY_DEST_DEVID_IDX]) = toId;
#if 0
	*(U32*)(&notify[NTY_PROTO_P2P_NOTIFY_ACKNUM_IDX]) = fromClient->ackNum;
	*(U32*)(&notify[NTY_PROTO_P2P_NOTIFY_IPADDR_IDX]) = fromClient->addr.sin_addr.s_addr;
	*(U16*)(&notify[NTY_PROTO_P2P_NOTIFY_IPPORT_IDX]) = fromClient->addr.sin_port;
#endif
	return ntyNotifyClient(toClient, notify);
}
#endif


/*
 * send friends list to client
 */
int ntySendFriendsTreeIpAddr(const void *client, U8 reqType) {
	int i = 0, length;
	U8 ack[NTY_LOGIN_ACK_LENGTH] = {0};
#if 0	
	const Client *pClient = client;
	void *pRBTree = ntyRBTreeInstance();

	C_DEVID *friends = ntyFriendsTreeGetAllNodeList(pClient->friends);
	U16 Count = ntyFriendsTreeGetNodeCount(pClient->friends);
	//ntylog("Count : %d, type:%d\n", Count, pClient->connectType);
	for (i = 0;i < Count;i ++) {
		Client *cliValue = ntyRBTreeInterfaceSearch(pRBTree, *(friends+i));
		*(C_DEVID*)(&ack[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_DEVID_IDX(i)]) = *(friends+i);
	}
	free(friends);
#else

	const Client *pClient = client;

	void *map = ntyMapInstance();
	ClientSocket *toClient = ntyMapSearch(map, pClient->devId);

	//NVector *vector = pClient->friends;
	int Count = 0;
	C_DEVID *friends = ntyVectorGetNodeList(pClient->friends, &Count);
	if (friends != NULL) {
		for (i = 0;i < Count;i ++) {
			*(C_DEVID*)(&ack[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_DEVID_IDX(i)]) = *(friends+i);
		}
		free(friends);
	}
	
	
#endif
	ack[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;
	ack[NTY_PROTO_PROTOTYPE_IDX] = (U8)MSG_UPDATE;
	if (reqType) {
		ack[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_LOGIN_ACK;
	} else {
		ack[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_HEARTBEAT_ACK;
	}
	//*(U32*)(&ack[NTY_PROTO_LOGIN_ACK_ACKNUM_IDX]) = pClient->ackNum;
	*(U16*)(&ack[NTY_PROTO_LOGIN_ACK_FRIENDS_COUNT_IDX]) = Count;
	*(U32*)(&ack[NTY_PROTO_LOGIN_ACK_CRC_IDX]) = ntyGenCrcValue(ack, NTY_PROTO_LOGIN_ACK_CRC_IDX);
	length += NTY_PROTO_LOGIN_ACK_CRC_IDX+sizeof(U32);

	return ntySendBuffer(toClient, ack, length);
}

/*
 * transparent transport data
 * VERSION 			1			BYTE
 * MESSAGE TYPE		1			BYTE
 * TYPE			1			BYTE
 * DEVID			8			BYTE
 * ACKNUM			4			BYTE
 * FRIENDID			8			BYTE
 * BYTECOUNT		2			BYTE
 * CONTENT			*(BYTECOUNT)	BYTE
 * CRC			4			BYTE
 */

#if 0 
int ntyRouteUserData(C_DEVID friendId, U8 *buffer) {
	int length = 0;
	U8 notify[RECV_BUFFER_SIZE] = {0};
	
	U16 cliCount = *(U16*)(&buffer[NTY_PROTO_DATAPACKET_RECE_COUNT_IDX]);
	U16 recByteCount = *(U16*)(&buffer[NTY_PROTO_DATAPACKET_CONTENT_COUNT_IDX(cliCount)]);
	//get friend ip addr and port
	void *pRBTree = ntyRBTreeInstance();
	UdpClient *pClient = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, friendId);
	if (pClient == NULL) return -1;
	
	notify[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	notify[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_DATAPACKET_REQ;

	*(C_DEVID*)(&notify[NTY_PROTO_DATAPACKET_NOTIFY_DEVID_IDX]) = *(C_DEVID*)(&buffer[NTY_PROTO_DATAPACKET_DEVID_IDX]); //friendId;
	*(U32*)(&notify[NTY_PROTO_DATAPACKET_ACKNUM_IDX]) = *(U32*)(buffer+NTY_PROTO_DATAPACKET_ACKNUM_IDX);
	*(C_DEVID*)(&notify[NTY_PROTO_DATAPACKET_NOTIFY_DEST_DEVID_IDX]) = friendId;//*(C_DEVID*)(buffer+NTY_PROTO_DATAPACKET_DEVID_IDX);
	memcpy(&notify[NTY_PROTO_DATAPACKET_NOTIFY_CONTENT_COUNT_IDX], &cliCount, 2);
	memcpy(&notify[NTY_PROTO_DATAPACKET_NOTIFY_CONTENT_IDX], &buffer[NTY_PROTO_DATAPACKET_CONTENT_IDX(cliCount)], recByteCount);

	ntylog(" recByteCount:%d  notify:%s\n", recByteCount, notify+NTY_PROTO_DATAPACKET_NOTIFY_CONTENT_IDX);
	length = NTY_PROTO_DATAPACKET_NOTIFY_CONTENT_IDX + recByteCount;
	*(U32*)(&notify[length]) = ntyGenCrcValue(notify, length);
	length += sizeof(U32);
	
	return ntySendBuffer(pClient, notify, length);
}
#endif


int ntySendDeviceTimeCheckAck(const Client *pClient, U32 ackNum) {
	int length = 0;
	U8 ack[RECV_BUFFER_SIZE] = {0};

	
	ack[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;
	ack[NTY_PROTO_PROTOTYPE_IDX] = (U8)MSG_ACK;
	ack[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_TIME_CHECK_ACK;
#if 0
	ack[NTY_PROTO_DEVID_IDX] = pClient->devId;
#else
	memcpy(ack+NTY_PROTO_DEVID_IDX, &pClient->devId, sizeof(C_DEVID));
#endif
	*(U32*)(&ack[NTY_PROTO_ACKNUM_IDX]) = ackNum;

	ntyTimeCheckStamp(ack);
	
	//*(U32*)(&ack[NTY_PROTO_TIMECHECK_CRC_IDX]) = ntyGenCrcValue(ack, NTY_PROTO_LOGIN_REQ_CRC_IDX);
	length = NTY_PROTO_TIMECHECK_CRC_IDX+sizeof(U32);

	void *map = ntyMapInstance();
	ClientSocket *nSocket = ntyMapSearch(map, pClient->devId);
	
	return ntySendBuffer(nSocket, ack, length);
	
}

int ntySendDeviceRouterInfo(const Client *pClient, U8 *buffer, int length) {
	U8 buf[RECV_BUFFER_SIZE] = {0};
	
	buf[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;
	buf[NTY_PROTO_PROTOTYPE_IDX] = (U8) MSG_UPDATE;	
	buf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_DATAPACKET_REQ;
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEVID_IDX]) = pClient->devId;
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEST_DEVID_IDX]) = pClient->devId;

	U8 *tempBuf = &buf[NTY_PROTO_DATAPACKET_CONTENT_IDX];
	memcpy(tempBuf, buffer, length);
	
	*(U16*)(&buf[NTY_PROTO_DATAPACKET_CONTENT_COUNT_IDX]) = (U16)length;
	length += NTY_PROTO_DATAPACKET_CONTENT_IDX;
	length += sizeof(U32);

	void *map = ntyMapInstance();
	ClientSocket *nSocket = ntyMapSearch(map, pClient->devId);

	return ntySendBuffer(nSocket, buf, length);
}

int ntySendAppRouterInfo(const Client *pClient, C_DEVID fromId, U8 *buffer, int length) {
	U8 buf[RECV_BUFFER_SIZE] = {0};
	int i = 0;
	
	buf[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;
	buf[NTY_PROTO_PROTOTYPE_IDX] = (U8) MSG_REQ;	
	buf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_DATAPACKET_REQ;
#if 0
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEVID_IDX]) = fromId;
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEST_DEVID_IDX]) = pClient->devId;
#else
	memcpy(&buf[NTY_PROTO_DATAPACKET_DEVID_IDX], &fromId, sizeof(C_DEVID));
	memcpy(&buf[NTY_PROTO_DATAPACKET_DEST_DEVID_IDX], &pClient->devId, sizeof(C_DEVID));
#endif
	U8 *tempBuf = &buf[NTY_PROTO_DATAPACKET_CONTENT_IDX];
	memcpy(tempBuf, buffer, length);

	ntylog(" ntySendAppRouterInfo --> fromId:%lld, toId:%lld, cmd:%s, length:%d, type:%x\n", *(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEVID_IDX]),
		*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEST_DEVID_IDX]), tempBuf, length, buf[NTY_PROTO_MSGTYPE_IDX]);
	
	*(U16*)(&buf[NTY_PROTO_DATAPACKET_CONTENT_COUNT_IDX]) = (U16)length;
	length += NTY_PROTO_DATAPACKET_CONTENT_IDX;
	length += sizeof(U32);

	void *map = ntyMapInstance();
	ClientSocket *nSocket = ntyMapSearch(map, pClient->devId);

	return ntySendBuffer(nSocket, buf, length);
}


static int ntyBoardcastItem(void* client, C_DEVID toId, U8 *data, int length) {
	Client *selfNode = client;

	ntylog(" ntyBoardcastItem --> Start\n");
	if (selfNode->devId == toId) return -1;
	
	void *pRBTree = ntyRBTreeInstance();
	Client *toClient = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, toId);
	if (toClient == NULL) { //local have no client node and need to multicast 
		ntylog(" ntyBoardcastItem --> toId:%lld is not Exist\n", toId);
		
#if ENABLE_MULTICAST_SYNC //multicast 
		//data[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_MULTICAST_REQ;
		int len = length;
		U8 buffer[RECV_BUFFER_SIZE] = {0};

		buffer[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;
		buffer[NTY_PROTO_PROTOTYPE_IDX] = (U8) MSG_REQ;	
		buffer[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_DATAPACKET_REQ;
		//buffer[NTY_PROTO_DEVID_IDX] = 
		memcpy(buffer+NTY_PROTO_DEVID_IDX, &selfNode->devId, sizeof(C_DEVID));
		memcpy(buffer+NTY_PROTO_DEST_DEVID_IDX, &toId, sizeof(C_DEVID));
		*(U16*)(&buffer[NTY_PROTO_DATAPACKET_CONTENT_COUNT_IDX]) = (U16)len;
		memcpy(buffer+NTY_PROTO_DATAPACKET_CONTENT_IDX, data, length);

		len += NTY_PROTO_DATAPACKET_CONTENT_IDX;
		len += sizeof(U32);
		
		ntyMulticastSend(buffer, len);
#endif
		return -1;
	}

	ntylog(" ntyBoardcastItem --> fromId:%lld, toId:%lld, data:%s, length:%d\n", selfNode->devId, toId, data, length);
	return ntySendAppRouterInfo(toClient, selfNode->devId, data, length);
}


int ntyBoardcastAllFriends(const Client *self, U8 *buffer, int length) {
	void *pRBTree = ntyRBTreeInstance();

	ntylog("ntyBoardcastAllFriends --> self devid:%lld\n", self->devId);
	Client *selfNode = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, self->devId);
	if (selfNode != NULL) {
		RBTree *friends = (RBTree*)selfNode->friends;
		if (friends != NULL) {
			ntylog("ntyBoardcastAllFriends --> self devid:%lld, selfNode.id:%lld\n", self->devId, selfNode->devId);
			ntyFriendsTreeBroadcast(friends, ntyBoardcastItem, selfNode, buffer, length);
		}
	}
	return 0;
}

int ntyBoardcastAllFriendsById(C_DEVID fromId, U8 *buffer, int length) {
	void *pRBTree = ntyRBTreeInstance();

	ntylog("ntyBoardcastAllFriendsById --> self devid:%lld\n", fromId);
	Client *selfNode = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, fromId);
	if (selfNode != NULL) {
		RBTree *friends = (RBTree*)selfNode->friends;
		if (friends != NULL) {
			ntylog("ntyBoardcastAllFriendsById --> self devid:%lld, selfNode.id:%lld\n", fromId, selfNode->devId);
			//if (fromId != selfNode->devId)
			ntyFriendsTreeBroadcast(friends, ntyBoardcastItem, selfNode, buffer, length);
		}
	}
}

int ntyBoardcastAllFriendsNotifyDisconnect(C_DEVID selfId) {
	U8 u8ResultBuffer[256] = {0};
	
	ntylog(" ntyBoardcastAllFriendsNotifyDisconnect --> Notify All Friends\n");
	sprintf(u8ResultBuffer, "Set Disconnect 1");
	ntyBoardcastAllFriendsById(selfId, u8ResultBuffer, strlen(u8ResultBuffer));

	return 0;
}

int ntyBoardcastAllFriendsNotifyConnect(C_DEVID selfId) {
	U8 u8ResultBuffer[256] = {0};
	
	ntylog(" ntyBoardcastAllFriendsNotifyConnect --> Notify All Friends\n");
	sprintf(u8ResultBuffer, "Set Connect 1");
	ntyBoardcastAllFriendsById(selfId, u8ResultBuffer, strlen(u8ResultBuffer));

	return 0;
}


#if 1
void ntyProtoHttpProxyTransform(C_DEVID fromId, C_DEVID toId, U8 *buffer, int length) {
	int n = 0;
	U8 buf[RECV_BUFFER_SIZE] = {0};

	buf[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;
	buf[NTY_PROTO_PROTOTYPE_IDX] = (U8) MSG_REQ;	
	buf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_DATAPACKET_REQ;
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEVID_IDX]) = fromId;
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEST_DEVID_IDX]) = toId;
	
	*(U16*)(&buf[NTY_PROTO_DATAPACKET_CONTENT_COUNT_IDX]) = (U16)length;
	memcpy(buf+NTY_PROTO_DATAPACKET_CONTENT_IDX, buffer, length);
	ntylog(" ntyProtoHttpProxyTransform ---> %s\n", buf+NTY_PROTO_DATAPACKET_CONTENT_IDX);
	
	length += NTY_PROTO_DATAPACKET_CONTENT_IDX;

	*(U32*)(&buf[length]) = ntyGenCrcValue(buf, length);
	length += sizeof(U32);

	void *map = ntyMapInstance();
	ClientSocket *nSocket = ntyMapSearch(map, toId);
	ntySendBuffer(nSocket, buf, length);
	
}

void ntyProtoHttpRetProxyTransform(C_DEVID toId, U8 *buffer, int length) {
	int n = 0;
	U8 buf[RECV_BUFFER_SIZE] = {0};
#if 0
	void *pRBTree = ntyRBTreeInstance();
	if (pRBTree == NULL) return ;
	UdpClient *destClient = (UdpClient*)ntyRBTreeInterfaceSearch(pRBTree, toId);
	if (destClient == NULL) {
		ntylog(" destClient is not exist, toId:%lld\n", toId);
		return ;
	}
#else
	void *map = ntyMapInstance();
	ClientSocket *nSocket = ntyMapSearch(map, toId);

#endif
	buf[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;
	buf[NTY_PROTO_PROTOTYPE_IDX] = (U8) MSG_REQ;	
	buf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_ROUTERDATA_REQ;
	//*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEVID_IDX]) = fromId;
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEST_DEVID_IDX]) = toId;
	
	*(U16*)(&buf[NTY_PROTO_DATAPACKET_CONTENT_COUNT_IDX]) = (U16)length;
	memcpy(buf+NTY_PROTO_DATAPACKET_CONTENT_IDX, buffer, length);

	ntylog(" ntyProtoHttpRetProxyTransform ---> %s, length:%d\n", buf+NTY_PROTO_DATAPACKET_CONTENT_IDX, length);
	length += NTY_PROTO_DATAPACKET_CONTENT_IDX;

	*(U32*)(&buf[length]) = ntyGenCrcValue(buf, length);
	length += sizeof(U32);

	ntySendBuffer(nSocket, buf, length);
	
}


#endif


/*
 * result : 0 --> OK
 *          1 --> AppId No Exist 
 *          2 --> DevId No Exist
 *          3 --> Have Bound
 *		 4 --> DB Exception
 */
void ntyProtoBindAck(C_DEVID aid, C_DEVID did, int result) {
	int length = 0;
	U8 buf[NTY_HEARTBEAT_ACK_LENGTH] = {0};
#if 0	
	void *pRBTree = ntyRBTreeInstance();
	Client *client = (Client*)ntyRBTreeInterfaceSearch(pRBTree, aid);
#else
	void *map = ntyMapInstance();
	ClientSocket *nSocket = ntyMapSearch(map, aid);
#endif
	if (nSocket == NULL) {
		ntylog(" destClient is not exist proxy:%lld\n", aid);
		return ;
	}

	buf[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;
	buf[NTY_PROTO_PROTOTYPE_IDX] = (U8) MSG_ACK;	
	buf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_BIND_ACK;
	*(C_DEVID*)(&buf[NTY_PROTO_BIND_ACK_DEVICEID_IDX]) = did;
	*(int*)(&buf[NTY_PROTO_BIND_ACK_RESULT_IDX]) = result;

	length = NTY_PROTO_BIND_ACK_CRC_IDX;
	//*(U32*)(&buf[length]) = ntyGenCrcValue(buf, length);
	length += sizeof(U32);

	ntySendBuffer(nSocket, buf, length);
}



/*
 * result : 0 --> OK
 *          1 --> AppId No Exist 
 *          2 --> DevId No Exist
 *          3 --> Have Bound
 *		 4 --> DB Exception
 */
 
void ntyProtoUnBindAck(C_DEVID aid, C_DEVID did, int result) {
	int length = 0;
	U8 buf[NTY_HEARTBEAT_ACK_LENGTH] = {0};
	
#if 0	
	void *pRBTree = ntyRBTreeInstance();
	Client *client = (Client*)ntyRBTreeInterfaceSearch(pRBTree, aid);
#else
	void *map = ntyMapInstance();
	ClientSocket *nSocket = ntyMapSearch(map, aid);
#endif
	if (nSocket == NULL) {
		ntylog(" destClient is not exist proxy:%lld\n", aid);
		return ;
	}

	buf[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;
	buf[NTY_PROTO_PROTOTYPE_IDX] = (U8) MSG_ACK;	
	buf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_UNBIND_ACK;
	*(C_DEVID*)(&buf[NTY_PROTO_UNBIND_ACK_DEVICEID_IDX]) = did;
	*(int*)(&buf[NTY_PROTO_UNBIND_ACK_RESULT_IDX]) = result;

	length = NTY_PROTO_UNBIND_ACK_CRC_IDX;
	//*(U32*)(&buf[length]) = ntyGenCrcValue(buf, length);
	length += sizeof(U32);

	ntySendBuffer(nSocket, buf, length);
}


void ntyProtoICCIDAck(C_DEVID did, U8 *phnum, U16 length) {
	U8 buf[NTY_HEARTBEAT_ACK_LENGTH*2] = {0};
#if 0	
	void *pRBTree = ntyRBTreeInstance();
	Client *client = (Client*)ntyRBTreeInterfaceSearch(pRBTree, did);
#else
	void *map = ntyMapInstance();
	ClientSocket *nSocket = ntyMapSearch(map, did);
#endif
	if (nSocket == NULL) {
		ntylog(" destClient is not exist proxy:%lld\n", did);
		return ;
	}

	buf[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;
	buf[NTY_PROTO_PROTOTYPE_IDX] = (U8) MSG_ACK;	
	buf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_ICCID_ACK;

	memcpy(buf+NTY_PROTO_ICCIDACK_SLFID_IDX, &did, sizeof(C_DEVID));
	memcpy(buf+NTY_PROTO_ICCIDACK_CONTENT_COUNT_IDX, &length, sizeof(U16));
	memcpy(buf+NTY_PROTO_ICCIDACK_CONTENT_IDX, phnum, length);

	length += NTY_PROTO_ICCIDACK_CONTENT_IDX;
	//*(U32*)(&buf[length]) = ntyGenCrcValue(buf, length);
	length += sizeof(U32);

	ntySendBuffer(nSocket, buf, length);
}

void ntySelfLogoutPacket(C_DEVID id, U8 *buffer) {
	buffer[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_LOGOUT_REQ;
	memcpy(buffer+NTY_PROTO_DEVID_IDX, &id, sizeof(C_DEVID));

	//*rLen = NTY_PROTO_MIN_LENGTH;
	U32 u32Crc = ntyGenCrcValue(buffer, NTY_PROTO_MIN_LENGTH-4);
	memcpy(buffer+NTY_PROTO_MIN_LENGTH-4, &u32Crc, sizeof(U32));
	
}


int ntyIterFriendsMessage(void *self, void *arg) {
	C_DEVID *pId = self;
	InterMsg *msg = arg;

	Client *client = msg->self;
	void *map = ntyMapInstance();
	ClientSocket *nSocket = ntyMapSearch(map, client->devId);
	if (nSocket == NULL)  return  NTY_RESULT_NOEXIST;

	return ntyProxyBuffer(nSocket, msg->buffer, msg->length);
}





