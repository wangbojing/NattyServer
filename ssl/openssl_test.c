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

//gcc -o openssl_test openssl_test.c openssl_aes_cfb.c openssl_aes_core.c openssl_cfb128.c -I ../include/

#include <stdio.h>
#include <string.h>
#include "openssl_aes.h"


int main(int argc, char **argv) {

	unsigned char ckey [] = "helloworldkey"; 
 	unsigned char ivec [] = "goodbyworldkey"; 
 
 	int bytes_read; 
 	unsigned char indata [AES_BLOCK_SIZE], outdata [AES_BLOCK_SIZE];; 
 	unsigned char decryptdata [AES_BLOCK_SIZE]; 
 
 	AES_KEY keyEn; 

	AES_set_encrypt_key(ckey, 128, & keyEn);
 
 	int num = 0; 
 
 	strcpy((char *)indata, "Hello World");
 	bytes_read = sizeof(indata);
 
 	AES_cfb128_encrypt(indata, outdata, bytes_read, &keyEn, ivec, &num, AES_ENCRYPT);
 	printf("orig: %s\n", indata);

	int i = 0;

	for (i = 0;i < AES_BLOCK_SIZE;i ++) {
		printf("outdata:%x\n", outdata[i]);
	}
 	
 	return 0; 

}


