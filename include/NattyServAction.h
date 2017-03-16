/*
 *  Author : luopeng , email : hibroad@hotmail.com
 * 
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of Author. (C) 2017
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



#ifndef __NATTY_SERVACTION_H__
#define __NATTY_SERVACTION_H__

#include "NattyAbstractClass.h"
#include "NattyJson.h"

void ntyJsonRecvResult(C_DEVID devId, char *jsonresult);

void ntyJsonBroadCastRecvResult(C_DEVID selfId, C_DEVID gId, char *jsonresult);

void ntyJsonCommonResult(C_DEVID devId, const char *code);

void ntyJsonCommonExtendResult(C_DEVID devId, const char * code, int id);

void ntyJsonAddEfenceAction(C_DEVID AppId, C_DEVID toId, JSON_Value *json, U8 *jsonstring, U16 jsonlen);

void ntyJsonDelEfenceAction(C_DEVID AppId, C_DEVID toId, JSON_Value *json, U8 *jsonstring, U16 jsonlen);

void ntyJsonRunTimeAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen);

void ntyJsonTurnAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen);

void ntyJsonICCIDAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen);

void ntyJsonAddScheduleAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen);

void ntyJsonDelScheduleAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen);

void ntyJsonUpdateScheduleAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen);

void ntyJsonSelectScheduleAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen);

void ntyJsonTimeTablesAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen);

void ntyJsonLocationWIFIAction(C_DEVID fromId, C_DEVID toId, JSON_Value *json, U8 *jsonstring, U16 jsonlen);

void ntyJsonLocationLabAction(C_DEVID fromId, C_DEVID toId, JSON_Value *json, U8 *jsonstring, U16 jsonlen);

void ntyJsonWeatherAction(C_DEVID clientId, C_DEVID toId, JSON_Value *json, U8 *jsonstring, U16 jsonlen);

void ntyJsonContactsAction(C_DEVID AppId, C_DEVID devId, JSON_Value *json, U8 *jsonstring, U16 jsonlen);


#endif // __NATTY_SERVACTION_H__


