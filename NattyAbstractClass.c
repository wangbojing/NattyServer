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
#include <stdio.h>

#include "NattyAbstractClass.h"

void *New(const void *_class, ...) {
	const AbstractClass *class = _class;
	void *p = calloc(1, class->size);
	memset(p, 0, class->size);
	
	assert(p);
	*(const AbstractClass**)p = class;
	
	if (class->ctor) {
		va_list params;
		va_start(params, _class);
		p = class->ctor(p, &params);
		va_end(params);
	}
	return p;
}


void Delete(void *_class) {
	const AbstractClass **class = _class;

	if (_class && (*class) && (*class)->dtor) {
		_class = (*class)->dtor(_class);
	}
	
	free(_class);
}

