/*!
 * Copyright (c) 2015-2018, Andrei Chudinovich
 * Released under MIT license
 * Date: 03.09.2018
 */



#ifndef HTTP_API_HPP
#define HTTP_API_HPP
#pragma once

#include <map>
#include <utils/util.h>
#include "net/httpServer.hpp"
#include <jsoncons/json.hpp>


#include "swagger.hpp"
#include "./resources/test.hpp"

namespace api
{
	/*The request functions ------------------------------------------------------*/

	util::BinData getResponce(util::BinData total_recv_data, std::string request_type, std::string request_source_path, std::map<std::string, std::string> params)
	{
		util::BinData answer;
		swagger::result res = swagger::getOperationID(request_type, request_source_path, params);

		if (request_source_path == "/swagger/ui")
		{
			if (config::LOG_DETALES == 1) util::writeLogLine("Request swagger page...", util::message_type.message, config::SOURCE_TYPE.api);
			std::string file_path = "/swagger/index.html";
			answer = net::getFileContent(file_path);
		}
		else if ((request_source_path == "/get/test") && (request_type == net::requestType.get))
		{
			if (config::LOG_DETALES == 1) util::writeLogLine("Request get test...", util::message_type.message, config::SOURCE_TYPE.api);
			answer = test::runGetTest(params);
		}
		else if ((request_source_path == "/post/test") && (request_type == net::requestType.post))
		{
			if (config::LOG_DETALES == 1) util::writeLogLine("Request post test...", util::message_type.message, config::SOURCE_TYPE.api);
			answer = test::runPostTest(total_recv_data);
		}
		else 
		{
			answer = net::getFileContent(request_source_path);
		}

		return answer;
	}

}
#endif

