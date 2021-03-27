/*!
 * Copyright (c) 2015-2018, Andrei Chudinovich
 * Released under MIT license
 * Date: 03.09.2018
 */


#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <utils/util.h>



#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib,"ws2_32")
#pragma comment(lib, "iphlpapi.lib")
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <linux/hdreg.h>
#include <net/if.h>
#include <sys/ioctl.h>

#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#define SOCKET int

#define SD_SEND 1
#define SD_RECEIVE 0
#define SD_BOTH 2
#endif




namespace net
{
	const int SERVER_DATASIZE = 16 * 1024;
	const int SERVER_RECVDATASIZE = 4200;


	void runTCPServer();
	util::BinData getOkAnswer(const std::string, int);
	std::string getErrorAnswer(int);
	std::string getErrorAnswer(int, std::string);
	util::BinData getFileContent(std::string);
}
#endif


