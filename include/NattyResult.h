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


#ifndef __NATTY_RESULT_H__
#define __NATTY_RESULT_H__

typedef enum _NRESULT {
	NTY_RESULT_BUSY = -3,
	NTY_RESULT_ERROR = -2, //param error
	NTY_RESULT_FAILED = -1, //param failed
	NTY_RESULT_SUCCESS = 0,
	NTY_RESULT_EXIST = 1,
	NTY_RESULT_PROCESS = 2,
	NTY_RESULT_NOEXIST = 3,
	NTY_RESULT_NEEDINSERT = 4,
	
} NResult;

typedef enum _BRESULT {
	NTY_BIND_ACK_SUCCUESS = 0,
	NTY_BIND_ACK_USERID_NOEXIST = 1,
	NTY_BIND_ACK_DEVICE_NOEXIST = 2,
	NTY_BIND_ACK_HAVE_BOUND = 3,
	NTY_BIND_ACK_NO_ACTIVITE = 4,
	NTY_BIND_ACK_FIRST_BIND = 5,
	NTY_BIND_ACK_REJECT = 6,
	NTY_BIND_ACK_ERROR = 7,
} BResult;

#endif


