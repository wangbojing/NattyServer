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





#ifndef __NATTY_ABSTRACT_CLASS_H__
#define __NATTY_ABSTRACT_CLASS_H__

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>


typedef struct {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
} AbstractClass;


void *New(const void *_class, ...);
void Delete(void *_class);


#endif


