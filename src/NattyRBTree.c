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



#include <string.h>
#include <ev.h>

#include "NattyRBTree.h"
#include "NattyUdpServer.h"
#include "NattyConfig.h"

//static pthread_mutex_t rbtree_mutex = PTHREAD_MUTEX_INITIALIZER;


void ntyRBTreeLeftRotate(RBTree *T, RBTreeNode *x) {
	RBTreeNode *y = x->right;
	x->right = y->left;

	if (y->left != T->nil) {
		y->left->parent = x;
	}
	y->parent = x->parent;

	if (x->parent == T->nil) {
		T->root = y;
	} else if (x == x->parent->left) {
		x->parent->left = y;
	} else {
		x->parent->right = y;
	}

	y->left = x;
	x->parent = y;
}


void ntyRBTreeRightRotate(RBTree *T, RBTreeNode *x) {
	RBTreeNode *y = x->left;
	x->left = y->right;

	if (y->right != T->nil) {
		y->right->parent = x;
	}

	y->parent = x->parent;
	if (x->parent == T->nil) {
		T->root = y;
	} else if (x == x->parent->right) {
		x->parent->right = y;
	} else {
		x->parent->left = y;
	}

	y->right = x;
	x->parent = y;
}

void ntyRBTreeInsertFixup(RBTree *T, RBTreeNode *z) {
	while (z->parent->color == RED) {
		if (z->parent == z->parent->parent->left) {
			RBTreeNode *y = z->parent->parent->right;
			if (y->color == RED) {
				z->parent->color = BLACK;
				y->color = BLACK;
				z->parent->parent->color= RED;
				z = z->parent->parent;
			} else {
				if (z == z->parent->right) {
					z = z->parent;
					ntyRBTreeLeftRotate(T, z);
				} 
				
				z->parent->color = BLACK;
				z->parent->parent->color = RED;
				ntyRBTreeRightRotate(T, z->parent->parent);
			}
		} else {
			RBTreeNode *y = z->parent->parent->left;
			if (y->color == RED) {
				z->parent->color = BLACK;
				y->color = BLACK;
				z->parent->parent->color = RED;
				z = z->parent->parent;
			} else { 
				if (z == z->parent->left) {
					z = z->parent;
					ntyRBTreeRightRotate(T, z);
				} 
				
				z->parent->color = BLACK;
				z->parent->parent->color = RED;
				ntyRBTreeLeftRotate(T, z->parent->parent);
			}
		}
	}
	T->root->color = BLACK;
}

void ntyRBTreeInsert(RBTree *T, RBTreeNode *z) {
	RBTreeNode *y = T->nil;
	RBTreeNode *x = T->root;

	while (x != T->nil) {
		y = x;
		if (z->key < x->key) {
			x = x->left;
		} else if(z->key > x->key) {
			x = x->right;
		} else { //Exist
			return ;
		}
	}

	z->parent = y;
	if (y == T->nil) {
		T->root = z;
	} else if (z->key < y->key) {
		y->left = z;
	} else {
		y->right = z;
	}

	z->left = T->nil;
	z->right = T->nil;
	//z->parent = T->nil;
	z->color = RED;

	ntyRBTreeInsertFixup(T, z);
}

RBTreeNode* ntyTreeMiniMum(RBTree *T, RBTreeNode *x) {
	while (x->left != T->nil) {
		x = x->left;
	}
	return x;
}

RBTreeNode* ntyTreeMaxiMum(RBTree *T, RBTreeNode *x) {
	while (x->right != T->nil) {
		x = x->right;
	}
	return x;
}

RBTreeNode* ntyTreeSuccessor(RBTree *T, RBTreeNode *x) {
	RBTreeNode *y = x->parent;
	if (x->right != T->nil) {
		return ntyTreeMiniMum(T, x->right);
	}
	
	while ((y != T->nil) && (x == y->right)) {
		x = y;
		y = y->parent;
	}
	return y;
}



void ntyRBTreeDeleteFixup(RBTree *T, RBTreeNode *x) {
	while ((x != T->root) && (x->color == BLACK)) {
		if (x == x->parent->left) {
			RBTreeNode *w = x->parent->right;
			if (w->color == RED) {
				w->color = BLACK;
				x->parent->color = RED;
				ntyRBTreeLeftRotate(T, x->parent);
				w = x->parent->right;
			} 

			if ((w->left->color == BLACK) && (w->right->color == BLACK)) {
				w->color = RED;
				x = x->parent;
			} else {
				if (w->right->color == BLACK) {
					w->left->color = BLACK;
					w->color = RED;
					ntyRBTreeRightRotate(T, w);
					w = x->parent->right;
				}
				
				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->right->color = BLACK;
				ntyRBTreeLeftRotate(T, x->parent);
				
				x = T->root;
			}
		}else {
			RBTreeNode *w = x->parent->left;
			if (w->color == RED) {
				w->color = BLACK;
				x->parent->color = RED;
				ntyRBTreeRightRotate(T, x->parent);
				w = x->parent->left;
			} 

			if ((w->left->color == BLACK) && (w->right->color == BLACK)) {
				w->color = RED;
				x = x->parent;
			} else { 
				if (w->left->color == BLACK) {
					w->right->color = BLACK;
					w->color = RED;
					ntyRBTreeLeftRotate(T, w);
					w = x->parent->left;
				}

				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->left->color = BLACK;
				ntyRBTreeRightRotate(T, x->parent);
				
				x = T->root;
			}
		}
	}
	x->color = BLACK;
}


RBTreeNode* ntyRBTreeDelete(RBTree *T, RBTreeNode *z) {
	RBTreeNode *y = T->nil;
	RBTreeNode *x = T->nil;
	
	
	if ((z->left == T->nil) || (z->right == T->nil)) {
		y = z;
	} else {
		y = ntyTreeSuccessor(T, z);
	}

	if (y->left != T->nil) {
		x = y->left;
	} else if (y->right != T->nil) {
		x = y->right;
	}

	x->parent = y->parent;
	if (y->parent == T->nil) {
		T->root = x;
	} else if (y == y->parent->left) {
		y->parent->left = x;
	} else {
		y->parent->right = x;
	}
		
	if (y != z) {
#if ENABLE_RBTREE_MUTEX
		pthread_mutex_lock(&T->rbtree_mutex);
		z->key = y->key;
		#if 0
		z->value = y->value;
		#else
		Client *pcz = z->value;
		Client *pcy = y->value;
		
		//memcpy(pcz->friends, pcy->friends, sizeof(RBTree));
		pcz->friends = pcy->friends;
		//pcz->watcher = pcy->watcher;
		//memcpy(pcz->watcher, pcy->watcher, sizeof(struct ev_io));
		memcpy(z->value, y->value, sizeof(Client));
		#endif
		pthread_mutex_unlock(&T->rbtree_mutex);
#else
		z->key = y->key;
		//z->value = y->value;
		memcpy(z->value, y->value, sizeof(Client));
#endif
	}
#if ENABLE_RBTREE_MUTEX
	pthread_mutex_lock(&T->rbtree_mutex);
	if (y->color == BLACK) {
		ntyRBTreeDeleteFixup(T, x);
	}
	pthread_mutex_unlock(&T->rbtree_mutex);
#else
	if (y->color == BLACK) {
		ntyRBTreeDeleteFixup(T, x);
	}
#endif
	return y;
}

RBTreeNode* ntyRBTreeSearch(RBTree *T, C_DEVID key) {
	RBTreeNode *node = T->root;

	while (node != T->nil) {
		if (key < node->key) {
			node = node->left;
		} else if (key > node->key) {
			node = node->right;
		} else {
			return node;
		}
	}
	return T->nil;
}


void* ntyRBTreeOperaCtor(void *_self, va_list *params) {
	RBTree *self = _self;
	self->nil = (RBTreeNode*)malloc(sizeof(RBTreeNode));
	self->nil->color = BLACK;
	self->root = self->nil;
	self->count = 0;
	self->rbtree_delete_lock = 0;

	int arg = va_arg(params, int);
	if (arg == 1) {
		self->heap_flag = 1; //data heap;
	} else {
		self->heap_flag = 0; //friend heap
	}

	pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;
	memcpy(&self->rbtree_mutex, &blank_mutex, sizeof(blank_mutex));
		
	return self;
}

static void ntyPosOrderRelease(RBTree *T, RBTreeNode *node) {
	if (node != T->nil) {
		ntyPosOrderRelease(T, node->left);
		ntyPosOrderRelease(T, node->right);
		free(node);
	}
	return ;
}



void* ntyRBTreeOperaDtor(void *_self) {
	RBTree *self = _self;
	
	ntyPosOrderRelease(self, self->root);
	free(self->nil);

	return self;
}

int ntyRBTreeOperaInsert(void *_self, C_DEVID key, void *value) {
	RBTree *self = _self;

	RBTreeNode *node = ntyRBTreeSearch(self, key);
	if (node == self->nil) {
		node = (RBTreeNode*)malloc(sizeof(RBTreeNode));
		node->key = key;
		node->value = value;
#if ENABLE_RBTREE_MUTEX		
		if(0 == cmpxchg(&self->rbtree_delete_lock, 0, 1, WORD_WIDTH)) {
			ntyRBTreeInsert(self, node);
			self->count ++;
			self->rbtree_delete_lock = 0;
		} else {
			free(node);
			return 2;
		}		
#else
		ntyRBTreeInsert(self, node);
		self->count ++;
#endif
		return 0;
	}
	return 1; //exist
}

void* ntyRBTreeOperaSearch(void *_self, C_DEVID key) {
	RBTree *self = _self;
	RBTreeNode *node = ntyRBTreeSearch(self, key);

	if (node == self->nil) {
		return NULL;
	}
	return node;
}

int ntyRBTreeOperaDelete(void *_self, C_DEVID key) {
	RBTree *self = _self;

	RBTreeNode *node = ntyRBTreeSearch(self , key);
	if (node == self->nil) return 3;
#if 1 //Release Friend list
	if (node->value != NULL) {
#if 1
		if(0 == cmpxchg(&self->rbtree_delete_lock, 0, 1, WORD_WIDTH)) {
			Client *client = node->value;
			if (client->friends != NULL) {
				client->friends = ntyRBTreeOperaDtor(client->friends);
				client->friends = NULL;
			}
			
			node = ntyRBTreeDelete(self, node);
#if 1
			self->count --;
#endif
#if 0
			free(node->value);
#endif
			free(node);

			self->rbtree_delete_lock = 0;
		} else {
			ntydbg(" ntyRBTreeOperaDelete --> have delete node\n"); 
			return 2;
		}
#endif
#if 0
		free(node->value);
#endif
	}
#endif
#if 0
	free(node);
#endif
#if 0 //Update Assertion `(list-friends) == count' failed. 
	self->count --;
#endif

	return 0;
}

int ntyRBTreeOperaUpdate(void *_self, C_DEVID key, void *value) {
	RBTree *self = _self;

	RBTreeNode *node = ntyRBTreeSearch(self , key);
	if (node == self->nil) return 1;

	free(node->value);
	node->value = value;

	return 0;
}

static void ntyInOrderTraversal(RBTree *T, RBTreeNode *node, HANDLE_CLIENTID handle_FN) {
	if (node != T->nil) {
		ntyInOrderTraversal(T, node->left, handle_FN);		
		handle_FN(T, node->key);
		ntyInOrderTraversal(T, node->right, handle_FN);
	}
	return ;
}

static void ntyInOrderTraversalNotify(RBTree *T, RBTreeNode *node, C_DEVID selfId, HANDLE_NOTIFY notify_FN) {
	if (node != T->nil) {
		ntyInOrderTraversalNotify(T, node->left, selfId, notify_FN);		
		notify_FN(selfId, node->key);
		ntyInOrderTraversalNotify(T, node->right, selfId, notify_FN);
	}
	return ;
}


static void ntyInOrderTraversalForList(const RBTree *T, RBTreeNode *node, C_DEVID *list) {
	if (node != T->nil) {
		ntyInOrderTraversalForList(T, node->left, list);
		//ntylog(" %lld ", node->key);
		//handle_FN(node->key);
		*list = node->key;
		ntyInOrderTraversalForList(T, node->right, ++list);
	}
	return ;
}

static void ntyInOrderMass(RBTree *T, RBTreeNode *node, HANDLE_MASS handle_FN, U8 *buf, int length) {
	if (node != T->nil) {
		
		ntyInOrderMass(T, node->left, handle_FN, buf, length);		
		handle_FN(node->key, buf, length);
		ntyInOrderMass(T, node->right, handle_FN, buf, length);
	}
	return ;
}

static void ntyInOrderBroadcast(RBTree *T, RBTreeNode *node, HANDLE_BROADCAST handle_FN, void *client, U8 *buf, int length) {
	if (node != T->nil) {
		Client *pClient = client;
		//ntylog(" ntyInOrderBroadcast fromId \n");
		ntyInOrderBroadcast(T, node->left, handle_FN, client, buf, length);
		//ntylog(" ntyInOrderBroadcast :%lld, fromId:%lld, buffer:%s, length:%d\n", node->key, pClient->devId, buf, length);
		handle_FN(client, node->key, buf, length);
		//ntylog(" ntyInOrderBroadcast --> handle_FN :%lld, buffer:%s, length:%d\n", node->key, buf, length);
		ntyInOrderBroadcast(T, node->right, handle_FN, client, buf, length);
	}
	return ;
}

static void ntyInOrderHeartBeat(RBTree *T, RBTreeNode *node, HANDLE_HEARTBEAT handle_FN, void *mainloop, TIMESTAMP stamp) {
	if (node != T->nil) {		
		ntyInOrderHeartBeat(T, node->left, handle_FN, mainloop, stamp);
		//ntylog(" ntyInOrderHeartBeat :%lld, stamp:%ld\n", node->key, stamp);
#if 0		
		handle_FN(node->value, mainloop, stamp);
#else
		handle_FN(node, mainloop, stamp);
#endif
		ntyInOrderHeartBeat(T, node->right, handle_FN, mainloop, stamp);
	}
	return ;
}



static void ntyPreOrderTraversal(RBTree *T, RBTreeNode *node) {
	if (node != T->nil) {
		//ntylog(" %lld ", node->key);
		ntyPreOrderTraversal(T, node->left);
		ntyPreOrderTraversal(T, node->right);
	}
	return ;
}

static void* ntyPreOrderTraversalForList(const RBTree *T, RBTreeNode *node, C_DEVID *list) {
	if (node != T->nil) {
		*(list++) = node->key;

		list = ntyPreOrderTraversalForList(T, node->left, list);
		list = ntyPreOrderTraversalForList(T, node->right, list);
	}
	return list;
}

static void ntyPosOrderTraversal(RBTree *T, RBTreeNode *node) {
	if (node != T->nil) {
		ntyPosOrderTraversal(T, node->left);
		ntyPosOrderTraversal(T, node->right);
		//ntylog(" %lld ", node->key);
	}
	return ;
}

static int ntyPrintTreeByLevel(RBTree *T, RBTreeNode *node, int level) {
	if (node == T->nil || level < 0) {
		return 0;
	}
	if (0 == level) {
		//ntylog(" %lld color:%d  %s\n", node->key, node->color, (char*)node->value);
		return 1;
	}

	return ntyPrintTreeByLevel(T, node->left, level-1) + ntyPrintTreeByLevel(T, node->right, level-1);
}
	
static void ntyLevelOrderTraversal(RBTree *T, RBTreeNode *node) {
	int i = 0;

	for (i = 0;;i ++) {
		if (!ntyPrintTreeByLevel(T, node, i)) break;
	}
}


void ntyRBTreeOperaTraversal(void *_self, HANDLE_CLIENTID handle_FN) {
	RBTree *self = _self;

	return ntyInOrderTraversal(self, self->root, handle_FN);
}


void ntyRBTreeOperaNotify(void *_self, C_DEVID selfId, HANDLE_NOTIFY notify_FN) {
	RBTree *self = _self;

	return ntyInOrderTraversalNotify(self, self->root, selfId, notify_FN);
}

void ntyRBTreeOperaMass(void *_self, HANDLE_MASS handle_FN, U8 *buf, int length) {
	RBTree *self = _self;
	
	return ntyInOrderMass(self, self->root, handle_FN, buf, length);
}

void ntyRBTreeOperaHeartBeat(void *_self, HANDLE_HEARTBEAT handle_FN, void *mainloop, TIMESTAMP stamp) {
	RBTree *self = _self;

	//ntylog("ntyRBTreeOperaHeartBeat\n");
	return ntyInOrderHeartBeat(self, self->root, handle_FN, mainloop, stamp);
}


void ntyRBTreeOperaBroadcast(void *_self, HANDLE_BROADCAST handle_FN, void *client,  U8 *buf, int length) {
	RBTree *self = _self;

	//ntylog("ntyRBTreeOperaBroadcast\n");
	return ntyInOrderBroadcast(self, self->root, handle_FN, client, buf, length);
}


static const RBTreeOpera ntyRBTreeOpera = {
	sizeof(RBTree),
	ntyRBTreeOperaCtor,
	ntyRBTreeOperaDtor,
	ntyRBTreeOperaInsert,
	ntyRBTreeOperaSearch,
	ntyRBTreeOperaDelete,
	ntyRBTreeOperaUpdate,
	ntyRBTreeOperaTraversal,
	ntyRBTreeOperaNotify,
	ntyRBTreeOperaMass,
	ntyRBTreeOperaBroadcast,
	ntyRBTreeOperaHeartBeat,
};

const void *pNtyRBTreeOpera = &ntyRBTreeOpera;



static void *pRBTree = NULL; //Singleton

void* ntyRBTreeInstance(void) {
	if (pRBTree == NULL) {
		int arg = 1;
		void *pTree = New(pNtyRBTreeOpera, arg);
		if ((unsigned long)NULL != cmpxchg((void*)(&pRBTree), (unsigned long)NULL, (unsigned long)pTree, WORD_WIDTH)) {
			Delete(pTree);
		} 
	}
	return pRBTree;
}

int ntyRBTreeInterfaceInsert(void *self, C_DEVID key, void *value) {
	RBTreeOpera **pRBTreeOpera = self;

	if (self && (*pRBTreeOpera) && (*pRBTreeOpera)->insert) {
		return (*pRBTreeOpera)->insert(self, key, value);
	}
	return -1;
}

//return node->value : UdpClient
void* ntyRBTreeInterfaceSearch(void *self, C_DEVID key) {
	RBTreeOpera **pRBTreeOpera = self;

	if (self && (*pRBTreeOpera) && (*pRBTreeOpera)->search) {
		RBTreeNode* node = (*pRBTreeOpera)->search(self, key);
		if (node != NULL) {
			return node->value; //UdpClient
 		} else {
			return NULL;
		}
	}
	return NULL;
}

int ntyRBTreeInterfaceDelete(void *self, C_DEVID key) {
	RBTreeOpera **pRBTreeOpera = self;

	if (self && (*pRBTreeOpera) && (*pRBTreeOpera)->delete) {
		return (*pRBTreeOpera)->delete(self, key);
	}
	return -1;
}

int ntyRBTreeInterfaceUpdate(void *self, C_DEVID key, void *value) {
	RBTreeOpera **pRBTreeOpera = self;

	if (self && (*pRBTreeOpera) && (*pRBTreeOpera)->update) {
		return (*pRBTreeOpera)->update(self, key, value);
	}
	return -1;
}

void ntyRBTreeRelease(void *self) {
	return Delete(self);
}


/*
 * use for client friends tree
 * 
 */

void *ntyFriendsTreeInstance(void) {
	int arg = 0;

	void *pFriends = New(pNtyRBTreeOpera, arg);
	return pFriends;
}


int ntyFriendsTreeInsert(void *self, C_DEVID key) {
	return ntyRBTreeInterfaceInsert(self, key, NULL);
}

void* ntyFriendsTreeSearch(void *self, C_DEVID key) {
	RBTreeOpera **pRBTreeOpera = self;

	if (self && (*pRBTreeOpera) && (*pRBTreeOpera)->search) {
		return (*pRBTreeOpera)->search(self, key);
	}
	return NULL;
}

int ntyFriendsTreeDelete(void *self, C_DEVID key) {
	return ntyRBTreeInterfaceDelete(self, key);
}

void ntyFriendsTreeTraversal(void *self, HANDLE_CLIENTID handle_FN) {
	RBTreeOpera **pRBTreeOpera = self;

	if (self && (*pRBTreeOpera) && (*pRBTreeOpera)->traversal) {
		return (*pRBTreeOpera)->traversal(self, handle_FN);
	}
}

void ntyFriendsTreeTraversalNotify(void *self, C_DEVID selfId, HANDLE_NOTIFY notify_FN) {
	RBTreeOpera **pRBTreeOpera = self;
	
	if (self && (*pRBTreeOpera) && (*pRBTreeOpera)->traversal) {
		return (*pRBTreeOpera)->notify(self, selfId, notify_FN);
	}
}

void ntyFriendsTreeMass(void *self, HANDLE_MASS handle_FN, U8 *buf, int length) {
	RBTreeOpera **pRBTreeOpera = self;
	
	if (self && (*pRBTreeOpera) && (*pRBTreeOpera)->mass) {
		return (*pRBTreeOpera)->mass(self, handle_FN, buf, length);
	}
}

void ntyFriendsTreeBroadcast(void *self, HANDLE_BROADCAST handle_FN, void *client, U8 *buf, int length) {
	RBTreeOpera **pRBTreeOpera = self;
	
	if (self && (*pRBTreeOpera) && (*pRBTreeOpera)->broadcast) {
		return (*pRBTreeOpera)->broadcast(self, handle_FN, client, buf, length);
	}
}

void ntyRBTreeHeartBeatDetect(void *self, HANDLE_HEARTBEAT handle_FN, void *mainloop, TIMESTAMP stamp) {
	RBTreeOpera **pRBTreeOpera = self;

	if (self && (*pRBTreeOpera) && (*pRBTreeOpera)->heartbeat) {
		return (*pRBTreeOpera)->heartbeat(self, handle_FN, mainloop, stamp);
	}
}




/*
 * 1 : Exist
 * 0 : no Exist
 */
int ntyFriendsTreeIsExist(void *self, C_DEVID key) {
	if (NULL == ntyFriendsTreeSearch(self, key)) {
		return 1;
	}
	return 0;
}

int ntyFriendsTreeIsEmpty(const void *self) {
	const RBTree *tree = self;
	if (self == NULL) return 0;
	if (tree->root == tree->nil) {
		return 1;
	}
	return 0;
}


C_DEVID* ntyFriendsTreeGetAllNodeList(const void *self) {
	const RBTree *tree = self;
	int count = tree->count;

	if (count == 0) return NULL;

	//C_DEVID *list = (C_DEVID*)malloc(count*sizeof(C_DEVID));
	C_DEVID *list = (C_DEVID*)malloc(count*sizeof(C_DEVID));
	C_DEVID *friends = list;
	//assert(list);
	if (list == NULL) { 
		ntydbg(" ntyFriendsTreeGetAllNodeList --> Malloc Error\n");
		return NULL;
	}
	memset(list, 0, count*sizeof(C_DEVID));
	
	list = ntyPreOrderTraversalForList(tree, tree->root, list);
#if 0
	ntydbg(" %d, count:%d\n", list-friends, count);
#else
	int c = list-friends;
	ntydbg(" %d, count:%d\n", c, count);
#endif
	assert((list-friends) == count);
	
	return friends;
}


U16 ntyFriendsTreeGetNodeCount(const void *self) {
	const RBTree *tree = self;
	return tree->count;
}

void ntyFriendsTreeRelease(void *self) {
	return Delete(self);
}

void* ntyFriendsTreeGetFristNode(void *self) {
	const RBTree *tree = self;
	return tree->root->value;
}

C_DEVID ntyFriendsTreeGetFristNodeKey(void *self) {
	const RBTree *tree = self;
	return tree->root->key;
}


//int nty

static void *pMap = NULL;
void* ntyMapInstance(void) {
	if (pMap == NULL) {
		int arg = 1;
		void *pTree = New(pNtyRBTreeOpera, arg);
		if ((unsigned long)NULL != cmpxchg((void*)(&pMap), (unsigned long)NULL, (unsigned long)pTree, WORD_WIDTH)) {
			Delete(pTree);
		} 
	}
	return pMap;
}

int ntyMapInsert(void *self, C_DEVID key, void *value) {
	RBTreeOpera **pRBTreeOpera = self;

	if (self && (*pRBTreeOpera) && (*pRBTreeOpera)->insert) {
		return (*pRBTreeOpera)->insert(self, key, value);
	}
	return -1;
}

//return node->value : UdpClient
void* ntyMapSearch(void *self, C_DEVID key) {
	RBTreeOpera **pRBTreeOpera = self;

	if (self && (*pRBTreeOpera) && (*pRBTreeOpera)->search) {
		RBTreeNode* node = (*pRBTreeOpera)->search(self, key);
		if (node != NULL) {
			return node->value; //UdpClient
 		} else {
			return NULL;
		}
	}
	return NULL;
}

int ntyMapDelete(void *self, C_DEVID key) {
	RBTreeOpera **pRBTreeOpera = self;

	if (self && (*pRBTreeOpera) && (*pRBTreeOpera)->delete) {
		return (*pRBTreeOpera)->delete(self, key);
	}
	return -1;
}

int ntyMapUpdate(void *self, C_DEVID key, void *value) {
	RBTreeOpera **pRBTreeOpera = self;

	if (self && (*pRBTreeOpera) && (*pRBTreeOpera)->update) {
		return (*pRBTreeOpera)->update(self, key, value);
	}
	return -1;
}


void ntyMapRelease(void *self) {
	return Delete(self);
}


#if 0
int main() {
	int i, count = 20, key;
	int keyArray[20] = {12, 1, 9, 2, 0, 11, 7, 19, 4, 15, 18, 5, 14, 13, 10, 16, 6, 3, 8, 17};

#if 0
	RBTreeNode *node;
	RBTree *T = (RBTree*)malloc(sizeof(RBTree));
	T->nil = (RBTreeNode*)malloc(sizeof(RBTreeNode));
	T->nil->color = BLACK;
	T->root = T->nil;
	node = T->nil;

	srand(time(NULL));  
	for (i = 0;i < count;i ++) {
		//key = ;
		//keyArray[i] = key;
		
		node = (RBTreeNode*)malloc(sizeof(RBTreeNode));
		node->key = keyArray[i];
		node->value = NULL;
				
		ntyRBTreeInsert(T, node);
	}

	ntylog("\n");

	for (i = 0;i < count;i ++) {
		node = ntyRBTreeSearch(T, keyArray[i]);

		node = ntyRBTreeDelete(T, node);
		free(node);

	}
#endif
	char *value = NULL;
	void* pRBTree = ntyRBTreeInstance();

	for (i = 0;i < count;i ++){
		value = (char*)malloc(16);
		strcpy(value, "123456");
		
		ntyRBTreeInterfaceInsert(pRBTree, keyArray[i], value);
	}
	//ntyLevelOrderTraversal(pRBTree, ((RBTree*)pRBTree)->root);
#if 1
	int size = ntFriendsTreeGetNodeCount(pRBTree);
	C_DEVID *list = ntyFriendsTreeGetAllNodeList(pRBTree);
	for (i = 0;i < size;i ++) {
		ntylog("  %lld, ", (C_DEVID)(*(list+i)));
		
	}
	free(list);
	ntylog(" addr: %x \n", list);
#endif		

	void* v = ntyRBTreeInterfaceSearch(pRBTree, 19);
	ntylog("\n value: %s\n", (char*)v);


	value = (char*)malloc(16);
	strcpy(value, "nihaoma?");
	ntylog(" Update: %d\n", ntyRBTreeInterfaceUpdate(pRBTree, 11, value));

	v = ntyRBTreeInterfaceSearch(pRBTree, 11);
	ntylog("\n value: %s\n", (char*)v);

	ntyRBTreeInterfaceDelete(pRBTree, 18);

	
	

	ntyRBTreeRelease(pRBTree);
}

#endif

