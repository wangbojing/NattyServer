


PWD = $(shell pwd)
#CC := gcc

objects = NattyAbstractClass.o NattyFilter.o NattyRBTree.o NattyServer.o \
			NattyUdpServer.o NattyUtils.o

Natty : $(objects)
	cc -o Natty $(objects) 


	
NattyFilter.o : NattyFilter.c NattyFilter.h

NattyRBTree.o : NattyRBTree.c NattyRBTree.h

NattyServer.o : NattyServer.c

NattyUdpServer.o : NattyUdpServer.c NattyUdpServer.h

NattyUtils.o : NattyUtils.c NattyUtils.h

NattyAbstractClass.o : NattyAbstractClass.c NattyAbstractClass.h


.PHONY : clean

clean:
	rm -rf Natty $(objects)



