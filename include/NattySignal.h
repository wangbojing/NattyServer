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

#ifndef __NATTY_SIGNAL_H__
#define __NATTY_SIGNAL_H__

#include <signal.h>

#define NTY_SIGNAL_HUP		SIGHUP
#define NTY_SIGNAL_INT		SIGINT
#define NTY_SIGNAL_QUIT		SIGQUIT
#define NTY_SIGNAL_ILL		SIGILL
#define NTY_SIGNAL_TRAP		SIGTRAP
#define NTY_SIGNAL_IOT		SIGIOT
#define NTY_SIGNAL_BUS		SIGBUS
#define NTY_SIGNAL_FPE		SIGFPE
#define NTY_SIGNAL_KILL		SIGKILL
#define NTY_SIGNAL_USR1		SIGUSR1
#define NTY_SIGNAL_SEGV		SIGSEGV
#define NTY_SIGNAL_USR2		SIGUSR2
#define NTY_SIGNAL_PIPE		SIGPIPE
#define NTY_SIGNAL_ALRM		SIGALRM
#define NTY_SIGNAL_TERM		SIGTERM
#define NTY_SIGNAL_CHLD		SIGCHLD
#define NTY_SIGNAL_CONT		SIGCONT
#define NTY_SIGNAL_STOP		SIGSTOP
#define NTY_SIGNAL_TSTP		SIGTSTP
#define NTY_SIGNAL_TTIN		SIGTTIN
#define NTY_SIGNAL_TTOU		SIGTTOU
#define NTY_SIGNAL_URG		SIGURG
#define NTY_SIGNAL_XCPU		SIGXCPU
#define NTY_SIGNAL_XFSZ		SIGXFSZ
#define NTY_SIGNAL_VTALRM	SIGVTALRM
#define NTY_SIGNAL_PROF		SIGPROF
#define NTY_SIGNAL_WINCH	SIGWINCH
#define NTY_SIGNAL_IO		SIGIO
#define NTY_SIGNAL_PWR		SIGPWR



void ntySignalRegister(void);




#endif



