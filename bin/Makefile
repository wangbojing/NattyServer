

SERVER_OBJ = NattyAbstractClass.o NattyFilter.o NattyRBTree.o NattyServer.o \
	NattyUdpServer.o NattyTcpServer.o NattyUtils.o NattyThreadPool.o NattyWorkQueue.o \
	NattySession.o NattyDaveMQ.o NattyHttpCurl.o NattyLetter.o NattyLog.o NattyHash.o \
	NattyHBD.o NattyDBOperator.o NattyNodeAgent.o NattyMulticast.o NattySignal.o NattyTimer.o	\
	NattyBPlusTree.o NattyVector.o NattyJson.o parson.o NattyServAction.o NattyMessage.o \
	NattySqlHandle.o NattyPush.o NattyConnectionPool.o 

CLIENT_OBJ = NattyNetwork.o NattyRBTree.o NattyAbstractClass.o NattyTimer.o NattyProtoClient.o \
	NattyClientMain.o NattyUtils.o NattyLetter.o NattyVector.o 

SERVER_BIN = NattyCloud
CLIENT_BIN = NattyClient

all : $(SERVER_BIN) $(CLIENT_BIN)

$(SERVER_BIN) : $(SERVER_OBJ)
	$(CC) -o $(ROOT_DIR)/$(SERVER_BIN) $(SERVER_OBJ) $(FLAG)

$(CLIENT_BIN) : $(CLIENT_OBJ)
	$(CC) -o $(ROOT_DIR)/$(CLIENT_BIN) $(CLIENT_OBJ) $(FLAG)

#bin : 
#	$(CC) -o $(ROOT_DIR)/$(SERVER_BIN) $(SERVER_OBJ) $(FLAG)
#	$(CC) -o $(ROOT_DIR)/$(CLIENT_BIN) $(CLIENT_OBJ) $(FLAG)

