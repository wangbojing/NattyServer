


PWD = $(shell pwd)
#CC := gcc
FLAG = -lpthread


objects = NattyAbstractClass.o NattyFilter.o NattyRBTree.o NattyServer.o \
	NattyUdpServer.o NattyUtils.o NattyThreadPool.o NattyWorkQueue.o \
	NattySession.o

client_obj = NattyUdpClient.o NattyRBTree.o NattyAbstractClass.o NattyTimer.o

Natty : $(objects)
	cc -o Natty $(objects) $(FLAG)

client : $(client_obj)
	cc -o NattyUdpClient $(client_obj) $(FLAG)

	
NattyFilter.o : NattyFilter.c NattyFilter.h

NattyRBTree.o : NattyRBTree.c NattyRBTree.h

NattyServer.o : NattyServer.c

NattyUdpServer.o : NattyUdpServer.c NattyUdpServer.h

NattyUtils.o : NattyUtils.c NattyUtils.h

NattyAbstractClass.o : NattyAbstractClass.c NattyAbstractClass.h

NattyThreadPool.o : NattyThreadPool.c NattyThreadPool.h

NattyWorkQueue.o : NattyWorkQueue.c NattyWorkQueue.h

NattySession.o : NattySession.c NattySession.h

NattyUdpClient.o : NattyUdpClient.c NattyUdpClient.h

NattyTimer.o : NattyTimer.c NattyTimer.h

.PHONY : clean

clean:
	rm -rf Natty $(objects) NattyUdpClient.o



