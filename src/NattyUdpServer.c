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
#include "NattyUtils.h"
#include "NattyRBTree.h"
#include "NattyResult.h"

#include <errno.h>

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
	memset(req, 0, sizeof(RequestPacket));
	
	req->client = (Client*)malloc(sizeof(Client));
	if (req->client == NULL ) {
		perror("malloc client failed\n");
		
		free(req);
		return NULL;
	}
	memset(req->client, 0, sizeof(Client));
	
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
	//void* pFilter = ntyProtocolFilterInstance();

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
			struct sockaddr_in addr;			
			
			bzero(buf, RECV_BUFFER_SIZE);    
			n = recvfrom(self->sockfd, buf, RECV_BUFFER_SIZE, 0, (struct sockaddr *) &addr, &clientlen);    
			ntylog("UdpRecv : %d.%d.%d.%d:%d, length:%d --> %x, id:%lld\n", *(unsigned char*)(&addr.sin_addr.s_addr), *((unsigned char*)(&addr.sin_addr.s_addr)+1),													
				*((unsigned char*)(&addr.sin_addr.s_addr)+2), *((unsigned char*)(&addr.sin_addr.s_addr)+3),													
				addr.sin_port, n, buf[NTY_PROTO_MSGTYPE_IDX], *(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]));	
			// proccess
			// i think process protocol and search client id from rb-tree
			//cancel UDP data parser
			continue;
#if 0
			req->client->devId = *(C_DEVID*)(&buffer[NTY_PROTO_DEVID_IDX]);
			req->client->ackNum = *(U32*)(buffer+NTY_PROTO_ACKNUM_IDX)+1;
#else
			ntyU8ArrayToU64(&buf[NTY_PROTO_DEVID_IDX], &req->client->devId);
			//req->client->ackNum = ntyU8ArrayToU32(&buf[NTY_PROTO_ACKNUM_IDX])+1;
#endif
			//req->sockfd = self->sockfd;
			//req->client->watcher = NULL;
			req->connectType = PROTO_TYPE_UDP;
			req->length = (U16)n;
			req->buffer = (U8*)malloc(n);
			if (req->buffer == NULL) {
				perror("malloc Recv Buffer failed\n");

				free(req->client);
				free(req);
				continue;
			}	
			memset(req->buffer, 0, sizeof(n));
			memcpy(req->buffer, buf, n);

			job = (Job*)malloc(sizeof(*job));

			if (job == NULL) {
				perror("malloc Job failed\n");
				freeRequestPacket(req);
				continue;
			}
			memset(job, 0, sizeof(Job));
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

	//ntyProtocolFilterProcess(pFilter, req->buffer, req->length, req->client);
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
#if 0
	if ((clientA->addr.sin_port == clientB->addr.sin_port) 
		&& (clientA->addr.sin_addr.s_addr == clientB->addr.sin_addr.s_addr)) {
		return 1;
	}
#endif
	return 0;
}


#if 0
int ntySendBuffer(const UdpClient *client, unsigned char *buffer, int length) {
	if (client == NULL) return -1;
	//if (client->sockfd == 0) return -1; //socket stored in watcher so delete it.

#if 1
	U32 Crc = ntyGenCrcValue(buffer, length-sizeof(U32));
	memcpy(buffer+length-sizeof(U32), &Crc, sizeof(U32));
#endif

	void *map = ntyMapInstance();
	ASSERT(map != NULL);

	NValue *nv = ntyMapSearch(map, client->devId);
	ASSERT(nv != NULL);

	if (client->connectType == PROTO_TYPE_UDP) {
		return sendto(nv->sockfd, buffer, length, 0, (struct sockaddr *)&nv->addr, sizeof(struct sockaddr_in));
	} else if (client->connectType == PROTO_TYPE_TCP) {
#if 0
		int ret = send(client->sockfd, buffer, length, 0);
#else
		int ret = send(nv->sockfd, buffer, length, 0);
#endif
		if (ret == -1) {
			ntylog(" tcp send errno : %d\n", errno);
		} else {
			ntylog(" tcp send success : %d\n", ret);
		}
		return ret;
	}
	return -1;
}

#else


static int ntySend(int sockfd, const void *buffer, int length, int flags) {

	int wrotelen = 0;
	int writeret = 0;

	unsigned char *p = (unsigned char *)buffer;

	struct pollfd pollfds = {0};
	pollfds.fd = sockfd;
	pollfds.events = ( POLLOUT | POLLERR | POLLHUP );

	do {
		int result = poll( &pollfds, 1, 5);
		if (pollfds.revents & POLLHUP) {
			
			ntylog(" ntySend errno:%d, revent:%x\n", errno, pollfds.revents);
			return NTY_RESULT_FAILED;
		}

		if (result < 0) {
			if (errno == EINTR) continue;

			ntylog(" ntySend errno:%d, result:%d\n", errno, result);
			return NTY_RESULT_FAILED;
		} else if (result == 0) {
		
			ntylog(" ntySend errno:%d, socket timeout \n", errno);
			return NTY_RESULT_FAILED;
		}

		writeret = send( sockfd, p + wrotelen, length - wrotelen, flags );
		if( writeret <= 0 )
		{
			break;
		}
		wrotelen += writeret ;

	} while (wrotelen < length);
	
	return wrotelen;
}


static int ntyRecv(int sockfd, void *data, size_t length, int *count) {
	int left_bytes;
	int read_bytes;
	int res;
	int ret_code;

	unsigned char *p;

	struct pollfd pollfds;
	pollfds.fd = sockfd;
	pollfds.events = ( POLLIN | POLLERR | POLLHUP );

	read_bytes = 0;
	ret_code = 0;
	p = (unsigned char *)data;
	left_bytes = length;

	while (left_bytes > 0) {

		read_bytes = recv(sockfd, p, left_bytes, 0);
		if (read_bytes > 0) {
			left_bytes -= read_bytes;
			p += read_bytes;
			continue;
 		} else if (read_bytes < 0) {
			if (!(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
				ret_code = (errno != 0 ? errno : EINTR);
			}
		} else {
			ret_code = ENOTCONN;
			break;
		}

		res = poll(&pollfds, 1, 5);
		if (pollfds.revents & POLLHUP) {
			ret_code = ENOTCONN;
			break;
		}

		if (res < 0) {
			if (errno == EINTR) {
				continue;
			}
			ret_code = (errno != 0 ? errno : EINTR);
		} else if (res == 0) {
			ret_code = ETIMEDOUT;
			break;
		}

	}

	if (count != NULL) {
		*count = length - left_bytes;
	}

	return ret_code;
}



int ntySendBuffer(ClientSocket *client, unsigned char *buffer, int length) {
	if (client == NULL) return NTY_RESULT_FAILED;
#if ENABLE_EV_WATCHER_MODE
	if (client->watcher == NULL) return NTY_RESULT_ERROR;
	int sockfd = client->watcher->fd;
#else
	int sockfd = client->sockfd;
#endif
#if 1
	U32 Crc = ntyGenCrcValue(buffer, length-sizeof(U32));
	memcpy(buffer+length-sizeof(U32), &Crc, sizeof(U32));
#endif

	if (client->connectType == PROTO_TYPE_UDP) {
#if 0 //dispatch
		return sendto(sockfd, buffer, length, 0, (struct sockaddr *)&client->addr, sizeof(struct sockaddr_in));
#else
		return NTY_RESULT_FAILED;
#endif
	} else if (client->connectType == PROTO_TYPE_TCP) {
#if 0
		int ret = send(sockfd, buffer, length, 0);
#else
		int ret = ntySend(sockfd, buffer, length, 0);
#endif
		if (ret == NTY_RESULT_FAILED) {
			ntylog(" tcp send errno : %d\n", errno);
			//delete client all fromId;
			//ntyClientCleanup(client);
		} else {
			ntylog(" tcp send success : %d\n", ret);
		}
		return ret;
	}
	return NTY_RESULT_FAILED;
}

int ntyProxyBuffer(ClientSocket *client, unsigned char *buffer, int length) {
	if (client == NULL) return NTY_RESULT_FAILED;
	int sockfd = client->watcher->fd;

	if (client->connectType == PROTO_TYPE_UDP) {
		return sendto(sockfd, buffer, length, 0, (struct sockaddr *)&client->addr, sizeof(struct sockaddr_in));
	} else if (client->connectType == PROTO_TYPE_TCP) {
		int ret = send(sockfd, buffer, length, 0);
		if (ret == -1) {
			ntylog(" tcp send errno : %d\n", errno);
			//release Client Resource
			//ntyClientCleanup(client);
		} else {
			ntylog(" tcp send success : %d\n", ret);
		}
		return ret;
	}
	return NTY_RESULT_FAILED;
}


#endif


void* ntyUdpServerInstance(void) {
	if (pUdpServer == NULL) {
		void *pServer = New(pNtyUdpServer);
		if ((unsigned long)NULL != cmpxchg((void*)(&pUdpServer), (unsigned long)NULL, (unsigned long)pServer, WORD_WIDTH)) {
			Delete(pServer);
		}
	}
	return pUdpServer;
}

