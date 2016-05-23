/*
 *  Author : WangBoJing , email : 1989wangbojing@gmail.com
 * 
 *  Copyright (c) 1998-2002 The OpenSSL Project.  All rights reserved.
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

#include "openssl_aes.h"
#include "openssl_modes.h"

/* The input and output encrypted as though 128bit cfb mode is being
 * used.  The extra state information to record how much of the
 * 128bit block we have used is contained in *num;
 */

void AES_cfb128_encrypt(const unsigned char *in, unsigned char *out,
	size_t length, const AES_KEY *key,
	unsigned char *ivec, int *num, const int enc) {

	CRYPTO_cfb128_encrypt(in,out,length,key,ivec,num,enc,(block128_f)AES_encrypt);
}

/* N.B. This expects the input to be packed, MS bit first */
void AES_cfb1_encrypt(const unsigned char *in, unsigned char *out,
		      size_t length, const AES_KEY *key,
		      unsigned char *ivec, int *num, const int enc)
    {
    CRYPTO_cfb128_1_encrypt(in,out,length,key,ivec,num,enc,(block128_f)AES_encrypt);
    }

void AES_cfb8_encrypt(const unsigned char *in, unsigned char *out,
		      size_t length, const AES_KEY *key,
		      unsigned char *ivec, int *num, const int enc)
    {
    CRYPTO_cfb128_8_encrypt(in,out,length,key,ivec,num,enc,(block128_f)AES_encrypt);
    }

