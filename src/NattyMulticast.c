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

#include "NattyMulticast.h"
#include "NattyThreadPool.h"
#include "NattyFilter.h"
#include "NattyUtils.h"


void *ntyMulticastServerInitialize(MulticastServer *server) {
	struct sockaddr_in ia;
	U8 ttl = 0xFF;
	int yes = 0;

	server->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (server->sockfd < 0) {
		ntylog("natty tcp server listen failed\n");
		return NULL;
	}

	bzero(&server->mreq, sizeof(struct ip_mreq));
	inet_pton(AF_INET, NTY_MULTICAST_ADDRESS, &ia.sin_addr);
	bcopy(&ia.sin_addr, &server->mreq.imr_multiaddr.s_addr, sizeof(struct in_addr));

	server->mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(server->sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &server->mreq, sizeof(struct ip_mreq)) == -1) {
		perror("setsockopt");
		return NULL;
	}
	setsockopt(server->sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &yes, sizeof(yes));
	setsockopt(server->sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(unsigned char));

	memset(&server->addr, 0, sizeof(server->addr));
	server->addr.sin_family = AF_INET;
	server->addr.sin_port = htons(NYT_MULTICAST_PORT);
	inet_pton(AF_INET, NTY_MULTICAST_ADDRESS, &server->addr.sin_addr);
	
	if (bind(server->sockfd, (struct sockaddr*)&server->addr, sizeof(server->addr)) < 0) {
		ntylog("natty tcp server bind failed\n");
		return NULL;
	}

	return server;
}


void *ntyMulticastServerRelease(MulticastServer *server) {
	memset(&server->addr, 0, sizeof(server->addr));
	close(server->sockfd);
	return server;
}

static void ntyMulticastServerJob(Job *job) {
	
	RequestPacket *req = (RequestPacket*)job->user_data;
	void* pFilter = ntyProtocolFilterInstance();
	ntyProtocolFilterProcess(pFilter, req->buffer, req->length, req->client);

	freeRequestPacket(req);
	free(job);
}

void *ntyMulticastServerCtor(void *self, va_list *params) {
	return ntyMulticastServerInitialize(self);
}

void *ntyMulticastServerDtor(void *self) {
	return ntyMulticastServerRelease(self);
}


int ntyMulticastServerProcess(const void *_self) {
	const MulticastServer *self = _self;
	
	int ret = -1, n;
	int clientlen = sizeof(struct sockaddr_in);	
	U8 recvmsg[RECV_BUFFER_SIZE] = {0};

	Job *job;
	void* pThreadPool = ntyThreadPoolInstance();
	
	struct pollfd fds;
	fds.fd = self->sockfd;
	fds.events = POLLIN;

	while(1) {
		ret = poll(&fds, 1, 5);
		if (ret) { //data is comming
			struct sockaddr_in addr;
			RequestPacket *req = (RequestPacket*)allocRequestPacket();
			if (req == NULL) {
				freeRequestPacket(req);
				return -1;
			}
			n = recvfrom(self->sockfd, recvmsg, RECV_BUFFER_SIZE, 0, (struct sockaddr *) &addr, &clientlen);
			if (n < 0) {
				printf("recvfrom error in udptalk!\n");
				freeRequestPacket(req);
				continue;
			} else {
				ntyU8ArrayToU64(&recvmsg[NTY_PROTO_DEVID_IDX], &req->client->devId);
				//req->client->ackNum = ntyU8ArrayToU32(&recvmsg[NTY_PROTO_ACKNUM_IDX])+1;

				req->sockfd = self->sockfd;
				//req->client->watcher = NULL;
				req->connectType = PROTO_TYPE_MULTICAST;
				req->length = (U16)n;
				req->buffer = (U8*)malloc(n);
				if (req->buffer == NULL) {
					perror("malloc Recv Buffer failed\n");

					free(req->client);
					free(req);
					continue;
				}			
				memcpy(req->buffer, recvmsg, n);

				job = (Job*)malloc(sizeof(*job));

				if (job == NULL) {
					perror("malloc Job failed\n");
					freeRequestPacket(req);
					continue;
				}

				job->job_function  = ntyMulticastServerJob;
				job->user_data = req;

				ntyThreadPoolPush(pThreadPool, job);
			}
		}
	}

	return 0;
}

int ntyMulticastServerSend(const void *_self, U8 *data, int length) {
	//*(U32*)(&network->buffer[len-sizeof(U32)]) = ntyGenCrcValue(network->buffer, len-sizeof(U32));
	const MulticastServer *self = _self;
	U8 buffer[RECV_BUFFER_SIZE] = {0};
	
	U32 Crc = ntyGenCrcValue(data, length-sizeof(U32));
	memcpy(buffer, data, length);
	buffer[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_MULTICAST_REQ;
	memcpy(buffer+length-sizeof(U32), &Crc, sizeof(U32));
	
	return sendto(self->sockfd, data, length, 0, (struct sockaddr *)&self->addr, sizeof(struct sockaddr_in));
}


static const MulticastServerHandle ntyMulticastServer = {
	sizeof(MulticastServer),
	ntyMulticastServerCtor,
	ntyMulticastServerDtor,
	ntyMulticastServerProcess,
	ntyMulticastServerSend,
};

static const void *pNtyMulticastServer = &ntyMulticastServer;
static void *pMulticastServer = NULL;

int ntyMulticastServerRun(const void *arg) {
	const MulticastServerHandle * const *pServerConf = arg;
	
	if (arg && (*pServerConf)->process && (*pServerConf)) {
		(*pServerConf)->process(arg);
	}
	return 0;
}

int ntyMulticastServerSendHandle(const void *arg, U8 *data, int length) {
	const MulticastServerHandle * const *pServerConf = arg;
	if (arg && (*pServerConf)->send && (*pServerConf)) {
		return (*pServerConf)->send(arg, data, length);
	}
	return 0;
}

void* ntyMulticastServerInstance(void) {
	if (pMulticastServer == NULL) {
		void *pServer = New(pNtyMulticastServer);
		if ((unsigned long)NULL != cmpxchg((void*)(&pMulticastServer), (unsigned long)NULL, (unsigned long)pServer, WORD_WIDTH)) {
			Delete(pServer);
		}
	}
	return pMulticastServer;
}


int ntyMulticastSend(U8 *data, int length) {
	void *multi = ntyMulticastServerInstance();
	return ntyMulticastServerSendHandle(multi, data, length);
}



