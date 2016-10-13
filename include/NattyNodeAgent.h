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

#ifndef __NATTY_NODE_AGENT_H__
#define __NATTY_NODE_AGENT_H__

#include "NattyConfig.h"
#include "NattyAbstractClass.h"
#include "NattyFilter.h"



typedef struct NTYNODE {
	const char *name;
	int length;
} ntyNode;

typedef struct {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	void (*setSuccessor)(void *_self, void *succ);
	void* (*getSuccessor)(const void *_self);
	void (*handleRequest)(const void *_self, U8 **table, int count, C_DEVID id);
} NodeFilter;

typedef Packet	NodePacket;

#define NTY_SEPARATOR_COMMAS		' '
#define NTY_SEPARATOR_COLON			':'



#if (CUSTOM_SELECT == CUSTOM_JG)

extern const ntyNode ntyNodeTable[];


enum ntyNode_Database {
	DB_NODE_START = 0,
	DB_NODE_POWER = DB_NODE_START,
	DB_NODE_SIGNAL,
	DB_NODE_PHONEBOOK,
	DB_NODE_FAMILYNUMBER,
	DB_NODE_FALLEN,
	DB_NODE_GPS,
	DB_NODE_WIFI,
	DB_NODE_LAB,
	DB_NODE_STEP,
	DB_NODE_HEARTRATE,
	DB_NODE_LOCATION,
	DB_NODE_STATUS,
	DB_NODE_CONFIG,
	DB_NODE_END = DB_NODE_CONFIG,
};

#else

#error "Please config custom."

#endif


int ntyNodeCompare(const U8 *taken);
void ntyNodeAgentProcess(const U8 *buffer, int length, C_DEVID id);


#endif


