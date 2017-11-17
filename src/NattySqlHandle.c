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

#include "NattyBPlusTree.h"

#include "NattySqlHandle.h" 
#include "NattyAbstractClass.h"
#include "NattyResult.h"
#include "NattyDaveMQ.h"
#include "NattyMessage.h"
#include "NattyServAction.h"
#include "NattyUdpServer.h"
#include "NattyProtocol.h"
#include "NattyPush.h"

int ntyReadOfflineVoiceHandle(void *arg);


int ntyVoiceAckHandle(void *arg) {
	if (arg == NULL) return NTY_RESULT_ERROR;
	VALUE_TYPE *tag = arg;

	C_DEVID fromId = tag->fromId;
	int msgId = tag->arg;
	ntylog("ntyVoiceAckHandle begin to delete msg,fromId:%lld,msgId:%d\n",fromId,msgId);

	int ret = ntyExecuteVoiceOfflineMsgDeleteHandle(msgId, fromId);
	if (ret == NTY_RESULT_FAILED) {
		ntylog("ntyVoiceAckHandle delete msgId failed\n");
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DB_NOEXIST);
	} 
#if 0
	//read offline voice msg
	ret = ntyReadOfflineVoiceMsgAction(fromId);
	if (ret == NTY_RESULT_NOEXIST) {
		//BindConfirmReq
		//
	}
	
	free(tag);
	return ret;
#else
	//the client one by one to read offline messages from server,include commond,voice,bind confirm messages.
	tag->Type = MSG_TYPE_VOICE_OFFLINE_READ_HANDLE;
	tag->cb = ntyReadOfflineVoiceHandle;	
	//tag->Type = MSG_TYPE_OFFLINE_MSG_REQ_HANDLE;
	//tag->cb = ntyOfflineMsgReqHandle;
	ntyDaveMqPushMessage(tag);
#endif
	return 	NTY_RESULT_SUCCESS;	

}

int ntyCommonAckHandle(void *arg) {
	if (arg == NULL) return NTY_RESULT_ERROR;
	VALUE_TYPE *tag = arg;
	ntylog(" ntyCommonAckHandle \n");

	C_DEVID fromId = tag->fromId;
	int msgId = tag->arg;

	int ret = ntyExecuteCommonOfflineMsgDeleteHandle(msgId, fromId);
	if (ret == NTY_RESULT_FAILED) {
		ntylog("ntyCommonAckHandle DB Error \n");
	}
	
	//the client one by one to read offline messages from server,include commond,voice,bind confirm messages.
	/*tag->Type = MSG_TYPE_OFFLINE_MSG_REQ_HANDLE;
	if (tag->Type == MSG_TYPE_DEVICE_COMMON_ACK_HANDLE) { //watch,not used.
		tag->cb = ntyDeviceOfflineMsgReqHandle;
	} else {
		tag->cb = ntyOfflineMsgReqHandle;
	}
	*/
	tag->Type = MSG_TYPE_OFFLINE_MSG_REQ_HANDLE;
	tag->cb = ntyOfflineMsgCommonReqHandle;
	ntyDaveMqPushMessage(tag);

	return ret;
}


int ntyVoiceDataReqHandle(void *arg) {
	if (arg == NULL) return NTY_RESULT_ERROR;
	VALUE_TYPE *tag = arg;
	C_DEVID senderId = tag->fromId;
	C_DEVID gId = tag->toId;
	U8 *filename = tag->Tag;
	
	ntylog( "ntyVoiceDataReqHandle fromId:%lld,gId:%lld\n", senderId, gId );
	U32 msgId = 0;

	//insert voice msg to db
	int ret = ntyQueryVoiceMsgInsertHandle(senderId, gId, filename, &msgId);
	if (ret == NTY_RESULT_SUCCESS) {
		ntyJsonCommonResult(senderId, NATTY_RESULT_CODE_SUCCESS);
	} else {
		ntyJsonCommonResult(senderId, NATTY_RESULT_CODE_ERR_DB_OPERATION);
		return ret;
	}
	//broadcast to all id
	//json is null, length is 0
	ntySendVoiceBroadCastResult(senderId, gId, NULL, 0, msgId);

	ntyFree( tag->Tag );
	ntyFree( tag );

	return NTY_RESULT_SUCCESS;
}


int ntyVoiceReqHandle(void *arg) {
	if (arg == NULL) return NTY_RESULT_ERROR;
	VALUE_TYPE *tag = arg;

	C_DEVID fromId = tag->fromId;
	int msgId = tag->arg;

	int ret = ntyVoiceReqAction(fromId, msgId);
	if ( NTY_RESULT_FAILED == ret ) {
		//delete fromId And MsgId
		ntylog("ntyVoiceReqHandle ntyVoiceReqAction ret=-1 failed\n");
		ret = ntyExecuteVoiceOfflineMsgDeleteHandle(msgId, fromId);
		if (ret == NTY_RESULT_FAILED) {
			ntylog("ntyVoiceReqHandle --> ntyExecuteVoiceOfflineMsgDeleteHandle failed\n");
		} 
		//send next voice msg
		ret = ntyReadOfflineVoiceMsgAction(fromId);
		if (ret == NTY_RESULT_NOEXIST) {
			//read
			ntylog("ntyVoiceReqHandle --> ntyReadOfflineVoiceMsgAction failed\n");
		}
	}

	return 0;
}

//添加联系人消息发送到手表
void ntyBindAgreeWatchStep(C_DEVID adminId, C_DEVID DeviceId, char *phonenum, int contactsTempId, char *pname, char *pimage) {
	DeviceAddContactsAck *pDeviceAddContactsAck = malloc(sizeof(DeviceAddContactsAck));
	if (pDeviceAddContactsAck == NULL) {
		ntylog("ntyBindAgreeWatchStep --> malloc DeviceAddContactsAck failed\n");
		return;
	}
	memset(pDeviceAddContactsAck, 0, sizeof(DeviceAddContactsAck));

	char bufIMEI[64] = {0};
	sprintf(bufIMEI, "%llx", DeviceId);
	char contactsId[16] = {0};
	sprintf(contactsId, "%d", contactsTempId);
	char add[16] = {0};
	char category[16] = {0};
	strcat(add, "Add");
	strcat(category, "Contacts");
	pDeviceAddContactsAck->id = contactsId;
	pDeviceAddContactsAck->IMEI = bufIMEI;
	pDeviceAddContactsAck->category = category;
	pDeviceAddContactsAck->action = add;
	pDeviceAddContactsAck->contacts.id = contactsId;
	pDeviceAddContactsAck->contacts.name = pname;
	pDeviceAddContactsAck->contacts.image = pimage;
	pDeviceAddContactsAck->contacts.telphone = phonenum;

	C_DEVID fromId = adminId;
	C_DEVID toId = DeviceId;
	char *jsonagree = ntyJsonWriteDeviceAddContacts(pDeviceAddContactsAck);
	if ( jsonagree == NULL ){
		ntylog( "ntyBindAgreeWatchStep jsonagree is NULL\n" );
		free( pDeviceAddContactsAck );
		return;
	}
	ntylog(" ntyBindAgreeWatchStep jsonagree: %s\n", jsonagree);
	int ret = ntySendRecodeJsonPacket(fromId, toId, jsonagree, (int)strlen(jsonagree));
	if (ret < 0) {
		ntylog("ntyBindAgreeWatchStep --> SendCommonReq Exception\n");
		//ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DEVICE_NOTONLINE);
	}
	ntyJsonFree( jsonagree );
	free( pDeviceAddContactsAck );
}

/*
 * 
 * adminId [in] 管理员Id
 * ProposerId [out] 申请者Id
 * DeviceId [in] 设备IMEI
 * msgId [in] TB_BIND_CONFIRM Id
 * phonenum [out] 申请者电话号码
 * pid [out] TB_PHONEBOOK Id
 * pname [out] TB_GROUP G_NAME
 * pimage [out] TB_GROUP G_DEVIMAGE
 */
int ntyBindConfirmStep(C_DEVID adminId, C_DEVID *ProposerId, C_DEVID DeviceId, U32 msgId, U8 *phonenum, int *pid, char *pname, char *pimage) {
	C_DEVID AppId = 0x0;
	int result = 0;
#if 0
	int ret = ntyExecuteDevAppGroupBindAndAgreeInsertHandle(msgId, ProposerId, phonenum, &contactsTempId, pname, pimage);
#else
	int ret = ntyExecuteDevAppGroupBindInsertHandle(msgId, ProposerId, phonenum, pid, pname, pimage);
#endif
	memcpy(&AppId, ProposerId, sizeof(C_DEVID));
	if (ret == -1) {
		ntylog(" ntyBindConfirm --> DB Exception\n");
	} else if (ret == 0) { //Bind Success Update RBTree	
		void *heap = ntyBHeapInstance();
		NRecord *record = ntyBHeapSelect(heap, AppId);
		if (record != NULL) {
			Client *aclient = record->value;
			if ( aclient == NULL ){
				ntylog(" ntyBindConfirm --> aclient==NULL\n");
				return NTY_RESULT_FAILED;
			}
			ntyVectorInsert(aclient->friends, &DeviceId, sizeof(C_DEVID));
		}

		record = ntyBHeapSelect(heap, DeviceId);
		if (record != NULL) {
			Client *dclient = record->value;
			if ( dclient == NULL ){
				ntylog(" ntyBindConfirm --> aclient==NULL\n");
				return NTY_RESULT_FAILED;
			}
			ntyVectorInsert(dclient->friends, &AppId, sizeof(C_DEVID));
		}
	}else{}
	ntylog(" ntyBindConfirm --> ntyJsonCommonResult\n");
	
	return ret;
}


int ntyBindConfirmReqHandle(void *arg) {
	ntylog("------------------- ntyBindConfirmReqHandle  begin--------------------------\n");
	if (arg == NULL) return NTY_RESULT_ERROR;
	VALUE_TYPE *tag = arg;
	U8 json[PACKET_BUFFER_SIZE] = {0};
	int ret = 0;
	C_DEVID adminId = tag->fromId;
	C_DEVID proposerId = tag->toId; //read from TB_BIND_CONFIRM proposerId by msgid
	C_DEVID gId = tag->gId;
	U32 msgId = tag->arg;

	char msgIds[64] = {0};
	char answer[64] = {0};
	char imei[64] = {0};
	char adminIds[64] = {0};
	char bindConfirmReq[64] = {0};
	memcpy(bindConfirmReq, NATTY_USER_PROTOCOL_BINDCONFIRMREQ, strlen(NATTY_USER_PROTOCOL_BINDCONFIRMREQ));
	sprintf(imei, "%llx", gId);
	sprintf(msgIds, "%d", msgId);
	sprintf(adminIds, "%lld", tag->fromId);
	
	U8 flag = tag->u8LocationType;
	ntylog(" ntyBindConfirmReqHandle flag:%d, proposerId:%lld\n", flag, proposerId);
	
	if ( flag == 1 ) { // AGREE
	
		int contactsTempId = 0;
		char phonenumArr[64] = {0};		
		//char pname[128] = {0};
		//char pimage[512] = {0};
		U8 *phonenum = (U8 *)malloc( 64 );
		memset( phonenum, 0, 64 );
		U8 *pname = (U8 *)malloc( 64 );
		memset( pname, 0, 64 );
		U8 *pimage = (U8 *)malloc( 512 );
		memset( pimage, 0, 512 );
		ret = ntyBindConfirmStep(adminId, &proposerId, gId, msgId, phonenum, &contactsTempId, pname, pimage); 
	#if 0
		//select phone 
		ntyQueryPhoneBookSelectHandle(gId, proposerId, phonenum);
		ntydbg("ntyBindConfirmReqHandle->flag:%d, phnum:%s\n", flag, phonenum);
	#endif
		ntylog( "ntyBindConfirmReqHandle after phonenum:%s\n",phonenum );
		memcpy( phonenumArr, phonenum, strlen(phonenum) );
		ntyBindAgreeWatchStep( adminId, gId, phonenumArr, contactsTempId, pname, pimage );

		BindBroadCast *pBindBroadCast = malloc(sizeof(BindBroadCast));
		memcpy(answer, NATTY_USER_PROTOCOL_AGREE, strlen(NATTY_USER_PROTOCOL_AGREE));	
		pBindBroadCast->result.IMEI = imei;
		pBindBroadCast->result.category = bindConfirmReq;
		pBindBroadCast->result.proposer = phonenumArr;
		pBindBroadCast->result.answer = answer;
		
		char *jsonresult = ntyJsonWriteBindBroadCast(pBindBroadCast);
		ntylog("ntyBindConfirmReqHandle agree json: ->%s\n",  jsonresult);
		int tempMsgId = 0;
		ret = ntyExecuteCommonMsgInsertHandle(adminId, gId, jsonresult, &tempMsgId);
		ntyJsonBroadCastRecvResult(adminId, gId, (U8*)jsonresult, tempMsgId); //broadcast to all which binded the device.
		//ret = ntySendPushNotifyIos( proposerId, gId, NTY_PUSH_BINDCONFIRM_OK_MSG_CONTEXT, 0 );	//push to the appid which binded the device.
		
		if( jsonresult != NULL ){
			ntyJsonFree( jsonresult );
		}
		ntyFree( pBindBroadCast );
		ntyFree( phonenum );
		ntyFree( pname );
		ntyFree( pimage );

	#if 0
		//发送管理员同意消息到手表
		ntyBindAgreeAction(imei, adminId, proposerId, gId, phonenum, msgId);
	#endif
	
	} else if ( flag == 0 ) {  // REJECT
		//char phonenum[64] = {0};
		U8 *phonenum = (U8 *)malloc( 64 );
		memset( phonenum, 0, 64 );
	#if 0
		int ret = ntyBindConfirm(adminId, &proposerId, gId, msgId, phonenum); 
	#else
		int ret = ntyExecuteBindConfirmDeleteHandle(msgId, phonenum, &proposerId);
	#endif
		BindBroadCast *pBindBroadCast = malloc(sizeof(BindBroadCast));
		memcpy(answer, NATTY_USER_PROTOCOL_REJECT, strlen(NATTY_USER_PROTOCOL_REJECT));
		pBindBroadCast->result.IMEI = imei;
		pBindBroadCast->result.category = bindConfirmReq;
		pBindBroadCast->result.proposer = phonenum;
		pBindBroadCast->result.answer = answer;
		
		char *jsonresult = ntyJsonWriteBindBroadCast( pBindBroadCast );
		ntylog("ntyBindConfirmReqHandle ntyJsonWriteBindBroadCast->%s,proposerId:%lld\n", jsonresult, proposerId );
		int tempMsgId = 0;
		ret = ntyExecuteCommonMsgToProposerInsertHandle(proposerId, gId, jsonresult, &tempMsgId);	
		ntyProtoBindAck( proposerId, gId, 6 ); //reject
		if( jsonresult != NULL ){
			ntyJsonFree( jsonresult );
		}
		ntyFree( pBindBroadCast );
		ntyFree( phonenum );
	}else{}
	
	ntyFree( tag );
	ntylog("------------------- ntyBindConfirmReqHandle  end--------------------------\n");
	return 0;
}

int ntyReadOfflineBindMsgHandle(void *arg) {
	VALUE_TYPE *tag = (VALUE_TYPE*)arg;
	if (tag == NULL) return NTY_RESULT_ERROR;
	
	C_DEVID fromId = tag->fromId;
	int ret = ntyReadOfflineBindMsgToAdminAction(fromId);
	if (ret == NTY_RESULT_NOEXIST) { 
		ntylog("ntyReadOfflineBindMsgHandle --> Client Id: %lld have no offline msg\n", fromId);
	}
	
	ntyFree(tag);
	return ret;
}

int ntyReadOfflineVoiceHandle(void *arg) {
	VALUE_TYPE *tag = (VALUE_TYPE*)arg;
	if (tag == NULL) return NTY_RESULT_ERROR;
	
	C_DEVID fromId = tag->fromId;
	int ret = ntyReadOfflineVoiceMsgAction(fromId);
	ntylog("ntyReadOfflineVoiceHandle fromId:%lld,ret:%d\n",fromId,ret );
	
	//if (ret == NTY_RESULT_NOEXIST) {		
		void *heap = ntyBHeapInstance();
		NRecord *record = ntyBHeapSelect(heap, fromId);
		if (record == NULL) goto exit;	
		Client *pClient = (Client *)record->value;
		if (pClient->deviceType != NTY_PROTO_CLIENT_WATCH) {
			tag->Type = MSG_TYPE_BIND_OFFLINE_PUSH_HANDLE;
			tag->cb = ntyReadOfflineBindMsgHandle;		
			ntyDaveMqPushMessage(tag);
			return NTY_RESULT_SUCCESS;
		}
	//}
	
exit:
	ntyFree( tag );
	return NTY_RESULT_SUCCESS;
}

int ntyDeviceOfflineMsgReqHandle(void *arg) {
	VALUE_TYPE *tag = (VALUE_TYPE*)arg;
	if (tag == NULL) return NTY_RESULT_ERROR;	
	C_DEVID fromId = tag->fromId;

	int ret = ntyReadDeviceOfflineCommonMsgAction(fromId);
	if (ret == NTY_RESULT_NOEXIST) {
		ntylog("ntyDeviceOfflineMsgReqHandle --> no offline message\n");	
		tag->Type = MSG_TYPE_VOICE_OFFLINE_READ_HANDLE;
		tag->cb = ntyReadOfflineVoiceHandle;
		ntyDaveMqPushMessage(tag);
		return NTY_RESULT_SUCCESS;
	}
	ntyFree(tag); 
	return NTY_RESULT_SUCCESS;
}

int ntyOfflineMsgCommonReqHandle(void *arg){
	VALUE_TYPE *tag = (VALUE_TYPE*)arg;
	if (tag == NULL) return NTY_RESULT_ERROR;
	
	C_DEVID fromId = tag->fromId;
	int ret = ntyReadOfflineCommonMsgAction(fromId);
	ntylog( "ntyOfflineMsgReqHandle fromId:%lld,ret:%d\n",fromId,ret );
		
	ntyFree( tag );	
	return NTY_RESULT_SUCCESS;
}

int ntyOfflineMsgReqHandle(void *arg) {
	VALUE_TYPE *tag = (VALUE_TYPE*)arg;
	if (tag == NULL) return NTY_RESULT_ERROR;
	
	C_DEVID fromId = tag->fromId;
	int ret = ntyReadOfflineCommonMsgAction(fromId);
	ntylog( "ntyOfflineMsgReqHandle fromId:%lld,ret:%d\n",fromId,ret );
	
	//if (ret == NTY_RESULT_NOEXIST) {	
		tag->Type = MSG_TYPE_VOICE_OFFLINE_READ_HANDLE;
		tag->cb = ntyReadOfflineVoiceHandle;
		ntyDaveMqPushMessage(tag);
		return NTY_RESULT_SUCCESS;
	//}
	
	ntyFree( tag );	
	return NTY_RESULT_SUCCESS;
}

int ntyBindPushAdministratorHandle(void *arg) {
	ntylog("ntyBindPushAdministratorHandle begin\n");
	VALUE_TYPE *tag = (VALUE_TYPE*)arg;
	if (tag == NULL) return NTY_RESULT_ERROR;
	C_DEVID devId = tag->gId;
	C_DEVID proposer = tag->fromId;
	C_DEVID admin = tag->toId;

	U8 *phnum = (U8 *) malloc( 32 );
	if( phnum == NULL ){
		ntylog("ntyBindPushAdministratorHandle malloc phnum failed\n");
	}
	memset( phnum, 0, 32 );
	ntyQueryPhoneBookSelectHandle(devId, proposer, phnum);
	ntylog("ntyBindPushAdministratorHandle phonenum:%s\n", phnum);

	if ( tag->Tag == NULL ){ 	
		ntylog("ntyBindPushAdministratorHandle tag->Tag==NULL return\n");
		ntyFree( phnum );
		return NTY_RESULT_ERROR;
	}
	BindReq *pBindReq = (BindReq*)tag->Tag;
	int msgId = tag->arg;

	BindConfirmPush *pBindConfirmPush = malloc(sizeof(BindConfirmPush));
	if (pBindConfirmPush == NULL) {
		goto exit;
	}
	pBindConfirmPush->result.IMEI = pBindReq->IMEI;
	pBindConfirmPush->result.proposer = phnum;
	pBindConfirmPush->result.userName = pBindReq->bind.userName;
	
	char ids[30] = {0};
	sprintf(ids, "%d", msgId);
	pBindConfirmPush->result.msgId = ids;
	char *jsonstring = ntyJsonWriteBindConfirmPush(pBindConfirmPush);
	if ( jsonstring == NULL ){
		ntylog("ntyBindPushAdministratorHandle jsonstring is NULL\n");
		goto exit;
	}
	ntylog("------------ntyBindPushAdministratorHandle send json to admin-------------\n");
	//send json to admin
	ntySendBindConfirmPushResult(proposer, devId, admin, (U8*)jsonstring, strlen(jsonstring));
	
exit:
	ntyFree( phnum );
	ntyFree( pBindReq );
	ntyFree( pBindConfirmPush );
	ntyFree( jsonstring );
	ntyFree( tag );
	
	ntylog("ntyBindPushAdministratorHandle end\n");
	return 0;
}

int ntyBindParserJsonHandle(void *arg) {
	VALUE_TYPE *tag = (VALUE_TYPE*)arg;
	if (tag == NULL) return NTY_RESULT_ERROR;
	JSON_Value *json = (JSON_Value*)tag->Tag;
	if ( json == NULL ) {
		ntylog( " ntyBindParserJsonHandle Json Error\n ");
		return NTY_RESULT_ERROR;
	}
	ntylog("ntyBindParserJsonHandle begin \n");
	BindReq *pBindReq = malloc(sizeof(BindReq));
	if (pBindReq == NULL) return NTY_RESULT_ERROR;
	ntyJsonBind(json, pBindReq);

	C_DEVID proposer = tag->fromId;
	C_DEVID devId = tag->gId;
	C_DEVID admin = tag->toId;
	U8 *name = (U8*)pBindReq->bind.watchName;
	U8 *wimage = (U8*)pBindReq->bind.watchImage;
	U8 *call = (U8*)pBindReq->bind.userName;
	U8 *uimage = (U8*)pBindReq->bind.userImage;

	if ( tag->arg == NTY_BIND_ACK_FIRST_BIND ) {
		U32 msgId = 0;
		//U8 phnum[32] = {0};
		U8 *phnum = (U8 *)malloc( 32 );
		if( phnum == NULL ){
			ntylog("ntyBindParserJsonHandle malloc phnum failed\n");
		}
		memset( phnum, 0, 32 );
		int ret = ntyQueryAdminGroupInsertHandle(devId, name, proposer, call, wimage, uimage, phnum, &msgId);
		if ( ret != NTY_RESULT_SUCCESS ) {
			ntylog("ntyBindParserJsonHandle --> ntyQueryAdminGroupInsertHandle failed\n");
			ntyFree( phnum );
			ntyFreeJsonValue(json);
			ntyFree(tag);
			return ret;
		}
		ntylog( "ntyBindParserJsonHandle ntyQueryAdminGroupInsertHandle after phonum:%s\n", phnum );
#if 1 //Update By WangBoJing Bind Add
		void *heap = ntyBHeapInstance();
		NRecord *record = ntyBHeapSelect(heap, proposer);
		if (record != NULL) {
			Client *aclient = record->value;
			if (aclient != NULL) {
				ntyVectorInsert(aclient->friends, &devId, sizeof(C_DEVID));
			}
		}
		record = ntyBHeapSelect(heap, devId);
		if (record != NULL) {
			Client *dclient = record->value;
			if (dclient != NULL) {
				ntyVectorInsert(dclient->friends, &proposer, sizeof(C_DEVID));
			}	
		}
#endif
		//add contacts msg, send to watch
		DeviceAddContactsAck *pDeviceAddContactsAck = malloc(sizeof(DeviceAddContactsAck));
		if ( pDeviceAddContactsAck == NULL ) {
			ntylog("ntyBindParserJsonHandle --> malloc DeviceAddContactsAck failed\n");
			ntyFree( phnum );
			ntyFreeJsonValue( json );
			ntyFree( tag );
			return -1;
		}
		memset( pDeviceAddContactsAck, 0, sizeof(DeviceAddContactsAck) );
		char bufIMEI[64] = {0};
		sprintf(bufIMEI, "%llx", devId);
		char contactsId[16] = {0};
		sprintf(contactsId, "%d", msgId);
		char add[16] = {0};
		char category[16] = {0};
		strcat(add, "Add");
		strcat(category, "Contacts");
		pDeviceAddContactsAck->id = contactsId;
		pDeviceAddContactsAck->IMEI = bufIMEI;
		pDeviceAddContactsAck->category = category;
		pDeviceAddContactsAck->action = add;
		pDeviceAddContactsAck->contacts.id = contactsId;
		pDeviceAddContactsAck->contacts.name = call;
		pDeviceAddContactsAck->contacts.image = uimage;
		pDeviceAddContactsAck->contacts.telphone = phnum;

		C_DEVID fromId = proposer;
		C_DEVID toId = devId;
		char *jsonagree = ntyJsonWriteDeviceAddContacts(pDeviceAddContactsAck);
		if ( jsonagree == NULL ){
			ntylog( "ntyBindParserJsonHandle jsonagree is NULL\n" );
			ntyFree( pDeviceAddContactsAck );
			ntyFree( phnum );
			ntyFreeJsonValue( json );
			ntyFree( tag );
			return -1;
		}
		ntylog("ntyBindParserJsonHandle jsonagree: %s\n", jsonagree);
		ret = ntySendRecodeJsonPacket( fromId, toId, jsonagree, (int)strlen(jsonagree) );
		if ( ret < 0 ) {
			ntylog(" ntyBindParserJsonHandle --> SendCommonReq Exception\n");
		}
		ntyJsonFree( jsonagree );
		ntyFree( pDeviceAddContactsAck );
		ntyFree( phnum );
		ntyFreeJsonValue(json);
		ntyFree(tag);
		return ret;
	} else if (tag->arg == NTY_BIND_ACK_SUCCESS) {
		int msgId = 0;
		//绑定确认添加
		ntyQueryBindConfirmInsertHandle(admin, devId, name, wimage, proposer, call, uimage, &msgId);
		tag->Tag = (U8*)pBindReq;
		tag->arg = msgId;
		tag->Type = MSG_TYPE_BIND_PUSH_ADMINISTRATOR_HANDLE;
		tag->cb = ntyBindPushAdministratorHandle;
		ntyDaveMqPushMessage(tag);
		//ntyFreeJsonValue(json);
		return NTY_RESULT_PROCESS;
	}else{}
	
	ntyFreeJsonValue(json);
	ntyFree( tag );
	return 0;
}


int ntyAdministratorSelectHandle(void *arg) {
	VALUE_TYPE *tag = (VALUE_TYPE*)arg;

	if (tag == NULL) return NTY_RESULT_ERROR;

	C_DEVID fromId = tag->fromId;
	C_DEVID devId = tag->gId;
	C_DEVID admin = 0x0;

	ntylog(" ntyAdministratorSelectHandle fromId:%lld, devId:%lld\n", fromId, devId);
	if(NTY_RESULT_FAILED == ntyQueryAdminSelectHandle(devId, &admin)) {
		ntylog(" ntyAdministratorSelectHandle Failed\n");
		if (tag->Tag != NULL) {
			JSON_Value *json = (JSON_Value*)tag->Tag;
			if (json != NULL) {
				ntyFreeJsonValue(json);
			}
			tag->Tag = NULL;
		}
		free(tag);

		return NTY_RESULT_ERROR;
	}

	tag->toId = admin;
	tag->Type = MSG_TYPE_BIND_JSON_PARSER_HANDLE;
	tag->cb = ntyBindParserJsonHandle;
	ntyDaveMqPushMessage(tag);

	return NTY_RESULT_SUCCESS;
}




int ntyBindDeviceCheckStatusReqHandle(void *arg) {
	VALUE_TYPE *tag = (VALUE_TYPE*)arg;
	if (tag == NULL) return NTY_RESULT_ERROR;
	
	C_DEVID fromId = tag->fromId;
	C_DEVID devId = tag->gId;

	ntylog(" ntyBindDeviceCheckStatusReqHandle fromId:%lld, devId:%lld\n", fromId, devId);
	int ret = ntyQueryDevAppGroupCheckSelectHandle(fromId, devId);

	if (ret == NTY_RESULT_FAILED) {
		ret = NTY_BIND_ACK_ERROR;
	}
	ntyProtoBindAck(fromId, devId, ret); //return to fromId

	ntylog(" ntyBindDeviceCheckStatusReqHandle ret:%d\n", ret);
	if (ret == NTY_BIND_ACK_FIRST_BIND) { //SQL first bind Watch
	
		tag->arg = NTY_BIND_ACK_FIRST_BIND;
		tag->toId = fromId;
		tag->Type = MSG_TYPE_BIND_JSON_PARSER_HANDLE;
		tag->cb = ntyBindParserJsonHandle;
		ntyDaveMqPushMessage(tag);
		
		return NTY_RESULT_SUCCESS;
	} else if (ret == NTY_BIND_ACK_SUCCESS){

		tag->arg = NTY_BIND_ACK_SUCCESS;
		tag->Type = MSG_TYPE_BIND_SELECT_ADMIN_HANDLE;
		tag->cb = ntyAdministratorSelectHandle;
		
		ntyDaveMqPushMessage(tag);

		return NTY_RESULT_SUCCESS;
	}else{}

	JSON_Value *json = (JSON_Value*)tag->Tag;
	if (json != NULL) {
		ntyFreeJsonValue(json);
	}
	ntyFree( tag );

	return NTY_RESULT_FAILED;

	
}


int ntyLocationBroadCastHandle(void *arg) {
	ntylog( "ntyLocationBroadCastHandle func run\n" );
	VALUE_TYPE *tag = (VALUE_TYPE*)arg;
	if (tag == NULL) return NTY_RESULT_ERROR;

	C_DEVID fromId = tag->fromId;
	C_DEVID gId = tag->gId;
	U8 *json = tag->Tag;
	int length = tag->length;
	ntySendLocationBroadCastResult(fromId, gId, json, length);

	if (tag->Tag == NULL) {
		free(tag->Tag);
	}
	free(tag);

	return NTY_RESULT_SUCCESS;
}


int ntyIOSPushHandle(void *arg) {
	VALUE_TYPE *tag = (VALUE_TYPE*)arg;
	ntylog("ntyIOSPushHandle --> begin\n");
	if (tag == NULL) return NTY_RESULT_ERROR;
	C_DEVID gId = tag->gId;
	U32 type = tag->arg;
	C_DEVID toId = tag->toId;
	U8 *msg = tag->Tag;

	void *heap = ntyBHeapInstance();
	NRecord *record = ntyBHeapSelect(heap, toId);
	if (record == NULL) goto exit;
	Client *pClient = (Client *)record->value;

#if 1 //add offline msg counter function. select 
	U32 counter = 0;
	ntyQueryOfflineMsgCounterSelectHandle(toId, &counter);
#endif

	if (pClient->deviceType == NTY_PROTO_CLIENT_IOS) {
		if (pClient->token != NULL) {
			ntylog("ntySendPushNotify IOS--> selfId:%lld  token:%s\n", toId, pClient->token);
			void *pushHandle = ntyPushHandleInstance();
			ntyPushNotifyHandle(pushHandle, gId, type, counter, msg, pClient->token, NTY_PUSH_CLIENT_DEVELOPMENT);
		}
		//goto exit;
	} else if (pClient->deviceType == NTY_PROTO_CLIENT_IOS_PUBLISH) {
		if (pClient->token != NULL) {
			ntylog("ntySendPushNotify IOSPUBLISH--> selfId:%lld  token:%s\n", toId, pClient->token);
			void *pushHandle = ntyPushHandleInstance();
			ntyPushNotifyHandle(pushHandle, gId, type, counter, msg, pClient->token, NTY_PUSH_CLIENT_PRODUCTION);
		}
		//goto exit;
	} else if (pClient->deviceType == NTY_PROTO_CLIENT_IOS_APP_B) {
		if (pClient->token != NULL) {
			ntylog("ntySendPushNotify IOS-B--> selfId:%lld  token:%s\n", toId, pClient->token);
			void *pushHandle = ntyPushHandleInstance();
			ntyPushNotifyHandle(pushHandle, gId, type, counter, msg, pClient->token, NTY_PUSH_CLIENT_DEVELOPMENT_B);
		}
		//goto exit;
	} else if (pClient->deviceType == NTY_PROTO_CLIENT_IOS_APP_B_PUBLISH) {
		if (pClient->token != NULL) {
			ntylog("ntySendPushNotify IOSPUBLISH-B--> selfId:%lld  token:%s\n", toId, pClient->token);
			void *pushHandle = ntyPushHandleInstance();
			ntyPushNotifyHandle(pushHandle, gId, type, counter, msg, pClient->token, NTY_PUSH_CLIENT_PRODUCTION_B);
		}
		//goto exit;
	}else{}
exit:
	ntyFree( msg );
	ntyFree( tag );
	
	return NTY_RESULT_SUCCESS;
}


int ntyCommonReqHandle(void *arg) {
	ntylog( "ntyCommonReqHandle func\n" );
	
	VALUE_TYPE *tag = (VALUE_TYPE*)arg;
	if (tag == NULL) return NTY_RESULT_ERROR;

	U8 *jsonstring = tag->Tag;
	C_DEVID fromId = tag->fromId;
	C_DEVID toId = tag->toId;
	int jsonlen = tag->length;

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

	free(tag);
}


