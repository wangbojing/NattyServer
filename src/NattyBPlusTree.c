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



#include "NattyBPlusTree.h"
#include <stdlib.h>
#include <string.h>


NBPNode * Queue = NULL;
bool bVerboseOutput = false;

NBPNode *ntyInsertIntoNodeAfterSplitting(NBPNode *root, NBPNode *old_node, int left_index, NTY_ID key, NBPNode *right) ;
NBPNode *ntyDeleteEntry(NBPNode *root, NBPNode *node, NTY_ID key, void *pointer);
NBPNode *ntyInsertIntoParent(NBPNode *root, NBPNode *left, NTY_ID key, NBPNode *right);
NBPNode *ntyInsertIntoNewRoot(NBPNode *left, NTY_ID key, NBPNode *right);
NRecord *ntyFind(NBPNode *root, NTY_ID key, bool verbose);




void ntyEnqueue(NBPNode *new_node) {
	NBPNode *c;
	
	if (Queue == NULL) {
		Queue = new_node;
		Queue->next = NULL;
	} else {
		c = Queue;
		while(c->next != NULL) {
			c = c->next;
		}
		c->next = new_node;
		new_node->next = NULL;
	}
}

NBPNode *ntyDequeue(void) {
	NBPNode *n = Queue;
	Queue = Queue->next;
	n->next = NULL;
	return n;
}


void ntyPrintTree(NBPNode *root) {
	NBPNode *node = NULL;
	int i = 0, rank = 0, new_rank = 0;

	if (root == NULL) {
		ntylog("Empty Tree.\n");
		return ;
	}

	Queue = NULL;
	ntyEnqueue(root);

	while (Queue != NULL) {
		node = ntyDequeue();
		if (node->parent != NULL && node == node->parent->pointers[0]) {
			new_rank = ntyPathToRoot(root, node);
			if (new_rank != rank) {
				rank = new_rank;
				ntylog("\n");
			}
		}
		if (bVerboseOutput) {
			ntylog("(%lx)", (unsigned long)node);
		}

		for (i = 0;i < node->nKeys;i ++) {
			if (bVerboseOutput) {
				ntylog("(%lx)", (unsigned long)node->pointers[i]);
			}
			ntylog("%lld ", node->keys[i]);
		}

		if (!node->isLeaf) {
			for (i = 0;i <= node->nKeys;i ++) {
				ntyEnqueue(node->pointers[i]);
			}
		}

		if (bVerboseOutput) {
			if (node->isLeaf) {
				ntylog("%lx ", (unsigned long)node->pointers[NTY_BPLUSTREE_ORDER-1]);
			} else {
				ntylog("%lx ", (unsigned long)node->pointers[node->nKeys]);
			}
		}
		ntylog("| ");
	}
	ntylog("\n");
}

void ntyFindAndPrint(NBPNode *root, NTY_ID key, bool verbose) {
	NRecord *r = ntyFind(root, key, verbose);
	if (r == NULL) {
		ntylog("Record not found under key %lld.\n", key);
	} else {
		ntylog("Record at %lx -- key %lld\n", (unsigned long)r, key);
	}
}

NBPNode *ntyFindLeaf(NBPNode *root, NTY_ID key, bool verbose) {
	int i = 0;
	NBPNode *c = root;

	if (c == NULL) {
		if (verbose) {
			ntylog("Empty tree. \n");
		}
		return c;
	}

	while (!c->isLeaf) {
		if (verbose) {
			ntylog("[");
			for (i = 0;i < c->nKeys-1;i ++) {
				ntylog("%lld ", c->keys[i]);
			}
			ntylog("%lld]", c->keys[i]);
		}
		i = 0;
		while (i < c->nKeys) {
			if (key >= c->keys[i]) i ++;
			else break;
		}

		if (verbose)
			ntylog("%d ->\n", i);
		c = (NBPNode*)c->pointers[i];
	}

	if (verbose) {
		ntylog("Leaf [");
		for (i = 0;i < c->nKeys-1;i ++) 
			ntylog("%lld ", c->keys[i]);
		ntylog("%lld] ->\n", c->keys[i]);
	}
	return c;
}

NRecord *ntyFind(NBPNode *root, NTY_ID key, bool verbose) {
	int i = 0;
	NBPNode *c = ntyFindLeaf(root, key, verbose);
	if (c == NULL) return NULL;

	for (i = 0;i < c->nKeys;i ++) {
		if (c->keys[i] == key) break;
	}

	if (i == c->nKeys) return NULL;
	else return (NRecord*)c->pointers[i];
}

int ntyCut(int length) {
	if (length % 2 == 0) return length/2;
	else return length/2 + 1;
}

NRecord *ntyMakeRecord(RECORDTYPE value) {
	NRecord *new_record = (NRecord*)malloc(sizeof(NRecord));
	if (new_record == NULL) {
		perror("Record Creation.\n");
		return NULL;
	} else {

		memset(new_record, 0, sizeof(NRecord));
		new_record->value = value;
	}
	return new_record;
}

NBPNode *ntyMakeNode(void) {
	NBPNode *new_node = (NBPNode*)malloc(sizeof(NBPNode));
	if (new_node == NULL) {
		perror("Node creation.");
		return NULL;
	}
	memset(new_node, 0, sizeof(NBPNode));
	new_node->keys = malloc((NTY_BPLUSTREE_ORDER-1) * sizeof(NTY_ID));
	if (new_node->keys == NULL) {
		perror("New Node Keys Array.\n");
		return NULL;
	}
	memset(new_node->keys, 0, (NTY_BPLUSTREE_ORDER-1) * sizeof(NTY_ID));
	
	new_node->pointers = malloc(NTY_BPLUSTREE_ORDER*sizeof(void*));
	if (new_node->pointers == NULL) {
		perror("New Node Keys Array.\n");
		return NULL;
	}
	memset(new_node->pointers, 0, NTY_BPLUSTREE_ORDER*sizeof(void*));

	new_node->isLeaf = false;
	new_node->nKeys = 0;
	new_node->parent = NULL;
	new_node->next = NULL;

	return new_node;
}


NBPNode *ntyMakeLeaf(void) {
	NBPNode *leaf = ntyMakeNode();
	leaf->isLeaf = true;
	return leaf;
}

int ntyPathToRoot(NBPNode *root, NBPNode *child) {
	int length = 0;
	NBPNode *c = child;
	while (c != root) {
		c = c->parent;
		length ++;
	}
	return length;
}


int ntyGetLeftIndex(NBPNode *parent, NBPNode *left) {
	int left_index = 0;
	
	while (left_index <= parent->nKeys && parent->pointers[left_index] != left)
		left_index ++;

	return left_index;
}

NBPNode *ntyInsertIntoLeaf(NBPNode *leaf, NTY_ID key, NRecord *pointer) {
	int i, insertion_point = 0;

	while (insertion_point < leaf->nKeys && leaf->keys[insertion_point] < key)
		insertion_point ++;

	for (i = leaf->nKeys;i > insertion_point;i --) {
		leaf->keys[i] = leaf->keys[i-1];
		leaf->pointers[i] = leaf->pointers[i-1];
	}

	leaf->keys[insertion_point] = key;
	leaf->pointers[insertion_point] = pointer;
	leaf->nKeys ++;

	return leaf;
}

NBPNode *ntyInsertIntoLeafAfterSplitting(NBPNode *root, NBPNode *leaf, NTY_ID key, NRecord *pointer) {
	NBPNode *new_leaf;
	NTY_ID *temp_keys;
	void **temp_pointers;
	NTY_ID new_key;
	int insertion_index, split , i, j;

	new_leaf = ntyMakeLeaf();
	temp_keys = malloc(NTY_BPLUSTREE_ORDER * sizeof(NTY_ID));
	if (temp_keys == NULL) {
		perror("Temporary keys array.\n");
		return NULL;
	}
	memset(temp_keys, 0, NTY_BPLUSTREE_ORDER * sizeof(NTY_ID));

	temp_pointers = malloc(NTY_BPLUSTREE_ORDER * sizeof(void*));
	if (temp_pointers == NULL) {
		perror("Temporary pointers array.\n");
		return NULL;
	}
	memset(temp_pointers, 0, NTY_BPLUSTREE_ORDER * sizeof(void*));

	insertion_index = 0;
	while (insertion_index < (NTY_BPLUSTREE_ORDER-1) && leaf->keys[insertion_index] < key)
		insertion_index ++;

	for (i = 0,j = 0;i < leaf->nKeys;i ++, j++) {
		if (j == insertion_index) j ++;
		temp_keys[j] = leaf->keys[i];
		temp_pointers[j] = leaf->pointers[i];
	}

	temp_keys[insertion_index] = key;
	temp_pointers[insertion_index] = pointer;

	leaf->nKeys = 0;
	split = ntyCut(NTY_BPLUSTREE_ORDER-1);

	for (i = 0;i < split;i ++) {
		leaf->pointers[i] = temp_pointers[i];
		leaf->keys[i] = temp_keys[i];
		leaf->nKeys ++;
	}

	for (i = split, j = 0;i < NTY_BPLUSTREE_ORDER;i ++,j ++) {
		new_leaf->pointers[j] = temp_pointers[i];
		new_leaf->keys[j] = temp_keys[i];
		new_leaf->nKeys ++;
	}

	free(temp_pointers);
	free(temp_keys);

	new_leaf->pointers[NTY_BPLUSTREE_ORDER-1] = leaf->pointers[NTY_BPLUSTREE_ORDER-1];
	leaf->pointers[NTY_BPLUSTREE_ORDER-1] = new_leaf;

	for (i = leaf->nKeys;i < NTY_BPLUSTREE_ORDER-1;i ++) {
		leaf->pointers[i] = NULL;
	}
	for (i = new_leaf->nKeys;i < NTY_BPLUSTREE_ORDER-1;i ++) {
		new_leaf->pointers[i] = NULL;
	}

	new_leaf->parent = leaf->parent;
	new_key = new_leaf->keys[0];

	return ntyInsertIntoParent(root, leaf, new_key,  new_leaf);
}



NBPNode *ntyInsertIntoNode(NBPNode *root, NBPNode *n, int left_index, NTY_ID key, NBPNode *right) {
	int i;

	for (i = n->nKeys;i > left_index;i --) {
		n->pointers[i+1] = n->pointers[i];
#if 0
		n->keys[i] = n->keys[i-1];
#else
		memcpy(&n->keys[i], &n->keys[i-1], sizeof(NTY_ID));
#endif
	}
	n->pointers[left_index+1] = right;
	n->keys[left_index] = key;
	n->nKeys++;

	return root;
}


NBPNode *ntyInsertIntoNodeAfterSplitting(NBPNode *root, NBPNode *old_node, int left_index, NTY_ID key, NBPNode *right) {
	int i, j, split, k_prime;
	NBPNode *new_node, *child;
	NTY_ID *temp_keys;
	NBPNode **temp_pointers;

	temp_pointers = (NBPNode **)malloc((NTY_BPLUSTREE_ORDER+1) * sizeof(NBPNode *));
	if (temp_pointers == NULL) {
		perror("Temporary pointers array for splitting nodes.");
		return NULL;
	}
	memset(temp_pointers, 0, (NTY_BPLUSTREE_ORDER+1) * sizeof(NBPNode *));

	temp_keys = malloc(NTY_BPLUSTREE_ORDER * sizeof(NTY_ID));
	if (temp_keys == NULL) {
		perror("Temporary pointers array for splitting nodes.");
		return NULL;
	}
	memset(temp_keys, 0, NTY_BPLUSTREE_ORDER * sizeof(NTY_ID));

	for (i = 0,j = 0;i < old_node->nKeys+1;i ++, j++) {
		if (j == left_index + 1) j ++;
		temp_pointers[j] = old_node->pointers[i];
	}

	for (i = 0,j = 0;i < old_node->nKeys;i ++, j ++) {
		if (j == left_index) j ++;
		temp_keys[j] = old_node->keys[i];
	}

	temp_pointers[left_index+1] = right;
	temp_keys[left_index] = key;

	split = ntyCut(NTY_BPLUSTREE_ORDER);
	new_node = ntyMakeNode();
	old_node->nKeys = 0;

	for (i = 0;i < split-1;i ++) {
		old_node->pointers[i] = temp_pointers[i];
		old_node->keys[i] = temp_keys[i];
		old_node->nKeys ++;
	}

	old_node->pointers[i] = temp_pointers[i];
	k_prime = temp_keys[split-1];

	for (++i, j = 0;i < NTY_BPLUSTREE_ORDER;i ++, j ++) {
		new_node->pointers[j] = temp_pointers[i];
		new_node->keys[j] = temp_keys[i];
		new_node->nKeys ++;
	}
	new_node->pointers[j] = temp_pointers[i];
	free(temp_pointers);
	free(temp_keys);
	new_node->parent = old_node->parent;
	for (i = 0;i <= new_node->nKeys;i ++) {
		child = new_node->pointers[i];
		child->parent = new_node;
	}

	return ntyInsertIntoParent(root, old_node, k_prime, new_node);
}


NBPNode *ntyInsertIntoParent(NBPNode *root, NBPNode *left, NTY_ID key, NBPNode *right) {
	int left_index;
	NBPNode *parent;

	parent = left->parent;
	if (parent == NULL) {
		return ntyInsertIntoNewRoot(left, key, right);
	}
	left_index = ntyGetLeftIndex(parent, left);
	if (parent->nKeys < NTY_BPLUSTREE_ORDER-1) {
		return ntyInsertIntoNode(root, parent, left_index, key, right);
	}

	return ntyInsertIntoNodeAfterSplitting(root, parent, left_index, key, right);
}


NBPNode *ntyInsertIntoNewRoot(NBPNode *left, NTY_ID key, NBPNode *right) {
	NBPNode *root = ntyMakeNode();
	root->keys[0] = key;
	root->pointers[0] = left;
	root->pointers[1] = right;
	root->nKeys ++;
	root->parent = NULL;
	left->parent = root;
	right->parent = root;

	return root;
}


NBPNode *ntyStartNewTree(NTY_ID key, NRecord *pointer) {
	NBPNode *root = ntyMakeLeaf();
	root->keys[0] = key;
	root->pointers[0] = pointer;
	root->pointers[NTY_BPLUSTREE_ORDER-1] = NULL;
	root->parent = NULL;
	root->nKeys ++;

	return root;
}

#if 0
NBPNode *ntyInsert(NBPNode *root, NTY_ID key, RECORDTYPE value) {
	NRecord *pointer;
	NBPNode *leaf;

	if (ntyFind(root, key, false) != NULL) return root;

	pointer = ntyMakeRecord(value);
	if (root == NULL)
		return ntyStartNewTree(key, pointer);

	leaf = ntyFindLeaf(root, key, false);
	if (leaf->nKeys < NTY_BPLUSTREE_ORDER-1) {
		leaf = ntyInsertIntoLeaf(leaf, key, pointer);
		return root;
	}

	return ntyInsertIntoLeafAfterSplitting(root, leaf, key, pointer);
}
#else
int ntyInsert(NBPNode **pRoot, NTY_ID key, RECORDTYPE value) {
	NRecord *pointer;
	NBPNode *leaf;
	NBPNode *root = *pRoot;

	if (ntyFind(root, key, false) != NULL) return NTY_RESULT_EXIST;

	pointer = ntyMakeRecord(value);
	if (root == NULL) {
		*pRoot = ntyStartNewTree(key, pointer);
		return NTY_RESULT_SUCCESS;
	}

	leaf = ntyFindLeaf(root, key, false);
	if (leaf->nKeys < NTY_BPLUSTREE_ORDER-1) {
		leaf = ntyInsertIntoLeaf(leaf, key, pointer);
		return NTY_RESULT_SUCCESS;
	}

	*pRoot = ntyInsertIntoLeafAfterSplitting(root, leaf, key, pointer);
	return NTY_RESULT_SUCCESS;
}

#endif

int ntyGetNeighborIndex(NBPNode *node) {
	int i;

	for (i = 0;i <= node->parent->nKeys;i ++) {
		if (node->parent->pointers[i] == node)
			return i-1;
	}
	printf("Search for nonexistent pointer to node in parent.\n");	
	printf("Node:  %#lx\n", (unsigned long)node);
}


NBPNode *ntyRemoveEntryFromNode(NBPNode *node, NTY_ID key, NBPNode *pointer) {
	int i = 0, num_pointers;

	while (node->keys[i] != key) i ++;
	for (++i;i < node->nKeys;i ++) {
		node->keys[i-1] = node->keys[i];
	}

	num_pointers = node->isLeaf ? node->nKeys : node->nKeys + 1;
	i = 0;
	while (node->pointers[i] != pointer) i ++;
	for (++i;i < num_pointers;i ++) {
		node->pointers[i-1] = node->pointers[i];
	}
	node->nKeys --;

	if (node->isLeaf) {
		for (i = node->nKeys;i < NTY_BPLUSTREE_ORDER-1;i ++) {
			node->pointers[i] = NULL;
		}
	} else {
		for (i = node->nKeys+1;i < NTY_BPLUSTREE_ORDER;i ++) {
			node->pointers[i] = NULL;
		}
	}
	return node;
}


NBPNode *ntyAdjustRoot(NBPNode *root) {
	NBPNode *new_root;

	if (root->nKeys > 0) return root;

	if (!root->isLeaf) {
		new_root = root->pointers[0];
		new_root->parent = NULL;
	} else {
		new_root = NULL;
	}

	free(root->keys);
	free(root->pointers);
	free(root);

	return new_root;
}


NBPNode *ntyCoalesceNodes(NBPNode *root, NBPNode *node, NBPNode *neighbor, int neighbor_index, NTY_ID k_prime) {
	int i, j, neighbor_insertion_index, n_end;
	NBPNode *tmp;

	if (neighbor_index == -1) {
		tmp = node;
		node = neighbor;
		neighbor = tmp;
	}

	neighbor_insertion_index = neighbor->nKeys;
	if (!node->isLeaf) {
		neighbor->keys[neighbor_insertion_index] = k_prime;
		neighbor->nKeys ++;

		n_end = node->nKeys;
		for (i = neighbor_insertion_index + 1, j = 0;j < n_end;i ++, j ++) {
			neighbor->keys[i] = node->keys[j];
			neighbor->pointers[i] = node->pointers[j];
			neighbor->nKeys ++;
			node->nKeys --;
	 	}

		neighbor->pointers[i] = node->pointers[j];
		for (i = 0;i < neighbor->nKeys+1;i ++) {
			tmp = (NBPNode *)neighbor->pointers[i];
			tmp->parent = neighbor;
		}
	} else {
		for (i = neighbor_insertion_index, j = 0;j < node->nKeys;i ++, j ++) {
			neighbor->keys[i] = node->keys[j];
			neighbor->pointers[i] = node->pointers[j];
			neighbor->nKeys++;
		}
		neighbor->pointers[NTY_BPLUSTREE_ORDER-1] = node->pointers[NTY_BPLUSTREE_ORDER-1];
	}

	root = ntyDeleteEntry(root, node->parent, k_prime, node);

	free(node->keys);
	free(node->pointers);
	free(node);

	return root;
}

NBPNode *ntyRedistributeNodes(NBPNode *root, NBPNode *node, NBPNode *neighbor, int neighbor_index, 
	int k_prime_index, NTY_ID k_prime) {
	int i;
	NBPNode *tmp;

	if (neighbor_index != -1) {
		if (!node->isLeaf) {
			node->pointers[node->nKeys+1] = node->pointers[node->nKeys];
		}

		for (i = node->nKeys;i > 0;i --) {
			node->keys[i] = node->keys[i-1];
			node->pointers[i] = node->pointers[i-1];
		}

		if (!node->isLeaf) {
			node->pointers[0] = neighbor->pointers[neighbor->nKeys];
			tmp = (NBPNode *)node->pointers[0];
			tmp->parent = node;
			neighbor->pointers[neighbor->nKeys] = NULL;
			node->keys[0] = k_prime;
			node->parent->keys[k_prime_index] = neighbor->keys[neighbor->nKeys-1];
		} else {
			node->pointers[0] = neighbor->pointers[neighbor->nKeys-1];
			neighbor->pointers[neighbor->nKeys-1] = NULL;
			node->keys[0] = neighbor->keys[neighbor->nKeys-1];
			node->parent->keys[k_prime_index] = node->keys[0];
		}
	}else {
		if (node->isLeaf) {
			node->keys[node->nKeys] = neighbor->keys[0];
			node->pointers[node->nKeys] = neighbor->pointers[0];
			node->parent->keys[k_prime_index] = neighbor->keys[1];
		} else {
			node->keys[node->nKeys] = k_prime;
			node->pointers[node->nKeys+1] = neighbor->pointers[0];
			tmp = (NBPNode *)node->pointers[node->nKeys + 1];
			tmp->parent = node;
			node->parent->keys[k_prime_index] = neighbor->keys[0];
		}

		for (i = 0;i < neighbor->nKeys-1;i ++) {
			neighbor->keys[i] = neighbor->keys[i+1];
			neighbor->pointers[i] = neighbor->pointers[i+1];
		}
		if (!node->isLeaf) {
			neighbor->pointers[i] = neighbor->pointers[i+1];
		}
	}

	node->nKeys++;
	neighbor->nKeys--;

	return root;
}



NBPNode *ntyDeleteEntry(NBPNode *root, NBPNode *node, NTY_ID key, void *pointer) {
	int min_keys;
	NBPNode *neighbor;
	int neighbor_index;
	int k_prime_index;
	NTY_ID k_prime;
	int capacity;

	node = ntyRemoveEntryFromNode(node, key, pointer);
	if (node == root) {
		return ntyAdjustRoot(root);
	}

	min_keys = node->isLeaf ? ntyCut(NTY_BPLUSTREE_ORDER-1) : ntyCut(NTY_BPLUSTREE_ORDER)-1;
	if (node->nKeys >= min_keys) return root;

	neighbor_index = ntyGetNeighborIndex(node);
	k_prime_index = (neighbor_index == -1 ? 0 : neighbor_index);
	k_prime = node->parent->keys[k_prime_index];
	neighbor = (neighbor_index == -1 ? node->parent->pointers[1] : node->parent->pointers[neighbor_index]);

	capacity = node->isLeaf ? NTY_BPLUSTREE_ORDER : NTY_BPLUSTREE_ORDER-1;
	if (neighbor->nKeys + node->nKeys < capacity) {
		return ntyCoalesceNodes(root, node, neighbor, neighbor_index, k_prime);
	} else {
		return ntyRedistributeNodes(root, node, neighbor, neighbor_index, k_prime_index, k_prime);
	}
	
}
#if 0
NBPNode *ntyDelete(NBPNode *root, NTY_ID key) {
	NBPNode *key_leaf;
	NRecord *key_record;

	key_record = ntyFind(root, key, false);
	key_leaf = ntyFindLeaf(root, key, false);

	if (key_record != NULL && key_leaf != NULL) {
		root = ntyDeleteEntry(root, key_leaf, key, key_record);
#if 1
		free(key_record->value);
#endif
		free(key_record);
	}

	return root;
}
#else
int ntyDelete(NBPNode **pRoot, NTY_ID key) {
	NBPNode *key_leaf;
	NBPNode *root = *pRoot;
	NRecord *key_record;
	

	key_record = ntyFind(root, key, false);
	key_leaf = ntyFindLeaf(root, key, false);

	if (key_record != NULL && key_leaf != NULL) {
		*pRoot = ntyDeleteEntry(root, key_leaf, key, key_record);

		free(key_record);

		return NTY_RESULT_SUCCESS;
	} 

	return NTY_RESULT_NOEXIST;
}

#endif


void ntyDestroyTreeNodes(NBPNode *root) {
	int i;

	if (root->isLeaf) {
		for (i = 0;i < root->nKeys;i ++) {
			free(root->pointers[i]);
		}
	} else {
		for (i = 0;i < root->nKeys+1;i ++) {
			ntyDestroyTreeNodes(root->pointers[i]);
		}
	}
	free(root->pointers);
	free(root->keys);
	free(root);
}

NBPNode *ntyDestroyTree(NBPNode *root) {
	ntyDestroyTreeNodes(root);
	return NULL;
}


void* ntyBHeapCtor(void *self, va_list *params) {
	NBHeap *pHeap = self;
	pHeap->root = NULL;
	pHeap->count = 0;
	pHeap->bptree_lock = 0;

	return pHeap;
}

void* ntyBHeapDtor(void *self) {
	NBHeap *pHeap = self;
	
	pHeap->root = ntyDestroyTree(pHeap->root);
	pHeap->count = 0;
	pHeap->bptree_lock = 0;
	
	return pHeap;
}

int ntyBHeapInsertHandle(void *self, NTY_ID key, RECORDTYPE value) {
	NBHeap *pHeap = self;
	int ret = NTY_RESULT_SUCCESS;

	if(0 == cmpxchg(&pHeap->bptree_lock, 0, 1, WORD_WIDTH)) {
		ret = ntyInsert(&pHeap->root, key, value);
		pHeap->bptree_lock = 0;
	} else {
		ret = NTY_RESULT_BUSY;
	}
	return ret;
}

int ntyBHeapDeleteHandle(void *self, NTY_ID key) {
	NBHeap *pHeap = self;
	int ret = NTY_RESULT_SUCCESS;
	
	if(0 == cmpxchg(&pHeap->bptree_lock, 0, 1, WORD_WIDTH)) {
		ret = ntyDelete(&pHeap->root, key);
		pHeap->bptree_lock = 0;
	} else {
		ret = NTY_RESULT_BUSY;
	}
	return ret;
}

int ntyBHeapUpdateHandle(void *self, NTY_ID key, RECORDTYPE value, size_t size) {
	NBHeap *pHeap = self;
	
	NRecord *record = ntyFind(pHeap->root, key, false);
	if (record != NULL) {
		memcpy(record->value, value, size);
		return NTY_RESULT_SUCCESS;
	}
	return NTY_RESULT_NOEXIST;
}

void* ntyBHeapSelectHandle(void *self, NTY_ID key) {
	NBHeap *pHeap = self;
	return ntyFind(pHeap->root, key, false);
}



static const NBHeapHandle ntyBHeapHandle = {
	sizeof(NBHeap),
	ntyBHeapCtor,
	ntyBHeapDtor,
	ntyBHeapInsertHandle,
	ntyBHeapDeleteHandle,
	ntyBHeapUpdateHandle,
	ntyBHeapSelectHandle
};


const void *pNtyBHeapHandle = &ntyBHeapHandle;

static void *pBHeap = NULL; //Singleton

void* ntyBHeapInstance(void) {
	if (pBHeap == NULL) {
		int arg = 1;
		void *pHeap = New(pNtyBHeapHandle, arg);
		if ((unsigned long)NULL != cmpxchg((void*)(&pBHeap), (unsigned long)NULL, (unsigned long)pHeap, WORD_WIDTH)) {
			Delete(pHeap);
		} 
	}
	return pBHeap;
}

void ntyBHeapRelease(void *self) {
	return Delete(self);
}

int ntyBHeapInsert(void *self, NTY_ID key, RECORDTYPE value) {
	NBHeapHandle **pHandle = self;
	if (self && (*pHandle) && (*pHandle)->insert) {
		return (*pHandle)->insert(self, key, value);
	}
	ntylog("ntyBHeapInsert Param Error\n");
	return NTY_RESULT_FAILED;
}

int ntyBHeapDelete(void *self, NTY_ID key) {
	NBHeapHandle **pHandle = self;
	if (self && (*pHandle) && (*pHandle)->delete) {
		return (*pHandle)->delete(self, key);
	}
	return NTY_RESULT_FAILED;
}

void* ntyBHeapSelect(void *self, NTY_ID key) {
	NBHeapHandle **pHandle = self;
	if (self && (*pHandle) && (*pHandle)->select) {
		return (*pHandle)->select(self, key);
	}
	ntylog("ntyBHeapSelect Param Error\n");
	return NULL;
}

int ntyBHeapUpdate(void *self, NTY_ID key, RECORDTYPE value, size_t size) {
	NBHeapHandle **pHandle = self;
	if (self && (*pHandle) && (*pHandle)->update) {
		return (*pHandle)->update(self, key, value, size);
	}
	return NTY_RESULT_FAILED;
}

#if 0

int main (int argc, char **argv) {
#if 0
	NBPNode *root = NULL;
	NTY_ID input;
	char instruction;

	printf("> ");
	while (scanf("%c", &instruction) != EOF) {
		switch (instruction) {
			case 'i': {
				scanf("%lld", &input);
				int *v = (int *)malloc(sizeof(void *));
				*v = 100;
				root = ntyInsert(root, input, v);
				ntyPrintTree(root);
				break;
			}
			case 'd': {
				scanf("%lld", &input);
				root = ntyDelete(root, input);
				ntyPrintTree(root);
				break;
			}
			case 'f': {
				scanf("%lld", &input);
				ntyFindAndPrint(root, input, instruction == 'p');
				break;
			}
			case 'p': {
				ntyPrintTree(root);
				break;
			}
		}
		while (getchar() != (int)'\n');		
		printf("> ");
	}
#else

	BPTreeHeap *heap = ntyBHeapInstance();
	NTY_ID input;
	char instruction;

	printf("> ");
	while (scanf("%c", &instruction) != EOF) {
		switch (instruction) {
			case 'i': {
				scanf("%lld", &input);
				int *v = (int *)malloc(sizeof(void *));
				*v = 100;
				//root = ntyInsert(root, input, v);
				//ntyPrintTree(root);
				int ret = ntyBHeapInsert(heap, (NTY_ID)input, v);
				//printf(" ret : %d\n", ret);
				ntyPrintTree(heap->root);
				break;
			}
			case 'd': {
				scanf("%lld", &input);
				//root = ntyDelete(root, input);
				ntyBHeapDelete(heap, (NTY_ID)input);
				ntyPrintTree(heap->root);
				break;
			}
			case 'f': {
				scanf("%lld", &input);
				ntyFindAndPrint(heap->root, input, instruction == 'p');
				break;
			}
			case 'p': {
				ntyPrintTree(heap->root);
				break;
			}
			case 'q': {
				exit(0);
			}
		}
		while (getchar() != (int)'\n');		
		printf("> ");
	}

#endif
}
#endif

