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




#include <stdio.h>
#include <string.h>

#include "NattyUtils.h"


void ntyDisplayLetter(char *LogStr, int length) {
	int i,j,k;
	//int len = strlen(LogStr)-1;
	char u8FontCode = 0x0;

	for (i = 0;i < 32;i ++) {
#if 0
		for (j = 0;j < 8;j ++) {
			if (((*(Font[FontCode]+2*i)) >> j) & 0x01) {
				//lcd_240240_draw_point(x+j,y+i,1);
				printf("*");
			} else {
				printf(" ");
			}
		}
		for (j = 0;j < 8;j ++) {
			if (((*(Font[FontCode]+2*i+1)) >> j) & 0x01) {
				//lcd_240240_draw_point(x+8+j,y+i,1);
				printf("*");
			} else {
				printf(" ");
			}
		}
#else
		for (k = 0;k < length;k ++) {
			if (*(LogStr+k) >= 0x41 && *(LogStr+k) <= 0x5A) {
				u8FontCode = *(LogStr+k)-0x41 + DB_LETTER_BIG_START;
			} else if (*(LogStr+k) >= 0x61 && *(LogStr+k) <= 0x7A) {
				u8FontCode = *(LogStr+k)-0x61 + DB_LETTER_SMALL_START;
			} else {
				u8FontCode = DB_LETTER_SMALL_START;
			}
			for (j = 0;j < 8;j ++) {				
				if (((*(Font[u8FontCode]+2*i)) >> j) & 0x01) {
					//lcd_240240_draw_point(x+j,y+i,1);
					printf("*");
				} else {
					printf(" ");
				}
			}
			for (j = 0;j < 8;j ++) {
				if (((*(Font[u8FontCode]+2*i+1)) >> j) & 0x01) {
					//lcd_240240_draw_point(x+8+j,y+i,1);
					printf("*");
				} else {
					printf(" ");
				}
			}
		}
#endif
		printf("\n");
	}

	printf("\n\n\n\n\n");
}


void ntyDisplay(void) {
#if 0
	ntyDisplayLetter(DB_LETTER_N);
	ntyDisplayLetter(DB_LETTER_A);
	ntyDisplayLetter(DB_LETTER_T);
	ntyDisplayLetter(DB_LETTER_T);
	ntyDisplayLetter(DB_LETTER_Y);
#else
	char *LogStr = "Natty";	
	ntyDisplayLetter(LogStr, 5);

	printf(" Author : WangBoJing , email : 1989wangbojing@gmail.com \n");
	printf(" This software is protected by Copyright and the information contained\n");
	printf(" herein is confidential. The software may not be copied and the information\n");
	printf(" contained herein may not be used or disclosed except with the written\n");
	printf(" permission of Author. (C) 2016\n\n\n");

#endif
}


