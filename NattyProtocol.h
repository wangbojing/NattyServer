




#ifndef __NATTY_PROTOCOL_H__
#define __NATTY_PROTOCOL_H__

/* ** **** ********  ****************  Length  ****************  ******** **** ** */
#define NTY_HEARTBEAT_ACK_LENGTH		5
#define NTY_DEVID_LENGTH				4
#define NTY_ACKNUM_LENGTH				4


/* ** **** ********  ****************  Index  ****************  ******** **** ** */
#define NTY_PROTO_TYPE_IDX			0	
#define NTY_PROTO_DEVID_IDX			1	
#define NTY_PROTO_ACKNUM_IDX		(NTY_PROTO_DEVID_IDX+NTY_DEVID_LENGTH)




/* ** **** ********  ****************  Header  ****************  ******** **** ** */
#define NTY_PROTO_LOGIN_REQ			0x01
#define NTY_PROTO_LOGIN_ACK			0x81

#define NTY_PROTO_HEARTBEAT_REQ		0x02 //'H'
#define NTY_PROTO_HEARTBEAT_ACK		0x82

#define NTY_PROTO_LOGOUT_REQ		0x03
#define NTY_PROTO_LOGOUT_ACK		0x83




#define NTY_PROTO_P2P_ADDR_REQ			0x11
#define NTY_PROTO_P2P_ADDR_ACK			0x91

#define NTY_PROTO_P2P_NOTIFY_REQ			0x12
#define NTY_PROTO_P2P_NOTIFY_ACK			0x92




#endif





