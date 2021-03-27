/*!
 * Copyright (c) 2015-2018, Andrei Chudinovich
 * Released under MIT license
 * Date: 03.09.2018
 */


#include "swagger.hpp"

swagger::result swagger::getOperationID(std::string& type, std::string& paht, std::map<std::string, std::string> params)
{
	swagger::result res;
	res.operationId = "test";

	return res;

}

bool swagger::loadOasSheme()
{
	bool res = false;
	std::string file_path = "api-docs/oas-3.0.0.json";
	util::BinData content = util::getFileContent(file_path, true);

	if (content.size() != 0)
	{
		std::string jdata = content.toString();
		try
		{
			jsc::strict_parse_error_handler err_handler;
			config::OAS_SHEME = jsc::json::parse(jdata, err_handler);
			res = true;
		}
		catch (const jsc::parse_error& e)
		{
			util::writeLogLine("Receive OAS3 sheme: Json sheme incorrect", util::message_type.error, "swagger    ");
		}
	}
	else
	{
		util::writeLogLine("Receive OAS3 sheme: Json sheme file not found", util::message_type.error, "swagger    ");
	}
	return res;
}
