/*!
 * Copyright (c) 2015-2018, Andrei Chudinovich
 * Released under MIT license
 * Date: 03.09.2018
 */


#include <stdio.h>
#include <thread>
#include "config.hpp"
#include "net/httpServer.hpp"
#include "net/wsServer.hpp"


int main(int argc, char* argv[])
{
	
	if (!config::init())
	{
		return -1;
	}

	if (config::WEBSOCKET_PORT != 0)
	{
		std::thread wsServerThread(ws::runWebSocketServer);
		wsServerThread.detach();
	}

	
	if (config::SERVER_PORT != 0)
	{
		net::runTCPServer();
	}


	util::writeLogLine(config::SERVER_NAME + " server stop", util::message_type.message, config::SOURCE_TYPE.server);
	std::this_thread::sleep_for(std::chrono::seconds(2));
	

	return 0;
}