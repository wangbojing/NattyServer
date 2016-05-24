

SERVER_OBJ = NattyAbstractClass.o NattyFilter.o NattyRBTree.o NattyServer.o \
	NattyUdpServer.o NattyUtils.o NattyThreadPool.o NattyWorkQueue.o \
	NattySession.o

CLIENT_OBJ = NattyUdpClient.o NattyRBTree.o NattyAbstractClass.o NattyTimer.o

SERVER_BIN = NattyServer
CLIENT_BIN = NattyClient

#$(SERVER_BIN) : $(SERVER_OBJ)
#	$(CC) -o $(SERVER_BIN) $(SERVER_OBJ) $(FLAG)

#$(CLIENT_BIN) : $(CLIENT_OBJ)
#	$(CC) -o $(CLIENT_BIN) $(CLIENT_OBJ) $(FLAG)

bin : 
	$(CC) -o $(ROOT_DIR)/$(SERVER_BIN) $(SERVER_OBJ) $(FLAG)
	$(CC) -o $(ROOT_DIR)/$(CLIENT_BIN) $(CLIENT_OBJ) $(FLAG)

