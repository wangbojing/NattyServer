/*
 *  Author : WangBoJing , email : 1989wangbojing@163.com
 * 
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of NALEX Inc. (C) 2016
 * 
 *
 ****		*****
   ***	  *
   ***        * 					    *			    *
   * ** 	  * 					    *			    *
   * ** 	  * 					    *			    *
   *  **	  * 					   **			   **
   *  **	  * 					  ***			  ***
   *   **	  * 	    ******	          ***********	   ***********	    *****         *****
   *   **	  * 	  **	    **		   **			   **			**		 **
   *	  **	  *    **		**		   **			   **			 **		 *
   *	  **	  *    **		**		   **			   **			  *		*
   *	   **   *    **		**		   **			   **			   ** 	*
   *	   **   * 		     ***		   **			   **			    *       *
   *	    **  * 	      ***** **		   **			   **			    **     *
   *	    **  * 	  *** 	**		   **			   **			    **     *
   *	     ** *    **		**		   **			   **			     *    *
   *	     ** *   **		**		   **			   **			     **  *
   *		***   **		**		   **			   **			       **
   *		***   **		**		   **	    * 	   **	    * 		 **
   *		 **   **		**	*	   **	    * 	   **	    * 		 **
   *		 **    **	  ****	*	    **   *		    **   *			 *
 *****		  *******	 ***		     ****		     ****			 *
														 *
														 *
													   *****
 *													   ****
 *
 *
 */






#ifndef __NATTY_RBTREE_H__
#define __NATTY_RBTREE_H__

#include "NattyAbstractClass.h"

#define RED		1
#define BLACK		2


typedef struct _RBTreeNode {
	int key;
	void *value;
	struct _RBTreeNode *right;
	struct _RBTreeNode *left;
	struct _RBTreeNode *parent;
	unsigned char color;
} RBTreeNode;

typedef struct _RBTree {
	RBTreeNode *root;
	RBTreeNode *nil;
} RBTree;

typedef struct _RBTreeOpera {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	int (*insert)(void *_self, int key, void *value);
	void* (*search)(void *_self, int key);
	int (*delete)(void *_self, int key);
	int (*update)(void *_self, int key, void *value);
} RBTreeOpera;



#endif



