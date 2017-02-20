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


#ifndef __NATTY_BPLUS_TREE_H__
#define __NATTY_BPLUS_TREE_H__


#include "NattyAbstractClass.h"
#include "NattyResult.h"
#include "NattyTimer.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef bool
#define bool 	char
#endif

#define false 	0
#define true 	1


#define DEFAULT_ORDER			7
#define NTY_BPLUSTREE_ORDER		DEFAULT_ORDER

typedef unsigned long long NTY_ID;


typedef struct _BPNode {
	void **pointers;
	NTY_ID *keys;
	struct _BPNode *parent;
	bool isLeaf;
	int nKeys;
	struct _BPNode *next;
} BPNode;

typedef BPNode		NBPNode;
typedef void*	RECORDTYPE;

typedef struct _Record {
	RECORDTYPE value;
} NRecord;

typedef struct _BPTreeHeap {
	const void *_;
	NBPNode *root;
	int count;
} BPTreeHeap;

typedef struct _BPTreeHeapHandle {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	int (*insert)(void *_self, NTY_ID key, RECORDTYPE value);
	int (*delete)(void *_self, NTY_ID key);
	int (*update)(void *_self, NTY_ID key, RECORDTYPE value, size_t size);
	void* (*select)(void *_self, NTY_ID key);
} BPTreeHeapHandle;

typedef BPTreeHeap NBHeap;
typedef BPTreeHeapHandle NBHeapHandle;


void* ntyBHeapInstance(void);
void ntyBHeaRelease(void *self);
int ntyBHeapInsert(void *self, NTY_ID key, RECORDTYPE value);
int ntyBHeapDelete(void *self, NTY_ID key);
void* ntyBHeapSelect(void *self, NTY_ID key);
int ntyBHeapUpdate(void *self, NTY_ID key, RECORDTYPE value, size_t size);








#endif


