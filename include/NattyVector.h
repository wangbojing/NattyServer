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



#ifndef __NATTY_VECTOR_H__
#define __NATTY_VECTOR_H__

#include <sys/queue.h>
#include <string.h>
#include <stdlib.h>

#include "NattyAbstractClass.h"

#define NTY_VECTOR_MAX_COUNT		4096

typedef int NVECTOR_CB(void *self, void *arg);

typedef struct nKnot {
	LIST_ENTRY(nKnot) entries;
	void *data;
	int len;
} NKnot;

typedef struct nVector {
	const void *_;
	LIST_HEAD(vectorheader, nKnot) header;
	int num;
	int max_num;	
} NVector;


typedef struct nVectorHandle {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	void* (*add)(void *_self, void *data, int len);
	int (*del)(void *_self, void *timer);
	void (*iter)(void *_self, NVECTOR_CB *cb, void *arg);
} NVectorHandle;


void* ntyVectorCreator(void);
void* ntyVectorDestory(void *self);
void* ntyVectorInsert(void *self, void *data, int len);
int ntyVectorDelete(void *self, void *data);
void ntyVectorIterator(void *self, NVECTOR_CB *cb, void *arg);


void *ntyVectorGetNodeList(void *self, int *num);






#endif


