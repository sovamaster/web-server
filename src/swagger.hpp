/*!
 * Copyright (c) 2015-2018, Andrei Chudinovich
 * Released under MIT license
 * Date: 03.09.2018
 */


#ifndef SWAGGER_H
#define SWAGGER_H

#pragma once


#include <utils/util.h>
#include <jsoncons/json.hpp>
#include "config.hpp"
#include <map>



namespace jsc = jsoncons;
namespace swagger
{
	struct parameter {
		std::string name;
		std::string type;
		std::string value;
	};
	
	struct result {
		std::string operationId = "";
		std::vector<parameter> parameters;
	};

	result getOperationID(std::string&, std::string&, std::map<std::string, std::string>);

	bool loadOasSheme();

}
#endif