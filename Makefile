

CC = gcc
#SUBDIRS=$(shell ls -l | grep ^d | awk '{if($$9 != "debug") print $$9}')
SUBDIRS = ssl/ src/ src/DaveMQ third_party/dht json/ #modules/
DEBUG = $(shell ls -l | grep ^d | awk '{if($$9 == "release") print $$9}')
ROOT_DIR = $(shell pwd)
BIN_DIR = $(ROOT_DIR)/bin

FLAG = -lpthread -lcurl -lzlog -lzdb -lev -ljemalloc -DUSE_JEMALLOC -DJEMALLOC_NO_DEMANGLE -O3 -I $(ROOT_DIR)/include #-Wunused-result

CUR_SOURCE = ${wildcard *.c}
CUR_OBJS = ${patsubst %.c, %.o, $(CUR_SOURCE)}

export CC OBJS_DIR BIN_DIR ROOT_DIR FLAG

#BIN=NattyServer

all : $(SUBDIRS) $(CUR_OBJS) DEBUG

$(SUBDIRS) : ECHO
	make -C $@

DEBUG : ECHO
	make -C bin

ECHO :
	@echo $(SUBDIRS)

$(CUR_OBJS) : %.o : %.c
	$(CC) -c $^ -o $(ROOT_DIR)/$(OBJS_DIR)/$@

#bin :
#	$(CC) -o $(BIN_DIR)/$(SERVER_BIN) $(BIN_DIR)/$(SERVER_OBJ) $(FLAG)
#	$(CC) -o $(BIN_DIR)/$(CLIENT_BIN) $(BIN_DIR)/$(OBJS_DIR)/$(CLIENT_OBJ) $(FLAG)

clean : 
	rm -rf $(BIN_DIR)/*.o
	rm -rf $(BIN_DIR)/Natty*
#	rm src/*.o
#	@rm -rf $(BIN_DIR)/*

