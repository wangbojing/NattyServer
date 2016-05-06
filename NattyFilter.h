


#ifndef __NATTY_FILTER_H__
#define __NATTY_FILTER_H__

#include "NattyUdpServer.h"
#include "NattyProtocol.h"
#include "NattyAbstractClass.h"

typedef struct {
	size_t size;
	void* (*ctor)(void *_self, va_list *params);
	void* (*dtor)(void *_self);
	void (*setSuccessor)(void *_self, void *succ);
	void* (*getSuccessor)(const void *_self);
	void (*handleRequest)(const void *_self, unsigned char type,const void* obj);
} ProtocolFilter;

typedef struct {
	const void *_;
	void *succ;
} Packet;

void* ntyProtocolFilterInit(void);
void ntyProtocolFilterProcess(void *_filter, unsigned char *buffer, const void *obj);
void ntyProtocolFilterRelease(void *_filter);




#endif


