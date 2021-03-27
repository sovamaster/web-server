/*!
 * Copyright (c) 2015-2018, Andrei Chudinovich
 * Released under MIT license
 * Date: 03.09.2018
 */


#ifndef SVCONFIG_HPP
#define SVCONFIG_HPP

#pragma once

#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <ctime>
#include <mutex>

#include <jsoncons/json.hpp>
#include <utils/util.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib,"ws2_32")
#pragma comment(lib, "iphlpapi.lib")
#else
#define SOCKET int
#endif



namespace jsc = jsoncons;
namespace config
{
	extern std::string PROGRAMM_VERSION;
	extern std::string SERVER_NAME;
	extern int LOG_DETALES;
	extern int SERVER_PORT;
	extern int SERVER_AUTH;
	extern std::string ALLOW_CORS;
	extern std::string CORS_HEADERS;
	extern std::string HTTP_SERVER_HEADER;
	extern std::mutex user_mutex;

	extern std::vector<SOCKET> WEBSOCKET_LIST;
	extern std::mutex websocket_mutex;
	extern jsc::json OAS_SHEME;
	extern int WEBSOCKET_PORT;


	struct sc_type {
		std::string server = "Server     ";
		std::string api = "Api        ";
	};

	struct http_user {
		std::string name;
		std::string login;
		std::string password;
		std::string hash;
		std::vector<std::string> permits;
	};

	extern std::vector<http_user> USER_LIST;
	extern sc_type SOURCE_TYPE;
	

	bool init();
}

#endif