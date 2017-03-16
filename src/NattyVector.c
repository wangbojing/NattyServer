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



#include "NattyVector.h"
#include "NattyResult.h"


void* ntyVectorCtor(void *self, va_list *params) {
	NVector *vector = self;

	memset(vector, 0, sizeof(NVector));
	LIST_INIT(&vector->header);
	
	vector->num = 0;
	vector->max_num = NTY_VECTOR_MAX_COUNT;

	return vector;
}

void* ntyVectorDtor(void *self) {
	NVector *vector = self;

	while(!LIST_EMPTY(&vector->header)) {
		NKnot *knot = LIST_FIRST(&vector->header);
		LIST_REMOVE(knot, entries);

		free(knot->data);
		free(knot);
	}

	vector->num = 0;
	vector->max_num = 0;
	//vector->header = NULL;

	return vector;
}


void* ntyVectorAdd(void *self, void *data, int len) {
	NVector *vector = self;
	NKnot *knot = NULL;

	if(vector->num < vector->max_num) {
		vector->num++;
	} else {
		return NULL;
	} 

	knot = malloc(sizeof(NKnot));
	if (knot == NULL) return NULL;

	if (data != NULL) {
		knot->len = 0;
		knot->data = malloc(len);
		memcpy(knot->data, data, len);
	} else {
		knot->data = NULL;
		knot->len = 0;
	}

	LIST_INSERT_HEAD(&vector->header, knot, entries);

	return knot;
}


int ntyVectorDel(void *self, void *data) {
	NVector *vector = self;
	NKnot *knot = NULL;
	
	for (knot = vector->header.lh_first ;knot != NULL;knot = knot->entries.le_next) {
		if (0 == memcmp(knot->data, data, knot->len)) {
			LIST_REMOVE(knot, entries);
			vector->num --;

			if (knot->data != NULL) {
				free(knot->data);
			}
			knot->len = 0;
			free(knot);

			return NTY_RESULT_SUCCESS;
		}
	}
	return NTY_RESULT_NOEXIST;
}

void ntyVectorIter(void *self, NVECTOR_CB *cb, void *arg) {
	NVector *vector = self;
	NKnot *knot = NULL;

	for (knot = vector->header.lh_first;knot != NULL;knot = knot->entries.le_next) {
		cb(knot->data, arg);
	}

	return ;
}

void *ntyVectorGetNodeList(void *self, int *num) {
	NVector *vector = self;
	int count = vector->num, i = 0;
	NKnot *knot = LIST_FIRST(&vector->header);

	*num = count;
	ntylog(" ntyVectorGetNodeList : %d\n", count);
	if (count == 0) return NULL;
	char *list = malloc(count * (knot->len));
	if (list == NULL) ASSERT(0);
	for (knot = vector->header.lh_first;(knot != NULL) && (i < count);knot = knot->entries.le_next) {
		memcpy(list+(i*knot->len), knot->data, knot->len);
		i ++;
	}

	return list;
}



static const NVectorHandle ntyVectorHandle  = {
	sizeof(NVector),
	ntyVectorCtor,
	ntyVectorDtor,
	ntyVectorAdd,
	ntyVectorDel,
	ntyVectorIter,
};

const void *pNtyVectorHandle = &ntyVectorHandle;

void* ntyVectorCreator(void) {
	return New(pNtyVectorHandle);
}

void* ntyVectorDestory(void *self) {
	Delete(self);
	return NULL;
}

void* ntyVectorInsert(void *self, void *data, int len) {
	NVectorHandle * const * handle = self;

	if (self && (*handle) && (*handle)->add) {
		return (*handle)->add(self, data, len);
	}

	return NULL;
}

int ntyVectorDelete(void *self, void *data) {
	NVectorHandle * const * handle = self;

	if (self && (*handle) && (*handle)->del) {
		return (*handle)->del(self, data);
	}

	return NTY_RESULT_FAILED;
}


void ntyVectorIterator(void *self, NVECTOR_CB *cb, void *arg) {
	NVectorHandle * const * handle = self;

	if (self && (*handle) && (*handle)->iter) {
		return (*handle)->iter(self, cb, arg);
	}

	return ;
}





