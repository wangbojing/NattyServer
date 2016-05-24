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
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <stdlib.h>//

#include <sys/mman.h>

int main() {
	int fd = -1;
	char buf[4096];
	char *pStart = NULL;
	struct pollfd fds;
	int ret;

	strcpy(buf, "Mem is char dev!\n");
	printf("1BUF: %s\n", buf);

	fd = open("/dev/DaveMqDev", O_RDWR);
	if (fd < 0) {
		perror("/dev/DaveMqDev");
		return -1;
	}
#if 1
	fds.fd = fd;
	fds.events = POLLIN;
	while (1) {
		ret = poll(&fds, 1, 50);
		if (ret == 0) {
			//printf("time out\n");
		} else {
			int len = 0;
			memset(buf, 0, sizeof(buf));
			pStart = mmap(NULL, 100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
			strcpy(buf, pStart);
			printf(" aaa buf:%s\n", buf);

			strcpy(pStart, "Buf is Not NULL!\n");
			memset(buf, 0, sizeof(buf));
			lseek(fd, 0, SEEK_SET);
			len = read(fd, buf, 4096);
			printf("buf: %s, len:%d\n", buf, len);
		}		
	}
#else
	memset(buf, 0, 4096);
	pStart = mmap(NULL, 100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	
#endif
	//fwrite(buf, sizeof(buf), 1, fp0);
	/*
	fseek(fp0, 0, SEEK_SET);

	memset(buf, 0, sizeof(buf));
	strcpy(buf, "buf is NULL!\n");
	printf("2buf: %s\n", buf);

	fread(buf, sizeof(buf), 1, fp0);
	printf("3buf:%s\n", buf);
	*/
	return 0;
}



