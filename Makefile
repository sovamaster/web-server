.PHONY: server

CPP = src/app.cpp 
CPP += src/libs/utils/util.cpp 
CPP += src/config.cpp
CPP += src/swagger.cpp
CPP += src/net/wsServer.cpp
CPP += src/net/httpServer.cpp

INCLUDE = -I/usr/local/include -Isrc/libs
LIB = -L/usr/local/lib -lssl -lcrypto
server:
	g++ -ansi -pedantic -Wall -O0 -g -pthread -std=c++11 -o web-server $(CPP) $(INCLUDE) $(LIB)