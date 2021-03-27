/*!
 * Copyright (c) 2015-2018, Andrei Chudinovich
 * Released under MIT license
 * Date: 03.09.2018
 */


#ifndef HTTP_TEST_HPP
#define HTTP_TEST_HPP
#pragma once

#include <map>
#include <utils/util.h>
#include <jsoncons/json.hpp>
#include "../config.hpp"
#include "../net/httpClient.hpp"

namespace jsc = jsoncons;

namespace test
{
	util::BinData runGetTest(std::map<std::string, std::string> params)
	{

		util::BinData result;

		std::string test_req = "/get?message="+ params["message"];
		net::httpResponce http_res = net::execRequest("httpbin.org", "443", net::requestType.get, test_req, config::SOURCE_TYPE.api, true);

		if (http_res.status == net::responceStatus.error)
		{
			util::writeLogLine("Error from test service", util::message_type.error, config::SOURCE_TYPE.api);

			std::string erroransv = net::getErrorAnswer(400);
			result << erroransv;
			return result;
		}

		std::string http_result = http_res.data.toString();

		util::BinData header = net::getOkAnswer("application/json; charset utf-8", http_result.length());
		result << header;
		result << http_result;

		return result;
	}
	
	util::BinData runPostTest(util::BinData& body_data)
	{
		util::BinData result;
		std::string jdata = body_data.toString();
		jsc::json j_body_data = jsc::json::object();

		try
		{
			jsc::strict_parse_error_handler err_handler;
			j_body_data = jsc::json::parse(jdata, err_handler);
		}
		catch (const jsc::parse_error& e)
		{
			std::string error_string = "Incorrect body data json";
			util::writeLogLine(error_string, util::message_type.error, config::SOURCE_TYPE.api);
			std::string erroransv = net::getErrorAnswer(400, error_string);
			result << erroransv;
			return result;
		}


		std::string request_data = j_body_data.as_string();

		net::httpClient client("httpbin.org", "443", config::SOURCE_TYPE.api, true);
		if (!client.connection)
		{
			std::string error_string = "No connection to httpbin.org service";
			if (config::LOG_DETALES == 1) util::writeLogLine(error_string, util::message_type.error, config::SOURCE_TYPE.api);

			std::string header = net::getErrorAnswer(500, error_string);
			result << header;
			return result;
		}

		net::httpRequest req;
		req.type = net::requestType.post;
		req.path = "/post";
		req.content_type = "application/json";
		req.data << request_data;

		net::httpResponce http_res = client.execQuery(req);
		client.close();

		if (http_res.status == net::responceStatus.error)
		{
			if (config::LOG_DETALES == 1) util::writeLogLine("Error request to test : " + http_res.descr, util::message_type.error, config::SOURCE_TYPE.api);

			int int_res = 500;
			if (http_res.result != "") int_res = atoi(http_res.result.c_str());

			std::string header = net::getErrorAnswer(int_res, http_res.descr);
			result << header;
			return result;

		}

		util::BinData header = net::getOkAnswer("application/json; charset utf-8", http_res.data.size());
		result << header;
		result << http_res.data;

		return result;
	}

}
#endif