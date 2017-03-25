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
	ntylog(" ntySignalProcess --> errno %d \n", errno);
	switch (sign_no) {
		case NTY_SIGNAL_INT: 
		case NTY_SIGNAL_TERM:  { 
			ntyConnectionPoolDeInit(); //Release MYSQL SOCKET CONNECTION
			break;
		}
#if 0
		case NTY_SIGNAL_PIPE: {
			break;
		}
		case NTY_SIGNAL_QUIT: 
		case NTY_SIGNAL_KILL:	
		case NTY_SIGNAL_FPE: 
		case NTY_SIGNAL_SEGV: {
			break;
		}
#endif
	}
}


void ntySignalRegister(void) {
	SIGNAL_REGISTER(NTY_SIGNAL_INT);
	SIGNAL_REGISTER(NTY_SIGNAL_TERM);
#if 0
	SIGNAL_REGISTER(NTY_SIGNAL_KILL);
	SIGNAL_REGISTER(NTY_SIGNAL_FPE);
	SIGNAL_REGISTER(NTY_SIGNAL_SEGV);
	SIGNAL_REGISTER(NTY_SIGNAL_PIPE);
	SIGNAL_REGISTER(NTY_SIGNAL_QUIT);
#endif
}

