/*
 *  Author : WangBoJing , email : 1989wangbojing@gmail.com
 * 
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of NALEX Inc. (C) 2016
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
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>
#include <poll.h>

#include "udpclient_proto.h"

#define BUFSIZE 1024

#define NTY_CRCTABLE_LENGTH			256
#define NTY_CRC_KEY		0x04c11db7ul

static U32 u32CrcTable[NTY_CRCTABLE_LENGTH] = {0};


void ntyGenCrcTable(void) {
	U16 i,j;
	U32 u32CrcNum = 0;

	for (i = 0;i < NTY_CRCTABLE_LENGTH;i ++) {
		U32 u32CrcNum = (i << 24);
		for (j = 0;j < 8;j ++) {
			if (u32CrcNum & 0x80000000L) {
				u32CrcNum = (u32CrcNum << 1) ^ NTY_CRC_KEY;
			} else {
				u32CrcNum = (u32CrcNum << 1);
			}
		}
		u32CrcTable[i] = u32CrcNum;
	}
}

U32 ntyGenCrcValue(U8 *buf, int length) {
	U32 u32CRC = 0xFFFFFFFF;
	
	while (length -- > 0) {
		u32CRC = (u32CRC << 8) ^ u32CrcTable[((u32CRC >> 24) ^ *buf++) & 0xFF];
	}

	return u32CRC;
}


/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

#define RECV_BUFFER_SIZE	NTY_LOGIN_ACK_LENGTH
#define DEVID				1

static int level = 0;
ClientInfo clientInfo[12] = {0};
static int clientIndex = 0;
static int clientCount = 0;
struct sockaddr_in clientaddr;
struct sockaddr_in serveraddr;
static U8 heartbeartRun = 0;
static int frienddevid = 0;


void* heartbeatThread(void *arg) {
	ThreadArg threadArg = *((ThreadArg*)arg);
	int devid = threadArg.devid;
	int sockfd = threadArg.sockfd;
	int len, n;
	U8 buf[NTY_LOGIN_ACK_LENGTH] = {0};
	if (heartbeartRun == 1) {
		heartbeartRun = 1;
		return NULL;
	}
	while (1) {
		bzero(buf, NTY_LOGIN_ACK_LENGTH);   
		buf[0] = 'A';
		buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_HEARTBEAT_REQ;
		*(U64*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]) = (U64) devid;
		*(U32*)(&buf[NTY_PROTO_LOGIN_REQ_ACKNUM_IDX]) = (U32) 12345;
		*(U32*)(&buf[NTY_PROTO_LOGIN_REQ_CRC_IDX]) = ntyGenCrcValue(buf, NTY_PROTO_LOGIN_REQ_CRC_IDX);
		
		len = NTY_PROTO_LOGIN_REQ_CRC_IDX+sizeof(U32);
		
		n = sendto(sockfd, buf, len, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
		
		sleep(10);
	}
}

void* recvThread(void *arg) {
	ThreadArg threadArg = *((ThreadArg*)arg);
	int devid = threadArg.devid;
	int sockfd = threadArg.sockfd;
	int ret, n, serverlen, err;
	U8 buf[RECV_BUFFER_SIZE] = {0};
	
	struct sockaddr_in addr;
	int clientlen = sizeof(struct sockaddr_in);
	pthread_t heartbeatThread_id;
	struct pollfd fds;
	fds.fd = sockfd;
	fds.events = POLLIN;
	
	while (1) {
		ret = poll(&fds, 1, 5);
		if (ret) {
			bzero(buf, NTY_LOGIN_ACK_LENGTH);   
			 
			n = recvfrom(sockfd, buf, RECV_BUFFER_SIZE, 0, (struct sockaddr *) &addr, &clientlen);    
			printf("%d.%d.%d.%d:%d size:%d --> %x\n", *(unsigned char*)(&addr.sin_addr.s_addr), *((unsigned char*)(&addr.sin_addr.s_addr)+1),													
				*((unsigned char*)(&addr.sin_addr.s_addr)+2), *((unsigned char*)(&addr.sin_addr.s_addr)+3),													
				addr.sin_port, n, buf[NTY_PROTO_TYPE_IDX]);
			
			if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_LOGIN_ACK) {
				int i = 0;
				int count = *(U16*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDS_COUNT_IDX]);
				clientCount = ((count < 12 && count >= 0)  ? count : 12);
				
				for (i = 0;i < count && i < 12;i ++) {
					clientInfo[i].devId = *(U64*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_DEVID_IDX(i)]);
					clientInfo[i].addr = *(U32*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_ADDR_IDX(i)]);
					clientInfo[i].port = *(U16*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_PORT_IDX(i)]);
					
					printf(" P2P client %d: %d.%d.%d.%d:%d \n", i+1,*(unsigned char*)(&clientInfo[i].addr), *((unsigned char*)(&clientInfo[i].addr)+1),													
						*((unsigned char*)(&clientInfo[i].addr)+2), *((unsigned char*)(&clientInfo[i].addr)+3),													
						clientInfo[i].port);
				}
				
				level = 2;
				printf(" Setup heartbeat Thread\n");
				err = pthread_create(&heartbeatThread_id, NULL, heartbeatThread, arg);
				if (err != 0) {
					printf(" can't create thread:%s\n", strerror(err));
					exit(0);
				}
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_HEARTBEAT_ACK) {
				
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_LOGOUT_ACK) {
				
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_ADDR_ACK) {
			
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_NOTIFY_ACK) {
				
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_NOTIFY_REQ) {	
				clientaddr.sin_family = AF_INET;							
				clientaddr.sin_addr.s_addr = *(U32*)(&buf[NTY_PROTO_P2P_NOTIFY_IPADDR_IDX]);
				clientaddr.sin_port = *(U16*)(&buf[NTY_PROTO_P2P_NOTIFY_IPPORT_IDX]);
				
				printf(" Notify %d.%d.%d.%d:%d\n", *(unsigned char*)(&clientaddr.sin_addr.s_addr), *((unsigned char*)(&clientaddr.sin_addr.s_addr)+1),													
				*((unsigned char*)(&clientaddr.sin_addr.s_addr)+2), *((unsigned char*)(&clientaddr.sin_addr.s_addr)+3),													
				clientaddr.sin_port);
				
				level = 2;
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_CONNECT_REQ) {	
				memcpy(&clientaddr, &addr, sizeof(struct sockaddr_in));
				level = 4;
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_CONNECT_ACK) {
				printf(" P2P Success\n");
				level = 0x0A;
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_ADDR_ACK){
				int i = 0, j = 0;
				int count = buf[NTY_PROTO_P2PADDR_REQ_FRIENDS_COUNT_IDX];
				for (i = 0;i < clientCount && i < 12;i ++) {
					for (j = 0;j < count;j ++) {
						if (*(U32*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_DEVID_IDX(j)]) ==  clientInfo[i].devId) {
							clientInfo[i].addr = *(U32*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_ADDR_IDX(j)]);
							clientInfo[i].port = *(U16*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_PORT_IDX(j)]);
						}
					}
				}
				level = 1;
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_DATAPACKET_NOTIFY_ACK) {
				printf(" send Success\n");
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_DATAPACKET_NOTIFY_REQ) {
				U8 recvBuf[NTY_P2P_NOTIFY_ACK_LENGTH] = {0};
				U16 count = *(U16*)(&buf[NTY_PROTO_DATAPACKET_NOTIFY_CONTENT_COUNT_IDX]);
				
				frienddevid = *(U64*)(&buf[NTY_PROTO_DATAPACKET_NOTIFY_SRC_DEVID_IDX]);
				memcpy(recvBuf, &buf[NTY_PROTO_DATAPACKET_NOTIFY_CONTENT_IDX], count);
				printf(" recv : %s\n", recvBuf);
				level = 5;
			} else {
				printf("%d.%d.%d.%d:%d size:%d --> %s\n", *(unsigned char*)(&addr.sin_addr.s_addr), *((unsigned char*)(&addr.sin_addr.s_addr)+1),													
				*((unsigned char*)(&addr.sin_addr.s_addr)+2), *((unsigned char*)(&addr.sin_addr.s_addr)+3),													
				addr.sin_port, n, buf);
			}
				
		}
	}
}

void sendLoginPacket(int sockfd, int devid) {
	int len, n;
	U8 buf[RECV_BUFFER_SIZE] = {0};
	
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_LOGIN_REQ;
	*(U64*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]) = (U64) devid;
	*(U32*)(&buf[NTY_PROTO_LOGIN_REQ_ACKNUM_IDX]) = (U32) 12345;
	*(U32*)(&buf[NTY_PROTO_LOGIN_REQ_CRC_IDX]) = ntyGenCrcValue(buf, NTY_PROTO_LOGIN_REQ_CRC_IDX);
	//serverlen = sizeof(serveraddr);
	len = NTY_PROTO_LOGIN_REQ_CRC_IDX+sizeof(U32);
			
	n = sendto(sockfd, buf, len, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
}



void *run(void *arg) {
    int sockfd, portno, portno_1, n;
    int serverlen, len;
    struct sockaddr_in tempaddr;
    struct sockaddr_in addr;
    struct hostent *server, *server_1;
    char *hostname = "127.0.0.1", *hostname_1; //"120.76.25.198"
    char buf[BUFSIZE];
    int key = 0, err;
    pthread_t recThread_id;
    int p2pConnectReqCount = 0;
	int devid = 0;
	ThreadArg threadArg = {0};

    
    //hostname = argv[1];
    portno = 8888;
#if 1    
    printf(" Press DevId <1 or 2>: ");
    scanf("%d", &devid);
#else
	srand(time(NULL));  
	devid = rand() % 5000;
#endif
    //hostname_1 = argv[3];
    //portno_1 = atoi(argv[4]);
    ntyGenCrcTable();

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname("120.76.25.198");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

	threadArg.sockfd = sockfd;
	threadArg.devid = devid;
	err = pthread_create(&recThread_id, NULL, recvThread, &threadArg);
	if (err != 0) {
		printf(" can't create thread:%s\n", strerror(err));
		exit(0);
	}
	

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);
    
#if 1 // send Login Req
	printf(" bbbbbbbbbbb \n");
	while (1) {
		bzero(buf, BUFSIZE);
		buf[0] = 'A';
		if (level == 0) { // send Login Req
			sendLoginPacket(sockfd, devid);
			sleep(3);
		} else if (level == 1) { //NTY_PROTO_P2P_CONNECT_REQ
			int i = 0;
			
			if (p2pConnectReqCount ++ == 3) {
				level = 0x0B;
				p2pConnectReqCount = 0;
			}
			sleep(3);
			
			buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2P_CONNECT_REQ;
			*(U64*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]) = (U64) devid;
			*(U32*)(&buf[NTY_PROTO_LOGIN_REQ_ACKNUM_IDX]) = (U32) 12345;
			*(U32*)(&buf[NTY_PROTO_LOGIN_REQ_CRC_IDX]) = ntyGenCrcValue(buf, NTY_PROTO_LOGIN_REQ_CRC_IDX);
			
			serverlen = sizeof(serveraddr);
			len = NTY_PROTO_LOGIN_REQ_CRC_IDX+sizeof(U32);
			
			for (i = 0;i < clientCount;i ++) {
				tempaddr.sin_addr.s_addr = clientInfo[i].addr;
				tempaddr.sin_port = clientInfo[i].port;
				
				if (tempaddr.sin_addr.s_addr == 0 || tempaddr.sin_port == 0) {
					buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2P_ADDR_REQ;
					*(U64*)(&buf[NTY_PROTO_P2PADDR_REQ_DEVID_IDX]) = (U64) devid;
					*(U32*)(&buf[NTY_PROTO_P2PADDR_REQ_ACKNUM_IDX]) = (U32) 12345;
					*(U32*)(&buf[NTY_PROTO_P2PADDR_REQ_FRIENDS_COUNT_IDX]) = clientInfo[i].devId;
					n = sendto(sockfd, buf, len, 0, (struct sockaddr *)&serveraddr, serverlen);
				} else {
					printf(" sendto %d.%d.%d.%d:%d\n", *(unsigned char*)(&tempaddr.sin_addr.s_addr), *((unsigned char*)(&tempaddr.sin_addr.s_addr)+1),													
						*((unsigned char*)(&tempaddr.sin_addr.s_addr)+2), *((unsigned char*)(&tempaddr.sin_addr.s_addr)+3),													
						tempaddr.sin_port);
					
					n = sendto(sockfd, buf, len, 0, (struct sockaddr *)&tempaddr, serverlen);
				}
				
			}
			
		} else if (level == 2) { //send NTY_PROTO_P2P_CONNECT_REQ
			
			if (p2pConnectReqCount ++ == 3) {
				level = 0x0B;
				p2pConnectReqCount = 0;
			}
			
			buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2P_CONNECT_REQ;
			*(U64*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]) = (U64) devid;
			*(U32*)(&buf[NTY_PROTO_LOGIN_REQ_ACKNUM_IDX]) = (U32) 12345;
			*(U32*)(&buf[NTY_PROTO_LOGIN_REQ_CRC_IDX]) = ntyGenCrcValue(buf, NTY_PROTO_LOGIN_REQ_CRC_IDX);
			
			serverlen = sizeof(serveraddr);
			len = NTY_PROTO_LOGIN_REQ_CRC_IDX+sizeof(U32);
			
			printf(" sendto %d.%d.%d.%d:%d\n", *(unsigned char*)(&clientaddr.sin_addr.s_addr), *((unsigned char*)(&clientaddr.sin_addr.s_addr)+1),													
				*((unsigned char*)(&clientaddr.sin_addr.s_addr)+2), *((unsigned char*)(&clientaddr.sin_addr.s_addr)+3),													
				clientaddr.sin_port);
			n = sendto(sockfd, buf, len, 0, (struct sockaddr *)&clientaddr, serverlen);
			
			sleep(3);
		}  else if (level == 3) { //NTY_PROTO_HEARTBEAT_REQ
			//run in heartbeatThread
			
			
		} else if (level == 4) {
			buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2P_CONNECT_ACK;
			*(U64*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]) = (U64) devid;
			*(U32*)(&buf[NTY_PROTO_LOGIN_REQ_ACKNUM_IDX]) = (U32) 12345;
			*(U32*)(&buf[NTY_PROTO_LOGIN_REQ_CRC_IDX]) = ntyGenCrcValue(buf, NTY_PROTO_LOGIN_REQ_CRC_IDX);
			
			serverlen = sizeof(addr);
			len = NTY_PROTO_LOGIN_REQ_CRC_IDX+sizeof(U32);
			
			n = sendto(sockfd, buf, len, 0, (struct sockaddr *)&clientaddr, serverlen);
			
			level = 0x0A;
		} else if (level == 5) {
			buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_DATAPACKET_NOTIFY_ACK;
			*(U64*)(&buf[NTY_PROTO_DATAPACKET_ACK_DEVID_IDX]) = (U64) devid;
			*(U32*)(&buf[NTY_PROTO_DATAPACKET_ACK_ACKNUM_IDX]) = (U32) 12345;
			*(U32*)(&buf[NTY_PROTO_DATAPACKET_ACK_SRC_DEVID_IDX]) = (U32) frienddevid;
			*(U32*)(&buf[NTY_PROTO_DATAPACKET_ACK_CRC_IDX]) = ntyGenCrcValue(buf, NTY_PROTO_DATAPACKET_ACK_CRC_IDX);
	    	
	    	n = sendto(sockfd, buf, NTY_PROTO_DATAPACKET_ACK_CRC_IDX+4, 0, (struct sockaddr *)&serveraddr, serverlen);
	    	
	    	level = 0x0B;
		} else if (level == 0x0A) {
			printf("Please enter msg: ");
	    	fgets(buf, BUFSIZE, stdin);
	    	
	    	n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&clientaddr, serverlen);
		} else if (level == 0x0B) { //server proxy
			U8 *tempBuf;
			
			buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_DATAPACKET_REQ;
			*(U64*)(&buf[NTY_PROTO_DATAPACKET_DEVID_IDX]) = (U64) devid;
			*(U32*)(&buf[NTY_PROTO_DATAPACKET_ACKNUM_IDX]) = (U32) 12345;
			
			*(U32*)(&buf[NTY_PROTO_DATAPACKET_RECE_COUNT_IDX]) = (U32) 0;
			
			tempBuf = &buf[NTY_PROTO_DATAPACKET_CONTENT_IDX(buf[NTY_PROTO_DATAPACKET_RECE_COUNT_IDX])];
			//*(U32*)(&buf[NTY_PROTO_LOGIN_REQ_CRC_IDX]) = ntyGenCrcValue(buf, NTY_PROTO_LOGIN_REQ_CRC_IDX);
			printf("Proxy Please enter msg: ");
	    	fgets(tempBuf, BUFSIZE, stdin);
	    	
	    	len = strlen(tempBuf);
	    	*(U16*)(&buf[NTY_PROTO_DATAPACKET_CONTENT_COUNT_IDX(buf[NTY_PROTO_DATAPACKET_RECE_COUNT_IDX])]) = (U16)len;
	    	len += 2;
	    	len += NTY_PROTO_DATAPACKET_CONTENT_IDX(buf[NTY_PROTO_DATAPACKET_RECE_COUNT_IDX]);
	    	
	    	*(U32*)(&buf[len]) = ntyGenCrcValue(buf, len);
	    	len += sizeof(U32);
	    	
	    	n = sendto(sockfd, buf, len, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
		} else {
			
		}
	}
#endif
    
#if 0    
    /* gethostbyname: get the server's DNS entry */
    server_1 = gethostbyname(hostname_1);
    if (server_1 == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname_1);
        exit(0);
    }
    bzero((char *) &serveraddr_1, sizeof(serveraddr_1));
  	serveraddr_1.sin_family = AF_INET;
  	bcopy((char *)server_1->h_addr, 
	  (char *)&serveraddr_1.sin_addr.s_addr, server_1->h_length);
  	serveraddr_1.sin_port = htons((unsigned short)portno_1);

		srand(time(NULL));  

		while (1) {
	    /* get a message from the user */
	    bzero(buf, BUFSIZE);
	    printf("Please enter msg: ");
	    //fgets(buf, BUFSIZE, stdin);
	
			key = rand() % 500;
			if (key % 4 == 1) {
				buf[0] = 0x01;
				strcpy(buf+1, "NTY_PROTO_LOGIN_REQ:101345");
				
				/* send the message to the server */
		    serverlen = sizeof(serveraddr);
		    n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, serverlen);
		    if (n < 0) 
		      error("ERROR in sendto");
		      
		    /* print the server's reply */
		    n = recvfrom(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&addr, &serverlen);
		    if (n < 0) 
		      error("ERROR in recvfrom");
		    printf("Echo from server: %s\n", buf);
			} else if (key % 4 == 2) {
				buf[0] = 0x02;
				strcpy(buf+1, "NTY_PROTO_HEARTBEAT_REQ:201345");
				
				/* send the message to the server */
		    serverlen = sizeof(serveraddr);
		    n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, serverlen);
		    if (n < 0) 
		      error("ERROR in sendto");
		    
		    /* print the server's reply */
		    n = recvfrom(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&addr, &serverlen);
		    if (n < 0) 
		      error("ERROR in recvfrom");
		    printf("Echo from server: %s\n", buf);
			} else if (key % 4 == 3) {
				buf[0] = 0x03;
				strcpy(buf+1, "NTY_PROTO_LOGOUT_REQ:301345");
				
				/* send the message to the server */
		    serverlen = sizeof(serveraddr_1);
		    n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr_1, serverlen);
		    if (n < 0) 
		      error("ERROR in sendto");
		    
		    /* print the server's reply */
		    n = recvfrom(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&addr, &serverlen);
		    if (n < 0) 
		      error("ERROR in recvfrom");
		    printf("Echo from server: %s\n", buf);
			} else if (key % 4 == 0) {
				buf[0] = 0x11;
				strcpy(buf+1, "NTY_PROTO_P2P_ADDR_REQ: 401345");
				
				/* send the message to the server */
		    serverlen = sizeof(serveraddr_1);
		    n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr_1, serverlen);
		    if (n < 0) 
		      error("ERROR in sendto");
		    
		    /* print the server's reply */
		    n = recvfrom(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&addr, &serverlen);
		    if (n < 0) 
		      error("ERROR in recvfrom");
		    printf("Echo from server: %s\n", buf);
			} else {
				buf[0] = 0x05;
				
				/* send the message to the server */
		    serverlen = sizeof(serveraddr_1);
		    n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr_1, serverlen);
		    if (n < 0) 
		      error("ERROR in sendto");
		    
		    /* print the server's reply */
		    n = recvfrom(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&addr, &serverlen);
		    if (n < 0) 
		      error("ERROR in recvfrom");
		    printf("Echo from server: %s\n", buf);
			}
			
	    
	    
	    
	    
	    sleep(1);
  	}
#endif

    return 0;
}



int main() {
#if 0
	pid_t fpid;
	fpid = fork();
	if (fpid < 0) {
		printf("error in fork!");   
	} else if (fpid == 0) {
		int err = 0, i = 0;
		pthread_t sendThread_id;
		pid_t id = fork();
		fork();
		fork();
		fork();
		fork();
		
		for (i = 0;i < 100;i ++) {
			err = pthread_create(&sendThread_id, NULL, run, NULL);
			if (err != 0) {
				printf(" can't create thread:%s\n", strerror(err));
				exit(0);
			}
		}
	} else {
		int err = 0, i = 0;
		pthread_t sendThread_id;
		pid_t id = fork();
		fork();
		fork();
		fork();
		fork();
		
		for (i = 0;i < 100;i ++) {
			err = pthread_create(&sendThread_id, NULL, run, NULL);
			if (err != 0) {
				printf(" can't create thread:%s\n", strerror(err));
				exit(0);
			}
		}
	}
#else
	int err = 0;
	pthread_t sendThread_id;

	err = pthread_create(&sendThread_id, NULL, run, NULL);
	if (err != 0) {
		printf(" can't create thread:%s\n", strerror(err));
		exit(0);
	}
#endif
	while(1);
}

