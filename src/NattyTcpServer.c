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


#include "NattyTcpServer.h"
#include "NattyThreadPool.h"
#include "NattyFilter.h"

#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ev.h>


static int ntySetNonblock(int fd) {
	int flags;

	flags = fcntl(fd, F_GETFL);
	if (flags < 0) return flags;
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0) return -1;
	return 0;
}

static void ntyTcpServerJob(Job *job) {
	RequestPacket *req = (RequestPacket*)job->user_data;
	void* pFilter = ntyProtocolFilterInstance();
	ntyProtocolFilterProcess(pFilter, req->buffer, req->length, req->client);

	freeRequestPacket(req);
	free(job);
}


void ntyOnReadEvent(struct ev_loop *loop, struct ev_io *watcher, int revents) {
	U8 buffer[RECV_BUFFER_SIZE];
	ssize_t rLen = 0;

	if (EV_ERROR & revents) {
		ntylog("error event in read");
		return ;
	}

	rLen = recv(watcher->fd, buffer, RECV_BUFFER_SIZE, 0);
	if (rLen < 0) {
		ntylog("read error\n");
	} else if (rLen == 0) {
		struct sockaddr_in client_addr;
		int nSize = sizeof(struct sockaddr_in);
		getpeername(watcher->fd,(struct sockaddr*)&client_addr, &nSize); 
		
		printf(" %d.%d.%d.%d:%d --> Client Disconnected\n", *(unsigned char*)(&client_addr.sin_addr.s_addr), *((unsigned char*)(&client_addr.sin_addr.s_addr)+1),													
				*((unsigned char*)(&client_addr.sin_addr.s_addr)+2), *((unsigned char*)(&client_addr.sin_addr.s_addr)+3),													
				client_addr.sin_port);	
		//release client
		
		ev_io_stop(loop, watcher);
		free(watcher);
	} else {
		struct sockaddr_in client_addr;
		int nSize = sizeof(struct sockaddr_in);	 
		void* pThreadPool = ntyThreadPoolInstance();
		
		RequestPacket *req = (RequestPacket*)allocRequestPacket();
		if (req == NULL) {
			freeRequestPacket(req);
			return ;
		}
			
		getpeername(watcher->fd,(struct sockaddr*)&req->client->addr, &nSize);
		printf(" %d.%d.%d.%d:%d, length:%ld --> %x, id:%lld\n", *(unsigned char*)(&req->client->addr.sin_addr.s_addr), *((unsigned char*)(&req->client->addr.sin_addr.s_addr)+1),													
				*((unsigned char*)(&req->client->addr.sin_addr.s_addr)+2), *((unsigned char*)(&req->client->addr.sin_addr.s_addr)+3),													
				req->client->addr.sin_port, rLen, buffer[NTY_PROTO_TYPE_IDX], *(C_DEVID*)(&buffer[NTY_PROTO_DEVID_IDX]));	

		req->client->sockfd = watcher->fd;
		req->client->clientType = PROTO_TYPE_TCP;
		req->length = (U16)rLen;
		req->buffer = (U8*)malloc(rLen);
		if (req->buffer == NULL) {
			ntylog("malloc Recv Buffer failed\n");

			free(req->client);
			free(req);
			return;
		}	

		memcpy(req->buffer, buffer, rLen);
		Job *job = (Job*)malloc(sizeof(*job));
		if (job == NULL) {
			ntylog("malloc Job failed\n");
			freeRequestPacket(req);
			return ;
		}
		job->job_function  = ntyTcpServerJob;
		job->user_data = req;

		ntyThreadPoolPush(pThreadPool, job);
	}

	return ;
}

void ntyOnAcceptEvent(struct ev_loop *loop, struct ev_io *watcher, int revents){
	int client_fd;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	struct ev_io *ev_client = (struct ev_io *)malloc(sizeof(struct ev_io));
	if (EV_ERROR & revents) {
		ntylog("error event in accept\n");
		return ;
	}
	
	client_fd = accept(watcher->fd, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd == -1) {
		ntylog("natty accept failed\n");
		return ;
	}
	if (ntySetNonblock(client_fd) < 0) {
		ntylog("failed to set client socket to non-blocking\n");
		close(client_fd);
		return ;
	}

	ntylog(" %d.%d.%d.%d:%d --> New Client Connected \n", 
		*(unsigned char*)(&client_addr.sin_addr.s_addr), *((unsigned char*)(&client_addr.sin_addr.s_addr)+1),													
		*((unsigned char*)(&client_addr.sin_addr.s_addr)+2), *((unsigned char*)(&client_addr.sin_addr.s_addr)+3),													
		client_addr.sin_port);

	ev_io_init(ev_client, ntyOnReadEvent, client_fd, EV_READ);
	ev_io_start(loop, ev_client);

	return ;
}

void *ntyTcpServerInitialize(TcpServer *server) {
	server->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (server->sockfd < 0) {
		ntylog("natty tcp server listen failed\n");
		return NULL;
	}

	memset(&server->addr, 0, sizeof(server->addr));
	server->addr.sin_family = AF_INET;
	server->addr.sin_addr.s_addr = INADDR_ANY;
	server->addr.sin_port = htons(NATTY_TCP_SERVER_PORT);
	if (bind(server->sockfd, (struct sockaddr*)&server->addr, sizeof(server->addr)) < 0) {
		ntylog("natty tcp server bind failed\n");
		return NULL;
	}

	return server;
}

void *ntyTcpServerRelease(TcpServer *server) {
	memset(&server->addr, 0, sizeof(server->addr));
	close(server->sockfd);
	return server;
}

int ntyTcpServerProcess(const void *_self) {
	const TcpServer *server = _self;
	int reuseaddr_on;
	struct ev_io socket_accept;
	struct ev_loop *loop = ev_default_loop(0);
	
	if (listen(server->sockfd, NATTY_CONNECTION_BACKLOG) < 0) {
		ntylog("natty tcp server listen failed\n");
		return -1;
	}

	if (ntySetNonblock(server->sockfd) < 0) {
		ntylog("natty tcp set nonblock failed\n");
		return -2;
	}

	ev_io_init(&socket_accept, ntyOnAcceptEvent, server->sockfd, EV_READ);
	ev_io_start(loop, &socket_accept);

	while (1) {
		ev_loop(loop, 0);
	}
	
	return 0;
}


void *ntyTcpServerCtor(void *self, va_list *params) {
	return ntyTcpServerInitialize(self);
}

void *ntyTcpServerDtor(void *self) {
	return ntyTcpServerRelease(self);
}

static const TcpServerHandle ntyTcpServer = {
	sizeof(UdpServer),
	ntyTcpServerCtor,
	ntyTcpServerDtor,
	ntyTcpServerProcess,
};

static const void *pNtyTcpServer = &ntyTcpServer;
static void *pTcpServer = NULL;

int ntyTcpServerRun(const void *arg) {
	const TcpServerHandle * const *pServerConf = arg;
	
	if (arg && (*pServerConf)->process && (*pServerConf)) {
		(*pServerConf)->process(arg);
	}
	return 0;
}

void* ntyTcpServerInstance(void) {
	if (pTcpServer == NULL) {
		void *pServer = New(pNtyTcpServer);
		if ((unsigned long)NULL != cmpxchg((void*)(&pTcpServer), (unsigned long)NULL, (unsigned long)pServer, WORD_WIDTH)) {
			Delete(pServer);
		}
	}
	return pTcpServer;
}




