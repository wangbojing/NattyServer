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
	
#if 0
	LIST_INIT(&vector->header);
#elif 1
	NKnot *nknot = (NKnot*)malloc(sizeof(NKnot));
	if (nknot == NULL) return vector;

	memset(nknot, 0, sizeof(NKnot));
	nknot->data = NULL;
	nknot->len = 0;
	nknot->next = NULL;

	vector->header = nknot;
#else
	vector->header = NULL;
#endif
	vector->num = 0;
	vector->max_num = NTY_VECTOR_MAX_COUNT;
	vector->vector_lock = 0;

	ntydbg("ntyVectorCtor\n");

	return vector;
}

void* ntyVectorDtor(void *self) {
	NVector *vector = self;

#if 0
	while(!LIST_EMPTY(&vector->header)) {
		NKnot *knot = LIST_FIRST(&vector->header);
		LIST_REMOVE(knot, entries);

		free(knot->data);
		free(knot);
	}
#else
	
	if (vector == NULL) return NULL;
	NKnot *iter = vector->header;

	while (iter != NULL) {		
		NKnot *knot = iter->next;

		if (iter->data != NULL) {
			free(iter->data);
		}
		free(iter);

		iter = knot;
	}

#endif
	vector->num = 0;
	vector->max_num = 0;
	//vector->header = NULL;

	return vector;
}


void* ntyVectorAdd(void *self, void *data, int len) {
	NVector *vector = self;
	NKnot *knot = NULL;
	if (vector == NULL) return NULL;

	if(vector->num < vector->max_num) {
		vector->num++;
	} else {
		return NULL;
	} 

	knot = malloc(sizeof(NKnot));
	if (knot == NULL) return NULL;
	memset(knot, 0, sizeof(NKnot));

	if (data != NULL) {
		
		knot->len = len;
		knot->next = NULL;
		knot->data = malloc(len);
		
		if (knot->data == NULL) {
			free(knot);
			return NULL;
		}
		memcpy(knot->data, data, len);
		
	} else {
		return NULL;
	}

	if(0 == cmpxchg(&vector->vector_lock, 0, 1, WORD_WIDTH)) {
#if 0
		LIST_INSERT_HEAD(&vector->header, knot, entries);
#else
		
		NKnot *first = vector->header;
		if (first != NULL) {
			NKnot *next = first->next;
			first->next = knot;
			knot->next = next;
		} else {
			NKnot *next = vector->header;
			vector->header = knot;
			knot->next = next;
		}
#endif
		vector->vector_lock = 0;
	} else {
		if (knot->data != NULL) {
			free(knot->data);
		}
		free(knot);
		return NULL;
	}

	return knot;
}


int ntyVectorDel(void *self, void *data) {
	NVector *vector = self;
	NKnot *knot = NULL;

	if (vector == NULL) return NTY_RESULT_FAILED;
#if 0	
	for (knot = vector->header.lh_first ;knot != NULL;knot = knot->entries.le_next) {
		if (0 == memcmp(knot->data, data, knot->len)) {
			if (0 == cmpxchg(&vector->vector_lock, 0, 1, WORD_WIDTH)) {
				LIST_REMOVE(knot, entries);
				vector->num --;

				if (knot->data != NULL) {
					free(knot->data);
				}
				knot->len = 0;
				free(knot);

				vector->vector_lock = 0;
			} else {
				return NTY_RESULT_BUSY;
			}

			return NTY_RESULT_SUCCESS;
		}
	}
#else

	knot = vector->header;
	NKnot *iter = knot->next;

	while (iter != NULL) {
		if (0 == memcmp(iter->data, data, iter->len)) {
			if (0 == cmpxchg(&vector->vector_lock, 0, 1, WORD_WIDTH)) {
				vector->num --;

				//delete node
				knot->next = iter->next;

				if (iter->data != NULL) {
					free(iter->data);
				}
				iter->len = 0;
				free(iter);				
				iter = knot;
				
				vector->vector_lock = 0;
			}
		}
		knot = iter;
		iter = iter->next;
	}
		
#endif
	return NTY_RESULT_NOEXIST;
}

void ntyVectorIter(void *self, NVECTOR_CB *cb, void *arg) {
	NVector *vector = self;
	if (vector == NULL) return ;
	NKnot *knot = vector->header;
	NKnot *iter = NULL;
#if 0
	for (knot = vector->header.lh_first;knot != NULL;knot = knot->entries.le_next) {
		cb(knot->data, arg);
	}
#else
	for (iter = knot->next;iter != NULL;iter = iter->next) {
		cb(iter->data, arg);
	}
#endif
	return ;
}

void *ntyVectorGetNodeList(void *self, int *num) {
	NVector *vector = self;
	int count = vector->num, i = 0;
#if 0
	NKnot *knot = LIST_FIRST(&vector->header);

	*num = count;
	ntydbg(" ntyVectorGetNodeList : %d\n", count);
	if (count == 0) return NULL;
	char *list = malloc(count * (knot->len));
	if (list == NULL) ASSERT(0);
	for (knot = vector->header.lh_first;(knot != NULL) && (i < count);knot = knot->entries.le_next) {
		memcpy(list+(i*knot->len), knot->data, knot->len);
		i ++;
	}
	return list;
#else
	return NULL;
#endif
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
	NVectorHandle ** handle = self;

	if (self && (*handle) && (*handle)->add) {
		return (*handle)->add(self, data, len);
	}

	return NULL;
}

int ntyVectorDelete(void *self, void *data) {
	NVectorHandle ** handle = self;

	if (self && (*handle) && (*handle)->del) {
		return (*handle)->del(self, data);
	}

	return NTY_RESULT_FAILED;
}


void ntyVectorIterator(void *self, NVECTOR_CB *cb, void *arg) {
	NVectorHandle ** handle = self;

	if (self && (*handle) && (*handle)->iter) {
		return (*handle)->iter(self, cb, arg);
	}

	return ;
}

#if 0
int vector_cb(void *self, void *arg) { 
	C_DEVID id = *(C_DEVID*)self;

	ntylog(" vector : %lld\n", id);
}


int main() {
	NVector *vector = ntyVectorCreator();

	C_DEVID data[10] = {240207489189498949, 240207489205306960, 240207489199736837, 240207489224171833, 240207489190414451,
			240207489167087685, 240207489173648928, 240207489188468597, 240207489173882466, 240207489200890485};

	int i = 0, j = 0;


	for (j = 0;j < 10;j ++) {
		for (i = 0;i < 10;i ++) {
			ntyVectorInsert(vector, &data[i], sizeof(C_DEVID));
		}
#if 0		
		for (i = 0;i < 10;i ++) {
			ntyVectorInsert(vector, &data[i], sizeof(C_DEVID));
		}
		ntyVectorIterator(vector, vector_cb, NULL);
#endif
		for (i = 0;i < 5;i ++) {
			ntyVectorDelete(vector, &data[i]);
			ntylog("\n");
		}
		ntyVectorIterator(vector, vector_cb, NULL);
		ntylog("End :%d\n", j);
	}

	ntyVectorIterator(vector, vector_cb, NULL);

	ntylog("vector \n");
}
#endif


