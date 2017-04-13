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

#include "../include/NattyBPlusTree.h"

#include "NattySqlHandle.h" 
#include "NattyAbstractClass.h"
#include "NattyResult.h"
#include "NattyDaveMQ.h"
#include "NattyMessage.h"
#include "NattyServAction.h"
#include "NattyUdpServer.h"

int ntyReadOfflineVoiceHandle(void *arg);


int ntyVoiceAckHandle(void *arg) {
	if (arg == NULL) return NTY_RESULT_ERROR;
	VALUE_TYPE *tag = arg;

	ntylog(" ntyVoiceAckHandle \n");

	C_DEVID fromId = tag->fromId;
	int msgId = tag->arg;

	int ret = ntyExecuteVoiceOfflineMsgDeleteHandle(msgId, fromId);
	if (ret == NTY_RESULT_FAILED) {
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

	tag->Type = MSG_TYPE_VOICE_OFFLINE_READ_HANDLE;
	tag->cb = ntyReadOfflineVoiceHandle;
	return ntyDaveMqPushMessage(tag);
	
#endif

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

	
	tag->fromId = fromId;
	tag->Type = MSG_TYPE_OFFLINE_MSG_REQ_HANDLE;
	tag->cb = ntyOfflineMsgReqHandle;

	ntyDaveMqPushMessage(tag);


	return ret;
}


int ntyVoiceDataReqHandle(void *arg) {
	if (arg == NULL) return NTY_RESULT_ERROR;
	VALUE_TYPE *tag = arg;


	C_DEVID senderId = tag->fromId;
	C_DEVID gId = tag->toId;
	U8 *filename = tag->Tag;
	
	ntylog(" ntyVoiceDataReqHandle \n");
	U32 msgId = 0;

	//insert voice msg to db
	int ret = ntyQueryVoiceMsgInsertHandle(senderId, gId, filename, &msgId);
	if (ret == NTY_RESULT_SUCCESS) {
		ntyJsonCommonResult(senderId, NATTY_RESULT_CODE_SUCCESS);
	} else { //
		ntyJsonCommonResult(senderId, NATTY_RESULT_CODE_ERR_DB_OPERATION);
		return ret;
	}
	//broadcast to all id
	//json is null, length is 0
	ntySendVoiceBroadCastResult(senderId, gId, NULL, 0, msgId);
}


int ntyVoiceReqHandle(void *arg) {
	if (arg == NULL) return NTY_RESULT_ERROR;
	VALUE_TYPE *tag = arg;

	C_DEVID fromId = tag->fromId;
	int msgId = tag->arg;

	int ret = ntyVoiceReqAction(fromId, msgId);
	if (NTY_RESULT_FAILED == ret) {
		//delete fromId And MsgId
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


int ntyBindConfirm(C_DEVID adminId, C_DEVID *ProposerId, C_DEVID DeviceId, U32 msgId, U8 *phonenum) {

	C_DEVID AppId = 0x0;

#if 0
	int ret = ntyQueryDevAppGroupInsertHandle(AppId, DeviceId);
#else
	int ret = ntyExecuteDevAppGroupBindInsertHandle(msgId, ProposerId, phonenum);
#endif
	memcpy(&AppId, ProposerId, sizeof(C_DEVID));
	if (ret == -1) {
		ntylog(" ntyBindDevicePacketHandleRequest --> DB Exception\n");
		ret = 4;
	} else if (ret == 0) { //Bind Success Update RBTree

		void *heap = ntyBHeapInstance();
		NRecord *record = ntyBHeapSelect(heap, AppId);
		if (record != NULL) {
			Client *aclient = record->value;
			ASSERT(aclient != NULL);
			ntyVectorInsert(aclient->friends, &DeviceId, sizeof(C_DEVID));
		}

		record = ntyBHeapSelect(heap, DeviceId);
		if (record != NULL) {
			Client *dclient = record->value;
			ASSERT(dclient != NULL);
			ntyVectorInsert(dclient->friends, &AppId, sizeof(C_DEVID));
		}

	}
	ntylog(" ntyBindConfirmReqPacketHandleRequest --> ntyJsonCommonResult\n");
	//ntyJsonCommonResult(adminId, NATTY_RESULT_CODE_SUCCESS);

	return ret;
}


int ntyBindConfirmReqHandle(void *arg) {
	if (arg == NULL) return NTY_RESULT_ERROR;

	VALUE_TYPE *tag = arg;
	U8 json[PACKET_BUFFER_SIZE] = {0};

	C_DEVID adminId = tag->fromId;
	C_DEVID proposerId = tag->toId; //read from TB_BIND_CONFIRM proposerId by msgid
	C_DEVID gId = tag->gId;
	U32 msgId = tag->arg;

	char msgIds[64] = {0};
	char answer[64] = {0};
	char imei[64] = {0};
	char bindConfirmReq[64] = {0};
	memcpy(bindConfirmReq, NATTY_USER_PROTOCOL_BINDCONFIRMREQ, strlen(NATTY_USER_PROTOCOL_BINDCONFIRMREQ));
	sprintf(imei, "%llx", gId);
	sprintf(msgIds, "%d", msgId);
	
	U8 flag = tag->u8LocationType;
	ntylog(" ntyBindConfirmReqHandle flag:%d, %lld\n", flag, proposerId);
	if (flag == 1) { 
		char phonenum[64] = {0};
		int ret = ntyBindConfirm(adminId, &proposerId, gId, msgId, phonenum); 
#if 0
		//select phone 
		ntyQueryPhoneBookSelectHandle(gId, proposerId, phonenum);
		ntydbg("ntyBindConfirmReqHandle->flag:%d, phnum:%s\n", flag, phonenum);
#endif
		BindBroadCast *pBindBroadCast = malloc(sizeof(BindBroadCast));
		memcpy(answer, NATTY_USER_PROTOCOL_AGREE, strlen(NATTY_USER_PROTOCOL_AGREE));
		
		pBindBroadCast->result.IMEI = imei;
		pBindBroadCast->result.category = bindConfirmReq;
		pBindBroadCast->result.proposer = phonenum;
		pBindBroadCast->result.answer = answer;
		
		char *jsonresult = ntyJsonWriteBindBroadCast(pBindBroadCast);
		ntydbg("ntyJsonBroadCastRecvResult->%s\n",  jsonresult);

		//保存离线数据到数据库
		int tempMsgId = 0;
		ret = ntyExecuteCommonMsgInsertHandle(adminId, gId, jsonresult, &tempMsgId);
		
		ntyJsonBroadCastRecvResult(adminId, gId, (U8*)jsonresult, msgId);
		ntyJsonFree(jsonresult);
		free(pBindBroadCast);
		
	} else if (flag == 0) {
	
		char phonenum[64] = {0};
#if 0
		int ret = ntyBindConfirm(adminId, &proposerId, gId, msgId, phonenum); 
#else
		int ret = ntyExecuteBindConfirmDeleteHandle(msgId, &proposerId);
		ntydbg("ntyJsonBroadCastRecvResult->%lld\n", proposerId);
#endif
		BindBroadCast *pBindBroadCast = malloc(sizeof(BindBroadCast));
		memcpy(answer, NATTY_USER_PROTOCOL_REJECT, strlen(NATTY_USER_PROTOCOL_REJECT));
		
		pBindBroadCast->result.IMEI = imei;
		pBindBroadCast->result.category = bindConfirmReq;
		pBindBroadCast->result.proposer = phonenum;
		pBindBroadCast->result.answer = answer;
		
		char *jsonresult = ntyJsonWriteBindBroadCast(pBindBroadCast);
		ntydbg("ntyJsonBroadCastRecvResult->%s\n",  jsonresult);

		//保存离线数据到数据库
		int tempMsgId = 0;
		ret = ntyExecuteCommonMsgToProposerInsertHandle(proposerId, gId, jsonresult, &tempMsgId);
		
		
		ntyProtoBindAck(proposerId, gId, 6); //reject

		ntyJsonFree(jsonresult);
		free(pBindBroadCast);
	}
	
	free(tag);
	
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
	free(tag);

	return ret;
}

int ntyReadOfflineVoiceHandle(void *arg) {
	VALUE_TYPE *tag = (VALUE_TYPE*)arg;
	if (tag == NULL) return NTY_RESULT_ERROR;
	
	C_DEVID fromId = tag->fromId;

	int ret = ntyReadOfflineVoiceMsgAction(fromId);
	if (ret == NTY_RESULT_NOEXIST) {
		
		tag->Type = MSG_TYPE_BIND_OFFLINE_PUSH_HANDLE;
		tag->cb = ntyReadOfflineBindMsgHandle;
		return ntyDaveMqPushMessage(tag);
	}
	free(tag);

	return ret;
}

int ntyOfflineMsgReqHandle(void *arg) {
	VALUE_TYPE *tag = (VALUE_TYPE*)arg;
	if (tag == NULL) return NTY_RESULT_ERROR;
	
	C_DEVID fromId = tag->fromId;

	int ret = ntyReadOfflineCommonMsgAction(fromId);
	if (ret == NTY_RESULT_NOEXIST) {
		
		tag->Type = MSG_TYPE_VOICE_OFFLINE_READ_HANDLE;
		tag->cb = ntyReadOfflineVoiceHandle;
		return ntyDaveMqPushMessage(tag);
	}
	free(tag);

	return ret;
}

int ntyBindPushAdministratorHandle(void *arg) {
	VALUE_TYPE *tag = (VALUE_TYPE*)arg;
	
	if (tag == NULL) return NTY_RESULT_ERROR;

	C_DEVID devId = tag->gId;
	C_DEVID proposer = tag->fromId;
	C_DEVID admin = tag->toId;

	char phonenum[30] = {0};
	ntyQueryPhoneBookSelectHandle(devId, proposer, phonenum);
	ntylog(" ntyBindPushAdministratorHandle : %s\n", phonenum);

	if (tag->Tag == NULL) return NTY_RESULT_ERROR;
	
	BindReq *pBindReq = (BindReq*)tag->Tag;
	
	int msgId = tag->arg;

	BindConfirmPush *pBindConfirmPush = malloc(sizeof(BindConfirmPush));
	pBindConfirmPush->result.IMEI = pBindReq->IMEI;
	pBindConfirmPush->result.proposer = phonenum;
	pBindConfirmPush->result.userName = pBindReq->bind.userName;
	
	char ids[30] = {0};
	sprintf(ids, "%d", msgId);
	pBindConfirmPush->result.msgId = ids;
	char *jsonstring = ntyJsonWriteBindConfirmPush(pBindConfirmPush);

	ntydbg("-----------------------send json to admin---------------------------------\n");
		//send json to admin
	ntySendBindConfirmPushResult(proposer, admin, (U8*)jsonstring, strlen(jsonstring));

	//release 
	free(pBindConfirmPush);
	free(pBindReq);
	free(tag);
}

int ntyBindParserJsonHandle(void *arg) {
	VALUE_TYPE *tag = (VALUE_TYPE*)arg;

	if (tag == NULL) return NTY_RESULT_ERROR;
	
	JSON_Value *json = (JSON_Value*)tag->Tag;
	if (json == NULL) {
		ntylog( " ntyBindParserJsonHandle Json Error\n ");
		return NTY_RESULT_ERROR;
	}

	ntylog(" ntyBindParserJsonHandle \n");
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

	if (tag->arg == NTY_BIND_ACK_FIRST_BIND) {

		ntyFreeJsonValue(json);
		free(tag);
		
		return ntyQueryAdminGroupInsertHandle(devId, name, proposer, call, wimage, uimage);
		
	} else if (tag->arg == NTY_BIND_ACK_SUCCUESS) {
		int msgId = 0;
		//绑定确认添加
		ntyQueryBindConfirmInsertHandle(admin, devId, name, wimage, proposer, call, uimage, &msgId);

		ntyFreeJsonValue(json);
		tag->Tag = (U8*)pBindReq;
	
		tag->arg = msgId;
		tag->Type = MSG_TYPE_BIND_PUSH_ADMINISTRATOR_HANDLE;
		tag->cb = ntyBindPushAdministratorHandle;
		ntyDaveMqPushMessage(tag);

		return NTY_RESULT_PROCESS;
		
	}

	ntyFreeJsonValue(json);
	free(tag);
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
	if (ret == NTY_BIND_ACK_FIRST_BIND) { //SQL first band Watch
	
		tag->arg = NTY_BIND_ACK_FIRST_BIND;
		tag->toId = fromId;
		tag->Type = MSG_TYPE_BIND_JSON_PARSER_HANDLE;
		tag->cb = ntyBindParserJsonHandle;
		ntyDaveMqPushMessage(tag);
		
		return NTY_RESULT_SUCCESS;
	} else if (ret == NTY_BIND_ACK_SUCCUESS){

		tag->arg = NTY_BIND_ACK_SUCCUESS;
		tag->Type = MSG_TYPE_BIND_SELECT_ADMIN_HANDLE;
		tag->cb = ntyAdministratorSelectHandle;
		
		ntyDaveMqPushMessage(tag);

		return NTY_RESULT_SUCCESS;
	}

	JSON_Value *json = (JSON_Value*)tag->Tag;
	if (json != NULL) {
		ntyFreeJsonValue(json);
	}
	free(tag);

	return NTY_RESULT_FAILED;

	
}




