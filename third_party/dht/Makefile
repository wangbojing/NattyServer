
SUBDIRS=$(shell ls -l | grep ^d | awk '{print $$9}')

CUR_SOURCE=${wildcard *.c}

CUR_OBJS=${patsubst %.c, %.o, $(CUR_SOURCE)}

all:$(SUBDIRS) $(CUR_OBJS)

$(SUBDIRS):ECHO

$(CUR_OBJS):%.o:%.c
	$(CC) -c $^ -o $(BIN_DIR)/$@ $(FLAG)

ECHO:
	@echo $(SUBDIRS)



