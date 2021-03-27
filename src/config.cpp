/*!
 * Copyright (c) 2015-2018, Andrei Chudinovich
 * Released under MIT license
 * Date: 03.09.2018
 */


#include "config.hpp"
namespace config
{
	std::string PROGRAMM_VERSION = "1.0";
	std::string SERVER_NAME = "web";
	int LOG_DETALES = 1;
	int SERVER_PORT = 0;
	int SERVER_AUTH = 0;
	std::string HTTP_SERVER_HEADER = "Server: "+ SERVER_NAME +"-server-" + PROGRAMM_VERSION;
	std::vector<http_user> USER_LIST;
	std::mutex user_mutex;
	sc_type SOURCE_TYPE;
	std::string ALLOW_CORS = "";
	std::string CORS_HEADERS = "";
	std::vector<SOCKET> WEBSOCKET_LIST;
	std::mutex websocket_mutex;
	jsc::json OAS_SHEME = jsc::json::object();
	int WEBSOCKET_PORT = 0;
	
	bool init()
	{
	

#ifdef WIN32
		SetConsoleOutputCP(CP_UTF8);

		WSADATA wsadata;
		WSAStartup(MAKEWORD(2, 2), &wsadata);
#endif

		FILE* dvalue = fopen("version.txt", "w");
		if (dvalue != NULL)
		{
			fputs(PROGRAMM_VERSION.c_str(), dvalue);
			fclose(dvalue);
		}

		char str[400];

		static FILE* file = fopen("web-server.ini", "r");
		if (file == NULL)
		{
			util::writeLogLine("file esb-server.ini not found", util::message_type.error, SOURCE_TYPE.server);
			return false;
		}


		std::vector<std::string> values;
		while (fgets(str, 400, file))
		{
			values = util::split(std::string(str), ":");
			if (values.size() < 2) continue;
			std::string val = "";
			if (values.size() > 2)
			{
				for (unsigned int i = 1; i < values.size(); i++)
				{
					if (i == 1)
						val += values[i];
					else
						val += ":" + values[i];
				}
			}
			else
				val = values[1];
			if (val == "") continue;
			size_t rs = val.find('\n');
			if (rs != std::string::npos) val.erase(val.length() - 1, 1);
			size_t ns = val.find('\r');
			if (ns != std::string::npos) val.erase(val.length() - 1, 1);

			if (values[0] == "MAIN-LOG")
			{
				util::MAIN_LOG = atoi(val.c_str());
			}
			else if (values[0] == "LOG-DETALES")
			{
				config::LOG_DETALES = atoi(val.c_str());
			}
			else if (values[0] == "SERVER-PORT")
			{
				SERVER_PORT = atoi(val.c_str());
			}
			else if (values[0] == "SERVER-AUTHT")
			{
				config::SERVER_AUTH = atoi(val.c_str());
			}
			else if (values[0] == "ALLOW-CORS")
			{
				config::ALLOW_CORS = val;
			}
			else if (values[0] == "WEBSOCKET-PORT")
			{
				config::WEBSOCKET_PORT = atoi(val.c_str());
			}

			util::writeLogLine("Set parameter - " + values[0] + ":" + val, util::message_type.message, SOURCE_TYPE.server);

		}
		values.clear();
		fclose(file);

		util::writeLogLine(config::SERVER_NAME + " server start: " + config::PROGRAMM_VERSION + " ...", util::message_type.message, SOURCE_TYPE.server);


		if (config::ALLOW_CORS != "")
		{
			if (config::SERVER_AUTH == 0)
			{
				config::CORS_HEADERS = "\r\nAccess-Control-Allow-Origin: " + config::ALLOW_CORS + "\r\nAccess-Control-Allow-Methods: POST, GET, OPTIONS\r\nAccess-Control-Allow-Headers: X-PINGOTHER, Content-Type, Accept, Origin, User-Agent, DNT, Cache-Control\r\nAccess-Control-Max-Age: 86400\r\nVary: Accept-Encoding, Origin";
			}
			else
			{
				config::CORS_HEADERS = "\r\nAccess-Control-Allow-Origin: " + config::ALLOW_CORS + "\r\nAccess-Control-Allow-Methods: POST, GET, OPTIONS\r\nAccess-Control-Allow-Headers: Authorization, X-PINGOTHER, Content-Type, Accept, Origin, User-Agent, DNT, Cache-Control, X-Mx-ReqToken\r\nAccess-Control-Max-Age: 86400\r\nVary: Accept-Encoding, Origin\r\nAccess-Control-Allow-Credentials: true";
			}
		}



		if (config::SERVER_AUTH == 1)
		{
			jsc::json jusers = jsc::json::object();

			try
			{
				jsc::strict_parse_error_handler err_handler;
				jusers = jsc::json::parse_file("users.json", err_handler);
			}
			catch (const jsc::parse_error& e)
			{
				util::writeLogLine("Incorrect json users list", util::message_type.error, SOURCE_TYPE.server);
				return false;
			}

			{
				std::lock_guard<std::mutex> lk(user_mutex);

				for (jsc::json userr_row : jusers["users"].array_range())
				{
					http_user user;
					user.login = userr_row["login"].as_string();
					user.name = userr_row["name"].as_string();
					user.password = userr_row["password"].as_string();
					std::string logp = user.login + ":" + user.password;
					std::string log_hash = util::base64_encode((const unsigned char*)logp.c_str(), logp.length());
					user.hash = util::strToLower(log_hash);

					for (jsc::json permit_row : userr_row["permits"].array_range())
					{
						user.permits.push_back(permit_row.as_string());
					}

					USER_LIST.push_back(user);
				}
			}
		}

		return true;
	}
}
