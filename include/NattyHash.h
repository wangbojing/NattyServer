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
 * The following hash function is based on MurmurHash3, placed into the public
 * domain by Austin Appleby.  See http://code.google.com/p/smhasher/ for
 * details.
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


#ifndef __NATTY_HASH_H__
#define __NATTY_HASH_H__


#include <sys/types.h>
#include <sys/socket.h>


#include "NattyRBTree.h"

#define	ZU(z)	((size_t)z)
#define	ZI(z)	((ssize_t)z)
#define	QU(q)	((U64)q)
#define	QI(q)	((S64)q)

#define	KZU(z)	ZU(z##ULL)
#define	KZI(z)	ZI(z##LL)
#define	KQU(q)	QU(q##ULL)
#define	KQI(q)	QI(q##LL)
 

#define NATTY_DICTIONARY_LENGTH			1<<8
#define NATTY_DICTIONARY_MASK			(NATTY_DICTIONARY_LENGTH-1)
#define NATTY_POLY						0x01101


typedef RBTree Tree;

typedef struct _Payload {
	unsigned int srcip;
	unsigned int dstip;
	unsigned short sport;
	unsigned short dport;
	unsigned short proto;
} Payload;

typedef struct _HashNode {
	Payload *info;
	C_DEVID devid;
	struct _HashNode *list;
} HashNode;
/*
 * if ip and port 
 * port 
 */
typedef struct _HashTable {
	const void *_;
	HashNode *Dictionary;
} HashTable;

typedef struct _HashTableHandle {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	int (*insert)(void *_self, U32 key, Payload* load, C_DEVID id);
	C_DEVID (*search)(void *_self, U32 key, Payload* load);
	int (*delete)(void *_self, U32 key, Payload* load);
	int (*update)(void *_self, U32 key, Payload* load, C_DEVID id);
} HashTableHandle;


U32 ntyHashKey(Payload *load);


void *ntyHashTableInstance(void);
void *ntyHashTableRelease(void);
int ntyHashTableInsert(void *self, U32 key, Payload* load, C_DEVID id);
C_DEVID ntyHashTableSearch(void *self, U32 key, Payload* load) ;
int ntyHashTableDelete(void *self, U32 key, Payload* load);
int ntyHashTableUpdate(void *self, U32 key, Payload* load, C_DEVID id);


C_DEVID ntySearchDevIdFromHashTable(struct sockaddr_in *addr);



#endif



