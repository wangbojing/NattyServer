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
	U16 count;
} RBTree;

typedef struct _RBTreeOpera {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	int (*insert)(void *_self, C_DEVID key, void *value);
	void* (*search)(void *_self, C_DEVID key);
	int (*delete)(void *_self, C_DEVID key);
	int (*update)(void *_self, C_DEVID key, void *value);
	void (*traversal)(void *_self, HANDLE_CLIENTID handle_FN);
	void (*notify)(void *_self, C_DEVID selfId, HANDLE_NOTIFY notify_FN);
} RBTreeOpera;



void* ntyRBTreeInstance(void);
int ntyRBTreeInterfaceInsert(void *self, C_DEVID key, void *value);
void* ntyRBTreeInterfaceSearch(void *self, C_DEVID key);
int ntyRBTreeInterfaceDelete(void *self, C_DEVID key);
int ntyRBTreeInterfaceUpdate(void *self, C_DEVID key, void *value);
void ntyRBTreeRelease(void *self);



void *ntyFriendsTreeInstance(void);
int ntyFriendsTreeInsert(void *self, C_DEVID key);
void* ntyFriendsTreeSearch(void *self, C_DEVID key);
int ntyFriendsTreeDelete(void *self, C_DEVID key);
void ntyFriendsTreeTraversal(void *self, HANDLE_CLIENTID handle_FN);
int ntyFriendsTreeIsExist(void *self, C_DEVID key);
int ntyFriendsTreeIsEmpty(const void *self);
void ntyFriendsTreeRelease(void *self);
C_DEVID* ntyFriendsTreeGetAllNodeList(const void *self);
U16 ntyFriendsTreeGetNodeCount(const void *self);
C_DEVID ntyFriendsTreeGetFristNodeKey(void *self);








#endif



