
UBDIRS=$(shell ls -l | grep ^d | awk '{print $$9}')

CUR_SOURCE=${wildcard *.c}

CUR_OBJS=${patsubst %.c, %.o, $(CUR_SOURCE)}

objects = NattyAbstractClass.o NattyFilter.o NattyRBTree.o NattyServer.o \
	NattyUdpServer.o NattyUtils.o NattyThreadPool.o NattyWorkQueue.o \
	NattySession.o

client_obj = NattyUdpClient.o NattyRBTree.o NattyAbstractClass.o NattyTimer.o

#OBJS = $(shell pwd)/$(objects)
#CLIENT_OBJS = $(shell pwd)/$(client_obj)

export objects client_obj

all:$(SUBDIRS) $(CUR_OBJS)

$(SUBDIRS):ECHO
	make -C $@

$(CUR_OBJS):%.o:%.c
	$(CC) -c $^ -o $(BIN_DIR)/$@ $(FLAG)

ECHO:
	@echo $(SUBDIRS)

