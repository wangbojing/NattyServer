
UBDIRS=$(shell ls -l | grep ^d | awk '{print $$9}')

CUR_SOURCE=${wildcard *.c}

CUR_OBJS=${patsubst %.c, %.o, $(CUR_SOURCE)}


#OBJS = $(shell pwd)/$(objects)
#CLIENT_OBJS = $(shell pwd)/$(client_obj)

#export objects client_obj

all:$(SUBDIRS) $(CUR_OBJS)

$(SUBDIRS):ECHO
	make -C $@

$(CUR_OBJS):%.o:%.c
	$(CC) -c $^ -o $(BIN_DIR)/$@ $(FLAG)

ECHO:
	@echo $(SUBDIRS)

