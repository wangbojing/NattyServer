


#ifndef __NATTY_UDP_CLIENT_H__
#define __NATTY_UDP_CLIENT_H__

#include "NattyProtocol.h"
#include "NattyRBTree.h"

typedef long long U64;
typedef unsigned int U32;
typedef unsigned short U16;
typedef unsigned char U8;
typedef long long C_DEVID;


typedef struct _ThreadArg {
	int sockfd;
	C_DEVID devid;
} ThreadArg;

typedef struct _FRIENDSINFO {
	//C_DEVID devid;
	int sockfd;
	U32 addr;
	U16 port;
	U8 isP2P;
} FriendsInfo;


#define MS(x)		(x*1000)

#define SIGNAL_LOGIN_REQ			0x00000001
#define SIGNAL_LOGIN_ACK			0x00000003

#define SIGNAL_HEARTBEAT_REQ		0x00000004
#define SIGNAL_HEARTBEAT_ACK		0x0000000C

#define SIGNAL_LOGOUT_REQ			0x00000010
#define SIGNAL_LOGOUT_ACK			0x00000030

#define SIGNAL_P2PADDR_REQ			0x00000040
#define SIGNAL_P2PADDR_ACK			0x000000C0

#define SIGNAL_P2PCONNECT_REQ		0x00000100
#define SIGNAL_P2PCONNECT_ACK		0x00000300

#define SIGNAL_DATAPACKET_REQ		0x00000400
#define SIGNAL_DATAPACKET_ACK		0x00000C00

#define SIGNAL_P2PDATAPACKET_REQ	0x00001000
#define SIGNAL_P2PDATAPACKET_ACK	0x00003000


enum {
	LEVEL_LOGIN = 0x00,
	LEVEL_HEARTBEART = 0x01,
	LEVEL_P2PCONNECT = 0x02,
	LEVEL_P2PCONNECT_ACK = 0x03,
	LEVEL_LOGOUT = 0x04,
	LEVEL_P2PADDR = 0x05,
	LEVEL_P2PDATAPACKET = 0x06,
	LEVEL_DATAPACKET = 0x07,
} LEVEL;

#endif



