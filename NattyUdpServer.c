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
 ****		*****
   ***	  *
   ***        * 					    *			    *
   * ** 	  * 					    *			    *
   * ** 	  * 					    *			    *
   *  **	  * 					   **			   **
   *  **	  * 					  ***			  ***
   *   **	  * 	    ******	          ***********	   ***********	    *****         *****
   *   **	  * 	  **	    **		   **			   **			**		 **
   *	  **	  *    **		**		   **			   **			 **		 *
   *	  **	  *    **		**		   **			   **			  *		*
   *	   **   *    **		**		   **			   **			   ** 	*
   *	   **   * 		     ***		   **			   **			    *       *
   *	    **  * 	      ***** **		   **			   **			    **     *
   *	    **  * 	  *** 	**		   **			   **			    **     *
   *	     ** *    **		**		   **			   **			     *    *
   *	     ** *   **		**		   **			   **			     **  *
   *		***   **		**		   **			   **			       **
   *		***   **		**		   **	    * 	   **	    * 		 **
   *		 **   **		**	*	   **	    * 	   **	    * 		 **
   *		 **    **	  ****	*	    **   *		    **   *			 *
 *****		  *******	 ***		     ****		     ****			 *
														 *
														 *
													   *****
 *													   ****
 *
 *
 */



#include "NattyFilter.h"
#include "NattyUdpServer.h"

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

int ntyUdpServerProcess(const void *_self) {
	const UdpServer *self = _self;
	//struct sockaddr_in clientaddr;
	int clientlen = sizeof(struct sockaddr_in);  
	
	struct pollfd fds;
	int ret = -1, n;
	char buf[RECV_BUFFER_SIZE];
	
	void* pFilter = ntyProtocolFilterInit();
	UdpClient *pClient = (UdpClient*)malloc(sizeof(UdpClient));
	
	if (self->sockfd <= 0) {
		error("Udp Server Socket no Initial");
	}	
	
	fds.fd = self->sockfd;
	fds.events = POLLIN;
	while(1) {
		ret = poll(&fds, 1, 5);
		if (ret) { //data is comming
			bzero(buf, RECV_BUFFER_SIZE);    
			n = recvfrom(self->sockfd, buf, RECV_BUFFER_SIZE, 0, (struct sockaddr *) &pClient->addr, &clientlen);    
			printf("%d.%d.%d.%d:%d --> %s\n", *(unsigned char*)(&pClient->addr.sin_addr.s_addr), *((unsigned char*)(&pClient->addr.sin_addr.s_addr)+1),													
				*((unsigned char*)(&pClient->addr.sin_addr.s_addr)+2), *((unsigned char*)(&pClient->addr.sin_addr.s_addr)+3),													
				pClient->addr.sin_port, buf);	
			// proccess
			// i think process protocol and search client id from rb-tree
			pClient->sockfd = self->sockfd;
			
			ntyProtocolFilterProcess(pFilter, buf, n, pClient);
			//send to ack
			n = sendto(self->sockfd, buf, n, 0, (struct sockaddr *) &pClient->addr, clientlen);    
			if (n < 0)       
				error("ERROR in sendto");  
		}
	}

	free(pClient);
	ntyProtocolFilterRelease(pFilter);
	return 0;
}


static const UdpServerOpera ntyUdpServer = {
	sizeof(UdpServer),
	ntyUdpServerCtor,
	ntyUdpServerDtor,
	ntyUdpServerProcess,
};

static const void *pNtyUdpServer = &ntyUdpServer;

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
	return sendto(client->sockfd, buffer, length, 0, (struct sockaddr *)&client->addr, sizeof(struct sockaddr_in));
}

const void* ntyUdpServerInstance(void) {
	return pNtyUdpServer;
}

