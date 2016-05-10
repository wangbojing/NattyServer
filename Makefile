


PWD = $(shell pwd)
#CC := gcc
FLAG = -lpthread


objects = NattyAbstractClass.o NattyFilter.o NattyRBTree.o NattyServer.o \
	NattyUdpServer.o NattyUtils.o NattyThreadPool.o NattyWorkQueue.o \

Natty : $(objects)
	cc -o Natty $(objects) $(FLAG)


	
NattyFilter.o : NattyFilter.c NattyFilter.h

NattyRBTree.o : NattyRBTree.c NattyRBTree.h

NattyServer.o : NattyServer.c

NattyUdpServer.o : NattyUdpServer.c NattyUdpServer.h

NattyUtils.o : NattyUtils.c NattyUtils.h

NattyAbstractClass.o : NattyAbstractClass.c NattyAbstractClass.h

NattyThreadPool.o : NattyThreadPool.c NattyThreadPool.h

NattyWorkQueue.o : NattyWorkQueue.c NattyWorkQueue.h

.PHONY : clean

clean:
	rm -rf Natty $(objects)



