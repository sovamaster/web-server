/*!
 * Copyright (c) 2015-2018, Andrei Chudinovich
 * Released under MIT license
 * Date: 03.09.2018
 */


#ifndef HTTP_WSSERWER_HPP
#define HTTP_WSSERWER_HPP
#pragma once

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

#define RECVWSDATASIZE 4200

namespace ws
{
	void sendWebSockMessage(SOCKET&, std::string);
	
	void sendMessageAllClientsWS(std::string&);

	void runWebSocketServer();
}
#endif