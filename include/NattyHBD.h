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

#ifndef __NATTY_HEARTBEAT_DETECT_H__
#define __NATTY_HEARTBEAT_DETECT_H__


#include "NattyTcpServer.h"
#include "NattyConfig.h"
#include "NattyRBTree.h"
#include "NattyFilter.h"
#include "NattySession.h"
#include "NattyTimer.h"
#include "NattyResult.h"


#define NATTY_HEARTBEAT_DURATION		(5*60)
#define NATTY_HEARTBEAT_RATIO			6/5
#define NATTY_HEARTBEAT_THRESHOLD		(NATTY_HEARTBEAT_DURATION*NATTY_HEARTBEAT_RATIO)

#define NATTY_DURATION_EVENT			(2*60)

#if 0
void ntyHeartBeatDetectTraversal(void *mainloop);
#else
int ntyCheckOnlineAlarmNotify(NITIMER_ID id, void *arg, int len);


#endif

#endif




