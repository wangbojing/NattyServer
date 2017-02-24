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

#ifndef __NATTY_EXCEPTION_H__
#define __NATTY_EXCEPTION_H__


#include <setjmp.h>


typedef struct tagExcepSign {
	jmp_buf _StackInfo;
	int _ExcepType;
} ExcepSign;


#define ExcepType(ExcepSign)  	((ExcepSign)._ExcepType)

#define Try(ExcepSign) 	if ((ExcepSign)._ExcepType == setjmp((ExcepSign)._StackInfo) == 0)

#define Catch(ExcepSign, ExcepType) 	else if ((ExcepSign)._ExcepType == (ExcepType))

#define CatchElse(ExcepSign) 	else if ((ExcepSign)._ExcepType < 0)

#define Throw(ExcepSign, ExcepType)		longjmp((ExcepSign)._StackInfo, ExcepType)

#endif











