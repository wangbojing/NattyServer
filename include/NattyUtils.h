/*
 *  Author : WangBoJing , email : 1989wangbojing@gmail.com
 * 
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of NALEX Inc. (C) 2016
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




#ifndef __NTY_UTILS_H__
#define __NTY_UTILS_H__

#include "NattyLetter.h"
#include "NattyAbstractClass.h"

#define JEMALLOC_NO_DEMANGLE 1
#define JEMALLOC_NO_RENAME	 1
#include <jemalloc/jemalloc.h>


typedef struct tm TimeStamp;

void ntyDisplay(void);


void ntyFree(void *p);
void *ntyMalloc(size_t size);
void *ntyJeMalloc(size_t size);


U32 ntyKMP(const char *text,const U32 text_length, const char *pattern,const U32 pattern_length, U32 *matches) ;
TimeStamp* ntyGetSystemTime(void);

U16 ntyU8ArrayToU16(U8 *buf);
U32 ntyU8ArrayToU32(U8 *buf);
void ntyU8ArrayToU64(U8 *buf, C_DEVID *id);
void ntyTimeCheckStamp(U8 *buf);


int ntySeparation(const U8 ch, const U8 *sequence, int length, U8 ***pChTable, int *Count); 
void ntyFreeTable(unsigned char ***table, int count);

char ntyIsAvailableNum(char *phnum);
int ntyCharToWchar(U8 *src, int length, wchar_t *dest);
TIMESTAMP ntyTimeStampGenrator(void);


#endif

