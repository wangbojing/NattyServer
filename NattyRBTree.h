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





#ifndef __NATTY_RBTREE_H__
#define __NATTY_RBTREE_H__

#include "NattyAbstractClass.h"

#define RED		1
#define BLACK		2


typedef struct _RBTreeNode {
	C_DEVID key;
	void *value;
	struct _RBTreeNode *right;
	struct _RBTreeNode *left;
	struct _RBTreeNode *parent;
	unsigned char color;
} RBTreeNode;

typedef struct _RBTree {
	const void *_;
	RBTreeNode *root;
	RBTreeNode *nil;
} RBTree;

typedef struct _RBTreeOpera {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	int (*insert)(void *_self, C_DEVID key, void *value);
	void* (*search)(void *_self, C_DEVID key);
	int (*delete)(void *_self, C_DEVID key);
	int (*update)(void *_self, C_DEVID key, void *value);
} RBTreeOpera;



void* ntyRBTreeInstance(void);
int ntyRBTreeInterfaceInsert(void *self, C_DEVID key, void *value);
void* ntyRBTreeInterfaceSearch(void *self, C_DEVID key);
int ntyRBTreeInterfaceDelete(void *self, C_DEVID key);
int ntyRBTreeInterfaceUpdate(void *self, C_DEVID key, void *value);
void ntyRBTreeRelease(void *self);





#endif



