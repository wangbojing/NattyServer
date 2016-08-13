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
#ifndef __NATTY_LETTER_H__
#define __NATTY_LETTER_H__

#define DATA_ARRAY_LENGTH		64
#define LATTER_ARRAY_LENGTH		62

extern unsigned char Data_0[DATA_ARRAY_LENGTH];
extern unsigned char Data_1[DATA_ARRAY_LENGTH];
extern unsigned char Data_2[DATA_ARRAY_LENGTH];
extern unsigned char Data_3[DATA_ARRAY_LENGTH];
extern unsigned char Data_4[DATA_ARRAY_LENGTH];

extern unsigned char Data_5[DATA_ARRAY_LENGTH];
extern unsigned char Data_6[DATA_ARRAY_LENGTH];
extern unsigned char Data_7[DATA_ARRAY_LENGTH];
extern unsigned char Data_8[DATA_ARRAY_LENGTH];
extern unsigned char Data_9[DATA_ARRAY_LENGTH];



extern unsigned char Letter_a[DATA_ARRAY_LENGTH];
extern unsigned char Letter_b[DATA_ARRAY_LENGTH];
extern unsigned char Letter_c[DATA_ARRAY_LENGTH];
extern unsigned char Letter_d[DATA_ARRAY_LENGTH];
extern unsigned char Letter_e[DATA_ARRAY_LENGTH];

extern unsigned char Letter_f[DATA_ARRAY_LENGTH];
extern unsigned char Letter_g[DATA_ARRAY_LENGTH];
extern unsigned char Letter_h[DATA_ARRAY_LENGTH];
extern unsigned char Letter_i[DATA_ARRAY_LENGTH];
extern unsigned char Letter_j[DATA_ARRAY_LENGTH];

extern unsigned char Letter_k[DATA_ARRAY_LENGTH];
extern unsigned char Letter_l[DATA_ARRAY_LENGTH];
extern unsigned char Letter_m[DATA_ARRAY_LENGTH];
extern unsigned char Letter_n[DATA_ARRAY_LENGTH];
extern unsigned char Letter_o[DATA_ARRAY_LENGTH];

extern unsigned char Letter_p[DATA_ARRAY_LENGTH];
extern unsigned char Letter_q[DATA_ARRAY_LENGTH];
extern unsigned char Letter_r[DATA_ARRAY_LENGTH];
extern unsigned char Letter_s[DATA_ARRAY_LENGTH];
extern unsigned char Letter_t[DATA_ARRAY_LENGTH];

extern unsigned char Letter_u[DATA_ARRAY_LENGTH];
extern unsigned char Letter_v[DATA_ARRAY_LENGTH];
extern unsigned char Letter_w[DATA_ARRAY_LENGTH];
extern unsigned char Letter_x[DATA_ARRAY_LENGTH];
extern unsigned char Letter_y[DATA_ARRAY_LENGTH];
extern unsigned char Letter_z[DATA_ARRAY_LENGTH];


extern unsigned char Letter_A[DATA_ARRAY_LENGTH];
extern unsigned char Letter_B[DATA_ARRAY_LENGTH];
extern unsigned char Letter_C[DATA_ARRAY_LENGTH];
extern unsigned char Letter_D[DATA_ARRAY_LENGTH];
extern unsigned char Letter_E[DATA_ARRAY_LENGTH];

extern unsigned char Letter_F[DATA_ARRAY_LENGTH];
extern unsigned char Letter_G[DATA_ARRAY_LENGTH];
extern unsigned char Letter_H[DATA_ARRAY_LENGTH];
extern unsigned char Letter_I[DATA_ARRAY_LENGTH];
extern unsigned char Letter_J[DATA_ARRAY_LENGTH];

extern unsigned char Letter_K[DATA_ARRAY_LENGTH];
extern unsigned char Letter_L[DATA_ARRAY_LENGTH];
extern unsigned char Letter_M[DATA_ARRAY_LENGTH];
extern unsigned char Letter_N[DATA_ARRAY_LENGTH];
extern unsigned char Letter_O[DATA_ARRAY_LENGTH];

extern unsigned char Letter_P[DATA_ARRAY_LENGTH];
extern unsigned char Letter_Q[DATA_ARRAY_LENGTH];
extern unsigned char Letter_R[DATA_ARRAY_LENGTH];
extern unsigned char Letter_S[DATA_ARRAY_LENGTH];
extern unsigned char Letter_T[DATA_ARRAY_LENGTH];

extern unsigned char Letter_U[DATA_ARRAY_LENGTH];
extern unsigned char Letter_V[DATA_ARRAY_LENGTH];
extern unsigned char Letter_W[DATA_ARRAY_LENGTH];
extern unsigned char Letter_X[DATA_ARRAY_LENGTH];
extern unsigned char Letter_Y[DATA_ARRAY_LENGTH];
extern unsigned char Letter_Z[DATA_ARRAY_LENGTH];


extern unsigned char *Font[];


enum Font_Database {
	DB_DATA_START = 0,
	DB_DATA_0 = DB_DATA_START,
	DB_DATA_1,
	DB_DATA_2,
	DB_DATA_3,
	DB_DATA_4,
	DB_DATA_5,
	DB_DATA_6,
	DB_DATA_7,
	DB_DATA_8,
	DB_DATA_9,
	DB_DATA_END = DB_DATA_9,

	DB_LETTER_SMALL_START,
	DB_LETTER_a = DB_LETTER_SMALL_START,
	DB_LETTER_b,
	DB_LETTER_c,
	DB_LETTER_d,
	DB_LETTER_e,
	DB_LETTER_f,
	DB_LETTER_g,
	DB_LETTER_h,
	DB_LETTER_i,
	DB_LETTER_j,
	DB_LETTER_k,
	DB_LETTER_l,
	DB_LETTER_m,
	DB_LETTER_n,
	DB_LETTER_o,
	DB_LETTER_p,
	DB_LETTER_q,
	DB_LETTER_r,
	DB_LETTER_s,
	DB_LETTER_t,
	DB_LETTER_u,
	DB_LETTER_v,
	DB_LETTER_w,
	DB_LETTER_x,
	DB_LETTER_y,
	DB_LETTER_z,
	DB_LETTER_SMALL_END = DB_LETTER_z,

	DB_LETTER_BIG_START,
	DB_LETTER_A = DB_LETTER_BIG_START,
	DB_LETTER_B,
	DB_LETTER_C,
	DB_LETTER_D,
	DB_LETTER_E,
	DB_LETTER_F,
	DB_LETTER_G,
	DB_LETTER_H,
	DB_LETTER_I,
	DB_LETTER_J,
	DB_LETTER_K,
	DB_LETTER_L,
	DB_LETTER_M,
	DB_LETTER_N,
	DB_LETTER_O,
	DB_LETTER_P,
	DB_LETTER_Q,
	DB_LETTER_R,
	DB_LETTER_S,
	DB_LETTER_T,
	DB_LETTER_U,
	DB_LETTER_V,
	DB_LETTER_W,
	DB_LETTER_X,
	DB_LETTER_Y,
	DB_LETTER_Z,
	DB_LETTER_BIG_END = DB_LETTER_Z,

};
#endif


