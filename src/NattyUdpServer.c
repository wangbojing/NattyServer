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



#include "NattyFilter.h"
#include "NattyUdpServer.h"
#include "NattyThreadPool.h"

void error(char *msg) {  
	perror(msg);  
	exit(1);
}


void* ntyUdpServerCtor(void *_self, va_list *params) {
	UdpServer *self = _self;
	short port = NATTY_UDP_SERVER;
	int optval; /* flag value for setsockopt */  

	self->sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
	if (self->sockfd < 0)     
		error("ERROR opening socket");

	optval = 1;  
	setsockopt(self->sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));  /*   * build the server's Internet address   */  
	bzero((char *) &self->addr, sizeof(struct sockaddr_in));  
	self->addr.sin_family = AF_INET;  
	self->addr.sin_addr.s_addr = htonl(INADDR_ANY);  
	self->addr.sin_port = htons(port); 

	if (bind(self->sockfd, (struct sockaddr *)&(self->addr), sizeof(struct sockaddr_in)) < 0)     
		error("ERROR on binding"); 

	return self;
}

void* ntyUdpServerDtor(void *_self) {
	UdpServer *self = _self;
	if (self->sockfd) {
		close(self->sockfd);
		self->sockfd = 0;
	}
	return self;
}

void freeRequestPacket(void *pReq) {
	RequestPacket *req = pReq;

	if (req != NULL) {
		if (req->client != NULL) {
			free(req->client);
		}
		if (req->buffer != NULL) {
			free(req->buffer);
		}
		free(req);
	}
}

void* allocRequestPacket(void) {
	RequestPacket *req = (RequestPacket*)malloc(sizeof(RequestPacket));
	if (req == NULL) {
		perror("malloc Request Packet failed\n");
		return NULL;
	}
	req->client = (UdpClient*)malloc(sizeof(UdpClient));
	if (req->client == NULL ) {
		perror("malloc client failed\n");
		
		free(req);
		return NULL;
	}
#if 0
	req->buffer = (U8*)malloc(sizeof(RECV_BUFFER_SIZE));
	if (req->buffer == NULL) {
		perror("malloc Recv Buffer failed\n");

		free(req->client);
		free(req);
		return NULL;
	}
#endif
	return req;
}

static void ntyUdpServerJob(Job *job) {
	
	RequestPacket *req = (RequestPacket*)job->user_data;
	void* pFilter = ntyProtocolFilterInstance();
	ntyProtocolFilterProcess(pFilter, req->buffer, req->length, req->client);

	//ntyProtocolFilterRelease(pFilter);
	freeRequestPacket(req);
	free(job);
}

int ntyUdpServerProcess(const void *_self) {
	const UdpServer *self = _self;
	//struct sockaddr_in clientaddr;
	int clientlen = sizeof(struct sockaddr_in);  
	
	struct pollfd fds;
	int ret = -1, n;
	unsigned char buf[RECV_BUFFER_SIZE];
	RequestPacket *req;

	Job *job;
	void* pThreadPool = ntyThreadPoolInstance();
	void* pFilter = ntyProtocolFilterInstance();

	if (self->sockfd <= 0) {
		error("Udp Server Socket no Initial");
	}	
	
	fds.fd = self->sockfd;
	fds.events = POLLIN;
	while(1) {
		ret = poll(&fds, 1, 5);
		if (ret) { //data is comming
			req = (RequestPacket*)allocRequestPacket();
			if (req == NULL) {
				continue;
			}
			
			bzero(buf, RECV_BUFFER_SIZE);    
			n = recvfrom(self->sockfd, buf, RECV_BUFFER_SIZE, 0, (struct sockaddr *) &req->client->addr, &clientlen);    
			ntylog("%d.%d.%d.%d:%d, length:%d --> %x, id:%lld\n", *(unsigned char*)(&req->client->addr.sin_addr.s_addr), *((unsigned char*)(&req->client->addr.sin_addr.s_addr)+1),													
				*((unsigned char*)(&req->client->addr.sin_addr.s_addr)+2), *((unsigned char*)(&req->client->addr.sin_addr.s_addr)+3),													
				req->client->addr.sin_port, n, buf[NTY_PROTO_TYPE_IDX], *(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]));	
			// proccess
			// i think process protocol and search client id from rb-tree
			req->client->sockfd = self->sockfd;
			req->client->clientType = PROTO_TYPE_UDP;
			req->length = (U16)n;
			req->buffer = (U8*)malloc(n);
			if (req->buffer == NULL) {
				perror("malloc Recv Buffer failed\n");

				free(req->client);
				free(req);
				continue;
			}			
			memcpy(req->buffer, buf, n);

			job = (Job*)malloc(sizeof(*job));

			if (job == NULL) {
				perror("malloc Job failed\n");
				freeRequestPacket(req);
				continue;
			}

			job->job_function  = ntyUdpServerJob;
			job->user_data = req;

			ntyThreadPoolPush(pThreadPool, job);
			//use thread pool to deal with message			

			//send to ack
			//n = sendto(self->sockfd, buf, n, 0, (struct sockaddr *) &pClient->addr, clientlen);    
			//if (n < 0)       
			//	error("ERROR in sendto");  
		}
	}

	ntyProtocolFilterProcess(pFilter, req->buffer, req->length, req->client);
	//free(pClient);
	//ntyProtocolFilterRelease(pFilter);
	return 0;
}


static const UdpServerOpera ntyUdpServer = {
	sizeof(UdpServer),
	ntyUdpServerCtor,
	ntyUdpServerDtor,
	ntyUdpServerProcess,
};

static const void *pNtyUdpServer = &ntyUdpServer;
static void *pUdpServer = NULL;

int ntyUdpServerRun(const void *arg) {
	const UdpServerOpera * const *pServerConf = arg;
	
	if (arg && (*pServerConf)->process && (*pServerConf)) {
		(*pServerConf)->process(arg);
	}
	return 0;
}

int ntyClientCompare(const UdpClient *clientA, const UdpClient *clientB) {
	if ((clientA->addr.sin_port == clientB->addr.sin_port) 
		&& (clientA->addr.sin_addr.s_addr == clientB->addr.sin_addr.s_addr)) {
		return 1;
	}

	return 0;
}

int ntySendBuffer(const UdpClient *client, unsigned char *buffer, int length) {
	if (client->clientType == PROTO_TYPE_UDP) {
		return sendto(client->sockfd, buffer, length, 0, (struct sockaddr *)&client->addr, sizeof(struct sockaddr_in));
	} else if (client->clientType == PROTO_TYPE_TCP) {
		int ret = send(client->sockfd, buffer, length, 0);
		ntydbg(" tcp send success : %d\n", ret);
		return ret;
	}
	return -1;
}

void* ntyUdpServerInstance(void) {
	if (pUdpServer == NULL) {
		void *pServer = New(pNtyUdpServer);
		if ((unsigned long)NULL != cmpxchg((void*)(&pUdpServer), (unsigned long)NULL, (unsigned long)pServer, WORD_WIDTH)) {
			Delete(pServer);
		}
	}
	return pUdpServer;
}

