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

#define JEMALLOC_NO_DEMANGLE 1
#define JEMALLOC_NO_RENAME	 1
#include <jemalloc/jemalloc.h>


#define BYTE_WIDTH				1
#define WCHAR_WIDTH				2
#define WORD_WIDTH				4

//ntyLogInfo(format, ##__VA_ARGS__)//
//fprintf(stdout, format, ##__VA_ARGS__)
#define NTY_DEBUG 	2
#if (NTY_DEBUG == 1) //catcher 
#define ntylog(format, ...) 		ntyLogInfo(format, ##__VA_ARGS__)
#define ntydbg(format, ...) 		fprintf(stdout, format, ##__VA_ARGS__)
#elif (NTY_DEBUG == 2) // Serial
#define ntylog(format, ...)			fprintf(stdout, format, ##__VA_ARGS__)
#define ntydbg(format, ...) 		fprintf(stdout, format, ##__VA_ARGS__)
#elif (NTY_DEBUG == 3) //Log file
#define ntylog(format, ...)			do { \
	ntyLogInfo(format, ##__VA_ARGS__);	 \
	fprintf(stdout, format, ##__VA_ARGS__);	\
	} while (0)
#define ntydbg(format, ...) 		fprintf(stdout, format, ##__VA_ARGS__)
#else
#define ntylog(format, ...)
#define ntydbg(format, ...) 
#endif

#define ASSERT(x)			assert(x)


#define NATTY_NULL_DEVID				0x0
#define NATTY_HOLDER_DEVID				0x1


typedef unsigned long long U64;
typedef signed long long S64;
typedef unsigned int U32;
typedef unsigned short U16;
typedef unsigned char U8;
typedef long long C_DEVID;
typedef long TIMESTAMP;
//typedef C_DEVID NTY_ID;

typedef   signed char   int8_t;
typedef unsigned char  uint8_t;
typedef   signed short  int16_t;
typedef unsigned short uint16_t;
typedef   signed int    int32_t;
typedef unsigned int   uint32_t;

typedef int (*HANDLE_CLIENTID)(void* client, C_DEVID id);
typedef int (*HANDLE_NOTIFY)(C_DEVID from, C_DEVID to);
typedef int (*HANDLE_MASS)(C_DEVID to, U8 *data, int length);
typedef int (*HANDLE_BROADCAST)(void* client, C_DEVID to, U8 *data, int length);
typedef int (*HANDLE_HEARTBEAT)(void* node, void *mainloop, TIMESTAMP stamp);

/*
 * AbstractClass
 * three layer to implement Object Oriented by C
 * 1. implement layer	: 
 * 2. Abstract layer: xxHandle Class defined
 * 3. interface layer: 
 */
typedef struct {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
} AbstractClass;


void *New(const void *_class, ...);
void Delete(void *_class);
unsigned long cmpxchg(void *addr, unsigned long _old, unsigned long _new, int size);



#endif


