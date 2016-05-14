


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
#include <sys/time.h>
#include <signal.h>

#include "NattyUdpClient.h"



#define SERVER_NAME		"127.0.0.1"
#define SERVER_PORT		8888
#define RECV_BUFFER_SIZE	(1024+16)

static int count = 0;
static U32 msgAck = 0x0;
static int level = 0;
static C_DEVID devid = 0;
static C_DEVID friendId = 0;
static U32 ackNum = 0;
static int sockfd_local = 0;

void ntyMessageOnDataLost(void) {
	///////////////////////////////////////////////////////////
	
}

void ntyMessageOnAck(int signo) {
	//printf(" Get a Sigalarm, %d counts! \n", ++count);
	U32 tempAck = msgAck;
	
	if (msgAck & SIGNAL_LOGIN_ACK) {
		tempAck &= SIGNAL_LOGIN_ACK;
		if (tempAck == SIGNAL_LOGIN_ACK) {
			ntyMessageOnDataLost();
			msgAck &= (~SIGNAL_LOGIN_ACK);
		} else {
			msgAck = (++tempAck);
			level = LEVEL_P2PCONNECT; //resend login
		}
	} else if (msgAck & SIGNAL_HEARTBEAT_ACK) {

	} else if (msgAck & SIGNAL_LOGOUT_ACK) {

	} else if (msgAck & SIGNAL_P2PADDR_ACK) {

	} else if (msgAck & SIGNAL_P2PCONNECT_ACK) {
		tempAck &= SIGNAL_P2PCONNECT_ACK;
		if (tempAck == SIGNAL_P2PCONNECT_ACK) {
			ntyMessageOnDataLost();
			msgAck &= (~SIGNAL_P2PCONNECT_ACK);
		} else {
			tempAck = tempAck >> 8;
			++tempAck;
			tempAck = tempAck << 8;
			msgAck = tempAck;
			
			level = LEVEL_DATAPACKET; //resend login
			//printf(" SIGNAL_P2PCONNECT_ACK\n");
		}
	} else if (msgAck & SIGNAL_DATAPACKET_ACK) {
		
	} else if (msgAck & SIGNAL_P2PDATAPACKET_ACK) {
		
	}
}

void error(char *msg) {    
	perror(msg);    
	exit(0);
}


void ntyInitTimer(void) {
	struct itimerval tick;

	signal(SIGALRM, ntyMessageOnAck);
	memset(&tick, 0, sizeof(tick));

	tick.it_value.tv_sec = 0;
	tick.it_value.tv_usec = MS(250);

	tick.it_interval.tv_sec = 0;
	tick.it_interval.tv_usec = MS(250);

	if (setitimer(ITIMER_REAL, &tick, NULL) < 0) {
		printf("Set timer failed!\n");
	}
}


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

struct sockaddr_in serveraddr;
int portno;
static U8 heartbeartRun = 0;


void *heartbeatThread(void *arg) {
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

		sleep(20);	
	}
}

void* recvThread(void *arg) {
	ThreadArg threadArg = *((ThreadArg*)arg);
	int devid = threadArg.devid;
	int sockfd = threadArg.sockfd;
	int ret, n, err;

	U8 buf[RECV_BUFFER_SIZE] = {0};
	struct sockaddr_in addr;
	int clientLen = sizeof(struct sockaddr_in);
	pthread_t heartbeatThread_id;

	struct pollfd fds;
	fds.fd = sockfd;
	fds.events = POLLIN;

	while (1) {
		ret = poll(&fds, 1, 5);
		if (ret) {
			bzero(buf, RECV_BUFFER_SIZE);
			n = recvfrom(sockfd, buf, RECV_BUFFER_SIZE, 0, (struct sockaddr *) &addr, &clientLen);
			printf("%d.%d.%d.%d:%d size:%d --> %x\n", *(unsigned char*)(&addr.sin_addr.s_addr), *((unsigned char*)(&addr.sin_addr.s_addr)+1),	
				*((unsigned char*)(&addr.sin_addr.s_addr)+2), *((unsigned char*)(&addr.sin_addr.s_addr)+3),	 addr.sin_port,
				n, buf[NTY_PROTO_TYPE_IDX]);
			if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_LOGIN_ACK) {
				int i = 0;
				msgAck &= (~SIGNAL_LOGIN_ACK);
				level = LEVEL_P2PCONNECT;
				
				int count = *(U16*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDS_COUNT_IDX]);
				void *pTree = ntyRBTreeInstance();
				for (i = 0;i < count;i ++) {
					C_DEVID friendId = *(C_DEVID*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_DEVID_IDX(i)]);
					FriendsInfo *pFriend = (FriendsInfo*)malloc(sizeof(FriendsInfo));
					assert(pFriend);
					pFriend->sockfd = sockfd;
					pFriend->addr = *(U32*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_ADDR_IDX(i)]);
					pFriend->port = *(U16*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_PORT_IDX(i)]);
					ntyRBTreeInterfaceInsert(pTree, friendId, pFriend);
				}
			
				printf(" Setup heartbeat Thread\n");	
				err = pthread_create(&heartbeatThread_id, NULL, heartbeatThread, arg);				
				if (err != 0) {					
					printf(" can't create thread:%s\n", strerror(err));	
					exit(0);				
				}
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_CONNECT_REQ) {
				msgAck &= (~SIGNAL_P2PCONNECT_ACK);
				level = LEVEL_P2PCONNECT_ACK;

				friendId = *(C_DEVID*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]);	
				void *pTree = ntyRBTreeInstance();	

				FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, friendId);
				if (pFriend != NULL) {
					pFriend->isP2P = 1;
					pFriend->port = addr.sin_port;
					pFriend->addr = addr.sin_addr.s_addr;

					printf(" P2P client:%lld request connect\n", friendId);
				}
			} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_CONNECT_ACK) {
				level = LEVEL_P2PDATAPACKET;
				friendId = *(C_DEVID*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]);	
				printf(" P2P client %lld connect Success\n", friendId);
			}

		}
	}
}

void sendLoginPacket(int sockfd, int devid) {	
	int len, n;	
	U8 buf[RECV_BUFFER_SIZE] = {0};	
	
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_LOGIN_REQ;	
	*(C_DEVID*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]) = (C_DEVID) devid;	
	*(U32*)(&buf[NTY_PROTO_LOGIN_REQ_ACKNUM_IDX]) = (U32) (++ackNum);
	*(U32*)(&buf[NTY_PROTO_LOGIN_REQ_CRC_IDX]) = ntyGenCrcValue(buf, NTY_PROTO_LOGIN_REQ_CRC_IDX);	//serverlen = sizeof(serveraddr);	
	//printf("crc : %x\n", *(U32*)(&buf[NTY_PROTO_LOGIN_REQ_CRC_IDX]));
	len = NTY_PROTO_LOGIN_REQ_CRC_IDX+sizeof(U32);				

	n = sendto(sockfd, buf, len, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	msgAck |= SIGNAL_LOGIN_REQ;
}

int sendP2PConnectReq(void* fTree, C_DEVID id) {
	//void *pRBTree = ntyRBTreeInstance();
	int len, n;	
	U8 buf[RECV_BUFFER_SIZE] = {0};
	struct sockaddr_in friendaddr;

	FriendsInfo *client = ntyRBTreeInterfaceSearch(fTree, id);
	if (client == NULL || client->isP2P) return ;

	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2P_CONNECT_REQ;
	*(C_DEVID*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]) = (C_DEVID) devid;
	*(U32*)(&buf[NTY_PROTO_LOGIN_REQ_ACKNUM_IDX]) = (U32) (++ackNum);
	*(U32*)(&buf[NTY_PROTO_LOGIN_REQ_CRC_IDX]) = ntyGenCrcValue(buf, NTY_PROTO_LOGIN_REQ_CRC_IDX);	
	//serverlen = sizeof(clientaddr);			
	len = NTY_PROTO_LOGIN_REQ_CRC_IDX+sizeof(U32);

	friendaddr.sin_addr.s_addr = client->addr;				
	friendaddr.sin_port = client->port;
	
	printf("sendP2PConnectReq :%lld \n", id);
	printf("%d.%d.%d.%d:%d\n", *(unsigned char*)(&friendaddr.sin_addr.s_addr), *((unsigned char*)(&friendaddr.sin_addr.s_addr)+1),													
				*((unsigned char*)(&friendaddr.sin_addr.s_addr)+2), *((unsigned char*)(&friendaddr.sin_addr.s_addr)+3),													
				friendaddr.sin_port);
	msgAck |= SIGNAL_P2PCONNECT_REQ;
	if (client->addr == 0 || client->port == 0) {
		client->isP2P = 0;
		return -1;
	}
	n = sendto(sockfd_local, buf, len, 0, (struct sockaddr *)&friendaddr, sizeof(friendaddr));
	
}

void sendP2PConnectAck(int sockfd, int devid) {
	int len, n;	
	U8 buf[RECV_BUFFER_SIZE] = {0};	
	
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2P_CONNECT_ACK;	
	*(C_DEVID*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]) = (C_DEVID) devid;	
	*(U32*)(&buf[NTY_PROTO_LOGIN_REQ_ACKNUM_IDX]) = (U32) (++ackNum);
	*(U32*)(&buf[NTY_PROTO_LOGIN_REQ_CRC_IDX]) = ntyGenCrcValue(buf, NTY_PROTO_LOGIN_REQ_CRC_IDX);	//serverlen = sizeof(serveraddr);	
	len = NTY_PROTO_LOGIN_REQ_CRC_IDX+sizeof(U32);				

	n = sendto(sockfd, buf, len, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	//msgAck |= SIGNAL_LOGIN_REQ;
}

void sendP2PDataPacketReq(int sockfd, int devid, const FriendsInfo *pFriend, U8 *buf, int length) {
	//NTY_PROTO_P2PDATAPACKET_REQ
	int n;
	//void *pTree = ntyRBTreeInstance();
	//FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, devid);
	
	struct sockaddr_in friendaddr;
	friendaddr.sin_addr.s_addr = pFriend->addr;				
	friendaddr.sin_port = pFriend->port;

	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2PDATAPACKET_REQ;
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEVID_IDX]) = (C_DEVID) devid;
	*(U32*)(&buf[NTY_PROTO_DATAPACKET_ACKNUM_IDX]) = (U32) (++ackNum);

	buf[NTY_PROTO_DATAPACKET_CONTENT_COUNT_IDX(0)] = (U16)length;
	length += NTY_PROTO_DATAPACKET_CONTENT_COUNT_IDX(0);
	*(U32*)(&buf[length]) = ntyGenCrcValue(buf, length);
	length += sizeof(U32);

	n = sendto(sockfd, buf, length, 0, (struct sockaddr *)&friendaddr, sizeof(struct sockaddr_in));
}

void sendProxyDataPacketReq(int sockfd, int devid, U8 *buf, int len) {
	int n = 0;
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_DATAPACKET_REQ;
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEVID_IDX]) = (C_DEVID) devid;
	*(U32*)(&buf[NTY_PROTO_DATAPACKET_ACKNUM_IDX]) = (U32) (++ackNum);

#if 1
	*(U32*)(&buf[NTY_PROTO_DATAPACKET_RECE_COUNT_IDX]) = (U32) 0;
#endif
	*(U16*)(&buf[NTY_PROTO_DATAPACKET_CONTENT_COUNT_IDX(buf[NTY_PROTO_DATAPACKET_RECE_COUNT_IDX])]) = (U16)len;
	len += 2;	    	
	len += NTY_PROTO_DATAPACKET_CONTENT_IDX(buf[NTY_PROTO_DATAPACKET_RECE_COUNT_IDX]);

	*(U32*)(&buf[len]) = ntyGenCrcValue(buf, len);	    	
	len += sizeof(U32);

	n = sendto(sockfd, buf, len, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
}

int main () {
	int sockfd, err;
	pthread_t recvThread_id, heartbeartThread_id;
	struct sockaddr_in addr;	
	struct hostent *server;
	
	char *hostname = SERVER_NAME;
	portno = SERVER_PORT;
	ThreadArg threadArg = {0};
	int devid = 0;
	U8 buf[RECV_BUFFER_SIZE];

	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		error(" ERROR opening socket");
	}
	server = gethostbyname(hostname);    
	if (server == NULL) {        
		fprintf(stderr,"ERROR, no such host as %s\n", hostname);  
		exit(0);    
	}
	
#if 1        
	printf(" Press DevId <1 or 2>: ");    
	scanf("%d", &devid);
#else	
	srand(time(NULL));  	
	devid = rand() % 5000;
#endif

	sockfd_local = sockfd;
	threadArg.sockfd = sockfd;
	threadArg.devid = devid;
	err = pthread_create(&recvThread_id, NULL, recvThread, &threadArg);	
	if (err != 0) {		
		printf(" can't create thread:%s\n", strerror(err));		
		exit(0);	
	}

	ntyInitTimer();
	ntyGenCrcTable();
	/* build the server's Internet address */    
	bzero((char *) &serveraddr, sizeof(serveraddr));    
	serveraddr.sin_family = AF_INET;    
	bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);    
	serveraddr.sin_port = htons(portno);

	while (1) {
		bzero(buf, RECV_BUFFER_SIZE);
		buf[0] = 'A';
		if (level == LEVEL_LOGIN) {
			sendLoginPacket(sockfd, devid);
		} else if (level == LEVEL_HEARTBEART) {

		} else if (level == LEVEL_P2PCONNECT) {
			void *pTree = ntyRBTreeInstance();
			ntyFriendsTreeTraversal(pTree, sendP2PConnectReq);			
		} else if (level == LEVEL_P2PCONNECT_ACK) {
			sendP2PConnectAck(sockfd, devid);
		} else if (level == LEVEL_P2PDATAPACKET) {
			printf("P2P client --> Please enter msg: ");	    	
			fgets(buf+NTY_PROTO_DATAPACKET_CONTENT_IDX(0), RECV_BUFFER_SIZE-NTY_PROTO_DATAPACKET_CONTENT_IDX(0), stdin);	
#if 1
			void *pTree = ntyRBTreeInstance();
			const FriendsInfo *pFriend = (const FriendsInfo *)ntyFriendsTreeGetFristNode(pTree);
			int len = strlen(buf+NTY_PROTO_DATAPACKET_CONTENT_IDX(0));
			sendP2PDataPacketReq(sockfd, devid, pFriend, buf, len);
#endif
		} else if (level == LEVEL_DATAPACKET) {
			U8 *tempBuf;
			//tempBuf = &buf[NTY_PROTO_DATAPACKET_CONTENT_IDX(buf[NTY_PROTO_DATAPACKET_RECE_COUNT_IDX])];
			tempBuf = &buf[NTY_PROTO_DATAPACKET_CONTENT_IDX(0)];
			printf("Proxy Please enter msg: ");	
			fgets(tempBuf, RECV_BUFFER_SIZE, stdin);
			int len = strlen(tempBuf);

			sendProxyDataPacketReq(sockfd, devid, buf, len);
		}

		sleep(2);
	}
	
	return 0;
}


