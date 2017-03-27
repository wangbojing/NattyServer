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

#include "NattySqlHandle.h" 
#include "NattyAbstractClass.h"
#include "NattyResult.h"
#include "NattyDaveMQ.h"
#include "NattyMessage.h"
#include "NattyServAction.h"

int ntyVoiceAckHandle(void *arg) {
	if (arg == NULL) return ;
	VALUE_TYPE *tag = arg;

	ntylog(" ntyVoiceAckHandle \n");

	C_DEVID fromId = tag->fromId;
	int msgId = tag->arg;

	int ret = ntyExecuteVoiceOfflineMsgDeleteHandle(msgId, fromId);
	if (ret == NTY_RESULT_FAILED) {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_ERR_DB_NOEXIST);
	} else {
		ntyJsonCommonResult(fromId, NATTY_RESULT_CODE_SUCCESS);
	}

	free(tag);

	return ret;
}

int ntyCommonAckHandle(void *arg) {
	if (arg == NULL) return ;
	VALUE_TYPE *tag = arg;

	ntylog(" ntyCommonAckHandle \n");

	C_DEVID fromId = tag->fromId;
	int msgId = tag->arg;

	int ret = ntyExecuteCommonOfflineMsgDeleteHandle(msgId, fromId);
	if (ret == NTY_RESULT_FAILED) {
		ntylog("ntyCommonAckHandle DB Error \n");
	}
	free(tag);

	return ret;
}


int ntyVoiceDataReqHandle(void *arg) {
	if (arg == NULL) return ;
	VALUE_TYPE *tag = arg;


	C_DEVID senderId = tag->fromId;
	C_DEVID gId = tag->toId;
	U8 *filename = tag->Tag;
	
	ntylog(" ntyVoiceDataReqHandle \n");
	U32 msgId = 0;

	//insert voice msg to db
	int ret = ntyQueryVoiceMsgInsertHandle(senderId, gId, filename, &msgId);
	if (ret != NTY_RESULT_FAILED) {
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
	if (arg == NULL) return ;
	VALUE_TYPE *tag = arg;

	C_DEVID fromId = tag->fromId;
	int msgId = tag->arg;

	ntyVoiceReqAction(fromId, msgId);

	return 0;
}









