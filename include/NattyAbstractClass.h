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





#ifndef __NATTY_ABSTRACT_CLASS_H__
#define __NATTY_ABSTRACT_CLASS_H__

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>


#define BYTE_WIDTH				1
#define WCHAR_WIDTH				2
#define WORD_WIDTH				4

//ntyLogInfo(format, ##__VA_ARGS__)//

#define NTY_DEBUG 	1
#if (NTY_DEBUG == 1) //catcher 
#define ntylog(format, ...) 		fprintf(stdout, format, ##__VA_ARGS__)
#define ntydbg(format, ...) 		fprintf(stdout, format, ##__VA_ARGS__)
#elif (NTY_DEBUG == 2) // Serial
#define ntylog(format, ...)
#define ntydbg(format, ...) 		
#elif (NTY_DEBUG == 3) //Log file
#define ntylog(format, ...)
#define ntydbg(format, ...) 
#else
#define ntylog(format, ...)
#define ntydbg(format, ...) 
#endif


typedef long long U64;
typedef unsigned int U32;
typedef unsigned short U16;
typedef unsigned char U8;
typedef long long C_DEVID;
typedef int (*HANDLE_CLIENTID)(void* client, C_DEVID id);
typedef int (*HANDLE_NOTIFY)(C_DEVID from, C_DEVID to);
typedef int (*HANDLE_MASS)(C_DEVID to, U8 *data, int length);


typedef struct {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
} AbstractClass;


void *New(const void *_class, ...);
void Delete(void *_class);
unsigned long cmpxchg(void *addr, unsigned long _old, unsigned long _new, int size);



#endif


