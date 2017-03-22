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
 
 
#ifndef __NATTY_LOG_H__
#define __NATTY_LOG_H__

#include <zlog.h>

#include "NattyAbstractClass.h"

typedef zlog_category_t nLogCategory;


#define NATTY_LOG_CONF		"/usr/local/Natty/nlog.conf"

#define NATTY_LOG_OUTSTREAM	nCategory
#define NATTY_LOG_CATEGORY	"nCategory"

#define NATTY_LOG_BUFFER_LENGTH			1024

//#define nLogInfo(format, ...)	zlog_info(NATTY_LOG_OUTSTREAM, format, ##__VA_ARGS__)

void ntyLogShow(const char *format, ...);



typedef struct _NLOG {
	const void *_;
	nLogCategory *nCategory;
} nLog;

typedef struct _LOGHANDLE {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	void (*logt)(void *self, const char *format);
} LogHandle;

#endif

 







