/*
 *  Author : WangBoJing , email : 1989wangbojing@163.com
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



#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define NATTY_SERVER_PORT		8880
#define BUFSIZE					1024


int ntyUdpServer(void) {
	int sockfd;
	int portno = NATTY_SERVER_PORT;
	int clientlen;
	
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;

	struct hostent *hostp;
	char buf[BUFSIZE];
	char *hostaddrp;
	char optval;
	int n;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		printf("Error open socket\n");
	}
	optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));

	bzero((char*)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)portno);

	if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
		printf("Error on binding\n");
	}

	clientlen = sizeof(clientaddr);
	while (1) {
		bzero(buf, BUFSIZE);
		n = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr*)&clientaddr, &clientlen);
		if (n < 0) {
			printf("Error in recvfrom\n");
		}
		hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
		if (hostp == NULL) {
			printf("Error on gethostbyaddr\n");
		}
		printf("server received datagram from %s\n", hostp->h_name, hostaddrp);
		printf("server received %d/%d bytes: %s\n", strlen(buf), n, buf);

		n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&clientaddr, clientlen);
		if (n < 0) {
			printf("Error in sendto\n");
		}
	}	
	return 0;
}

int main () {
	return ntyUdpServer();
}







