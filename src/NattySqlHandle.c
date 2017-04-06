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

	ret = ntyReadOfflineVoiceMsgAction(fromId);
	if (ret == NTY_RESULT_NOEXIST) {
		//BindConfirmReq
		//
	}
	
	free(tag);

	return ret;
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
#if 1 //Update By WangBoJing 
	ret = ntyReadOfflineCommonMsgAction(fromId);
	if (ret == NTY_RESULT_NOEXIST) {
		ret = ntyReadOfflineVoiceMsgAction(fromId);
		if (ret == NTY_RESULT_NOEXIST) {
			//read
			//
		}
	}

#endif	
	free(tag);

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


int ntyBindConfirm(C_DEVID adminId, C_DEVID AppId, C_DEVID DeviceId, U32 msgId) {
#if 0
	int ret = ntyQueryDevAppGroupInsertHandle(AppId, DeviceId);
#else
	int ret = ntyExecuteDevAppGroupBindInsertHandle(msgId);
#endif
	if (ret == -1) {
		ntylog(" ntyBindDevicePacketHandleRequest --> DB Exception\n");
		ret = 4;
	} else if (ret == 0) { //Bind Success Update RBTree

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
	C_DEVID proposerId = tag->toId;
	C_DEVID gId = tag->gId;
	U32 msgId = tag->arg;

	
	U8 flag = tag->u8LocationType;
	ntylog(" ntyBindConfirmReqHandle flag:%d\n", flag);
	if (flag == 1) { 
		char answer[64] = {0};
		char phonenum[64] = {0};
		
		int ret = ntyBindConfirm(adminId, proposerId, gId, msgId); 

		ntyQueryPhoneBookSelectHandle(gId, proposerId, phonenum);
		ntydbg("ntyBindConfirmReqHandle->flag:%d, phnum:%s\n", flag, phonenum);
		
		BindBroadCast *pBindBroadCast = malloc(sizeof(BindBroadCast));
		memcpy(answer, NATTY_USER_PROTOCOL_AGREE, strlen(NATTY_USER_PROTOCOL_AGREE));
		char imei[64] = {0};
		sprintf(imei, "%llx", gId);
		pBindBroadCast->result.IMEI = imei;
		
		char bindConfirm[64] = {0};
		memcpy(bindConfirm, NATTY_USER_PROTOCOL_BINDCONFIRM, strlen(NATTY_USER_PROTOCOL_BINDCONFIRM));
		pBindBroadCast->result.category = bindConfirm;
		pBindBroadCast->result.proposer = phonenum;
		pBindBroadCast->result.answer = answer;
		
		char *jsonresult = ntyJsonWriteBindBroadCast(pBindBroadCast);
		ntydbg("ntyJsonBroadCastRecvResult->%s\n",  jsonresult);
		
		ntyJsonBroadCastRecvResult(adminId, gId, (U8*)jsonresult, msgId);
		ntyJsonFree(jsonresult);
		free(pBindBroadCast);

		
	} else if (flag == 0) {

		int ret = ntyExecuteBindConfirmDeleteHandle(msgId);
	
		ntydbg("ntyJsonBroadCastRecvResult->%lld\n", proposerId);
		ntyProtoBindAck(proposerId, gId, 6); //reject

		
	}

	free(tag);
	
}


int ntyLoginReqHandle(void *arg) {
	VALUE_TYPE *tag = (VALUE_TYPE*)arg;
	C_DEVID fromId = tag->fromId;

	int ret = ntyReadOfflineCommonMsgAction(fromId);
	if (ret == NTY_RESULT_NOEXIST) {
		ret = ntyReadOfflineVoiceMsgAction(fromId);
		if (ret == NTY_RESULT_NOEXIST) {
			//read 
			//
		}
	}
	
	free(tag);
}






