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
#include <stdlib.h>
#include <time.h>

#include "NattyUtils.h"
#include "NattyProtocol.h"


void ntyTimeCheckStamp(U8 *buf) {
	time_t timer;
	struct tm *tblock;
	timer = time(NULL);
	tblock = localtime(&timer);

	buf[NTY_PROTO_TIMECHECK_YEAR_IDX] = (U8)(tblock->tm_year % 100);
	buf[NTY_PROTO_TIMECHECK_MONTH_IDX] = (U8)(tblock->tm_mon);
	buf[NTY_PROTO_TIMECHECK_DAY_IDX] = (U8)(tblock->tm_mday);
	buf[NTY_PROTO_TIMECHECK_WDAY_IDX] = (U8)(tblock->tm_wday);
	buf[NTY_PROTO_TIMECHECK_HOUR_IDX] = (U8)(tblock->tm_hour);
	buf[NTY_PROTO_TIMECHECK_MINUTE_IDX] = (U8)(tblock->tm_min);
	buf[NTY_PROTO_TIMECHECK_SECOND_IDX] = (U8)(tblock->tm_sec);
}


void ntyDisplayLetter(char *LogStr, int length) {
	int i,j,k;
	//int len = strlen(LogStr)-1;
	char u8FontCode = 0x0;

	for (i = 0;i < 32;i ++) {
#if 0
		for (j = 0;j < 8;j ++) {
			if (((*(Font[FontCode]+2*i)) >> j) & 0x01) {
				//lcd_240240_draw_point(x+j,y+i,1);
				ntylog("*");
			} else {
				ntylog(" ");
			}
		}
		for (j = 0;j < 8;j ++) {
			if (((*(Font[FontCode]+2*i+1)) >> j) & 0x01) {
				//lcd_240240_draw_point(x+8+j,y+i,1);
				ntylog("*");
			} else {
				ntylog(" ");
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
					ntydbg("*");
				} else {
					ntydbg(" ");
				}
			}
			for (j = 0;j < 8;j ++) {
				if (((*(Font[u8FontCode]+2*i+1)) >> j) & 0x01) {
					//lcd_240240_draw_point(x+8+j,y+i,1);
					ntydbg("*");
				} else {
					ntydbg(" ");
				}
			}
		}
#endif
		ntydbg("\n");
	}

	ntydbg("\n\n\n\n\n");
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

	ntydbg(" Author : WangBoJing , email : 1989wangbojing@gmail.com \n");
	ntydbg(" This software is protected by Copyright and the information contained\n");
	ntydbg(" herein is confidential. The software may not be copied and the information\n");
	ntydbg(" contained herein may not be used or disclosed except with the written\n");
	ntydbg(" permission of Author. (C) 2016\n\n\n");
	//ntydbg("--------------------------------------------------------------------\n");
#endif
}

void ntyFree(void *p) {
	if (p != NULL) {
		free(p);
	}
}

void *ntyMalloc(size_t size) {
	return malloc(size);
}


#if 1
void ntyBuildNext(const char *pattern, U32 length, U32 *next) {
	U32 i = 1 ,t = 0;

	next[1] = 0;
	while (i < length + 1) {
		while (t > 0 && pattern[i-1] != pattern[t-1]) {
			t = next[t];
		}

		++ t;
		++ i;
		if (pattern[i-1] == pattern[t-1]) {
			next[i] = next[t];
		} else {
			next[i] = t;
		}
	}

	while (t > 0 && pattern[i-1] != pattern[t-1]) {
		t = next[t];
	}

	++ t;
	++ i;

	next[i] = t;
}

#define PATTERN_LENGTH_COUNT 32

U32 ntyKMP(const char *text,const U32 text_length, const char *pattern,const U32 pattern_length, U32 *matches) {
	U32 i,j,n;
	U32 next[PATTERN_LENGTH_COUNT];

	ntyBuildNext(pattern, pattern_length, next);

	i = 0;
	j = 1;
	n = 0;
	
	while (pattern_length + 1 - j <= text_length - i) {
		if (text[i] == pattern[j-1]) {
			++ i;
			++ j;
			if (j == pattern_length + 1) {
				matches[n++] = i - pattern_length;
				j = next[j];
			}
		} else {
			j = next[j];
			if (j == 0) {
				++i;
				++j;
			}
		}
	}

	return n;
}

#endif



