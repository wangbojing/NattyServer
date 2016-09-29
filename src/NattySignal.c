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



#include "NattySignal.h"
#include "NattyDBOperator.h"
#include "NattyAbstractClass.h"

#include <errno.h>


#define SIGNAL_REGISTER(x)		signal(x, ntySignalProcess)

static void ntySignalProcess(int sign_no) {
	ntylog(" ntySignalProcess --> sign_no %d \n", sign_no);
	ntylog(" ntySignalProcess --> errno %d \n", errno, strerror(errno));
	switch (sign_no) {
		case NTY_SIGNAL_INT: 
		case NTY_SIGNAL_QUIT: 
		case NTY_SIGNAL_TERM: 
		case NTY_SIGNAL_KILL:	
		case NTY_SIGNAL_FPE: 
		case NTY_SIGNAL_SEGV: { 
			ntyConnectionPoolDeInit(); //Release MYSQL SOCKET CONNECTION
			break;
		}
	}
}


void ntySignalRegister(void) {
	SIGNAL_REGISTER(NTY_SIGNAL_INT);
	SIGNAL_REGISTER(NTY_SIGNAL_QUIT);
	SIGNAL_REGISTER(NTY_SIGNAL_TERM);
	SIGNAL_REGISTER(NTY_SIGNAL_KILL);
	SIGNAL_REGISTER(NTY_SIGNAL_FPE);
	SIGNAL_REGISTER(NTY_SIGNAL_SEGV);
}

