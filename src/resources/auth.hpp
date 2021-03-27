#ifndef HTTP_AUTH_HPP
#define HTTP_AUTH_HPP
#pragma once

#include <map>
#include <utils/util.h>
#include <jsoncons/json.hpp>
#include "../config.hpp"
#include "../net/httpClient.hpp"
#include <openssl/md5.h>
#include "resources.hpp"
#include <libpq-fe.h>

namespace jsc = jsoncons;

namespace auth
{
	std::string getToken(std::string users_id)
	{
		std::string result = "";

		PGconn* conn = NULL;
		PGresult* res = NULL;

		std::string db_conn_string = config::PG_CONN;
		conn = PQconnectdb(db_conn_string.c_str());

		if (PQstatus(conn) != CONNECTION_OK)
		{
			util::writeLogLine("Get token: Connection to database failed: " + std::string(PQerrorMessage(conn)), util::message_type.error, config::SOURCE_TYPE.api);
			return result;
		}

		if (PQsetClientEncoding(conn, "utf8") != 0)
		{
			util::writeLogLine("Get token: Encoding no set: " + std::string(PQerrorMessage(conn)), util::message_type.error, config::SOURCE_TYPE.api);
			return result;
		}

		int current_point = util::getCurrentUnixTime();
		int check_point = current_point - config::TOKEN_LIFE_TIME;
		std::string check_point_str = std::to_string(check_point);

		int session_rows = 0;
		const char* session_params[2];
		session_params[0] = users_id.c_str();
		session_params[1] = check_point_str.c_str();

		std::string session_sql = "SELECT token FROM esb.sessions WHERE users_id = $1 AND last_time > $2";
		res = PQexecParams(conn, session_sql.c_str(), 2, NULL, session_params, NULL, NULL, 0);
		int status = PQresultStatus(res);
		if (status != PGRES_TUPLES_OK)
		{
			util::writeLogLine("Login: No select session: " + std::string(PQerrorMessage(conn)), util::message_type.error, config::SOURCE_TYPE.api);

			PQclear(res);
			if (PQstatus(conn) == CONNECTION_OK)
				PQfinish(conn);

			return result;
		}
		else
		{
			session_rows = PQntuples(res);
		}

		if (session_rows > 0)
		{
			result = std::string(PQgetvalue(res, 0, 0));

			PQclear(res);
			if (PQstatus(conn) == CONNECTION_OK)
				PQfinish(conn);

			return result;
		}

		PQclear(res);



		int time_point = util::getCurrentUnixTime();
		std::string time_point_str = std::to_string(time_point);
		std::string token_seed = std::to_string(time_point) + users_id;
			
		const char* new_session_params[3];
		new_session_params[0] = users_id.c_str();
		new_session_params[1] = time_point_str.c_str();
		new_session_params[2] = token_seed.c_str();

		std::string new_session_sql = "INSERT INTO esb.sessions (token, users_id, last_time) VALUES (upper(md5(random()::text || $3)), $1, $2) RETURNING token";

		res = PQexecParams(conn, new_session_sql.c_str(), 3, NULL, new_session_params, NULL, NULL, 0);
		status = PQresultStatus(res);
		if (status != PGRES_TUPLES_OK)
		{
			util::writeLogLine("No inserted new addres: " + std::string(PQerrorMessage(conn)), util::message_type.error, config::SOURCE_TYPE.api);
		}
		else
		{
			result = std::string(PQgetvalue(res, 0, 0));
		}

		PQclear(res);
		if (PQstatus(conn) == CONNECTION_OK)
			PQfinish(conn);

		return result;
	}

	util::BinData checkToken(std::map<std::string, std::string> params, util::BinData& body_data, bool is_admin = false)
	{
		util::BinData result;

		PGconn* conn = NULL;
		PGresult* res = NULL;

		try
		{
			std::string token = "";
			
			std::map<std::string, std::string>::iterator token_it = params.find("token");
			if (token_it != params.end())
			{
				token = params["token"];
			}

			if ((token == "") && (body_data.size()))
			{
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

				if (j_body_data.has_key("token"))
				{
					token = j_body_data["token"].as_string();
				}
			}

			if (token == "")
			{
				std::string error_string = "No token data";
				util::writeLogLine(error_string, util::message_type.error, config::SOURCE_TYPE.api);
				std::string erroransv = net::getErrorAnswer(400, error_string);
				result << erroransv;
				return result;
			}

			std::string db_conn_string = config::PG_CONN;
			conn = PQconnectdb(db_conn_string.c_str());

			if (PQstatus(conn) != CONNECTION_OK)
			{
				util::writeLogLine("Login: Connection to database failed: " + std::string(PQerrorMessage(conn)), util::message_type.error, config::SOURCE_TYPE.api);
				std::string header = net::getErrorAnswer(500);
				result << header;
				return result;
			}

			if (PQsetClientEncoding(conn, "utf8") != 0)
			{
				util::writeLogLine("Login: Encoding no set: " + std::string(PQerrorMessage(conn)), util::message_type.error, config::SOURCE_TYPE.api);
				std::string header = net::getErrorAnswer(500);
				result << header;
				return result;
			}

			std::string users_id = "";

			int session_rows = 0;
			const char* session_params[1];
			session_params[0] = token.c_str();

			std::string session_sql = "SELECT last_time, users_id FROM esb.sessions WHERE upper(token) = upper($1)";
			res = PQexecParams(conn, session_sql.c_str(), 1, NULL, session_params, NULL, NULL, 0);
			int status = PQresultStatus(res);
			if (status != PGRES_TUPLES_OK)
			{
				util::writeLogLine("Chek token: No select session: " + std::string(PQerrorMessage(conn)), util::message_type.error, config::SOURCE_TYPE.api);

				PQclear(res);
				if (PQstatus(conn) == CONNECTION_OK)
					PQfinish(conn);

				std::string erroransv = net::getErrorAnswer(500);
				result << erroransv;
				return result;
			}
			else
			{
				session_rows = PQntuples(res);
			}

			if (session_rows == 0)
			{
				PQclear(res);
				if (PQstatus(conn) == CONNECTION_OK)
					PQfinish(conn);

				std::string error_string = "Invalid token data";
				util::writeLogLine(error_string, util::message_type.error, config::SOURCE_TYPE.api);
				std::string erroransv = net::getErrorAnswer(400, error_string);
				result << erroransv;
				return result;
			}
			else
			{
				int token_last_time = atoi(PQgetvalue(res, 0, 0));
				users_id = PQgetvalue(res, 0, 1);
				int current_point = util::getCurrentUnixTime();
				int check_point = current_point - config::TOKEN_LIFE_TIME;
				if (check_point > token_last_time)
				{
					jsc::json body_res = jsc::json::object();
					std::string error_string = "Token is expired";
					util::writeLogLine(error_string, util::message_type.message, config::SOURCE_TYPE.api);
					body_res["status"] = "error";
					body_res["descr"] = error_string;

					std::string answer = body_res.to_string();
					util::BinData header = net::getOkAnswer("application/json; charset utf-8", answer.length());
					result << header;
					result << answer;

					PQclear(res);
					if (PQstatus(conn) == CONNECTION_OK)
						PQfinish(conn);

					return result;
				}
			}


			int new_point = util::getCurrentUnixTime();
			std::string new_point_str = std::to_string(new_point);
			const char* token_params[2];
			token_params[0] = new_point_str.c_str();
			token_params[1] = token.c_str();

			std::string token_sql = "UPDATE esb.sessions SET last_time = $1 WHERE token = $2"; ;

			res = PQexecParams(conn, token_sql.c_str(), 2, NULL, token_params, NULL, NULL, 0);
			status = PQresultStatus(res);
			if (status != PGRES_COMMAND_OK)
			{
				util::writeLogLine("Chek token: No update token time: " + std::string(PQerrorMessage(conn)), util::message_type.error, config::SOURCE_TYPE.api);

				PQclear(res);
				if (PQstatus(conn) == CONNECTION_OK)
					PQfinish(conn);

				std::string erroransv = net::getErrorAnswer(500);
				result << erroransv;
				return result;
			}

			PQclear(res);

			if (is_admin)
			{
				const char* usr_params[1];
				usr_params[0] = users_id.c_str();

				std::string usr_sql = "SELECT admin FROM esb.users WHERE id = $1";
				res = PQexecParams(conn, usr_sql.c_str(), 1, NULL, usr_params, NULL, NULL, 0);
				int status = PQresultStatus(res);
				if (status != PGRES_TUPLES_OK)
				{
					util::writeLogLine("Chek token: No select user: " + std::string(PQerrorMessage(conn)), util::message_type.error, config::SOURCE_TYPE.api);

					PQclear(res);
					if (PQstatus(conn) == CONNECTION_OK)
						PQfinish(conn);

					std::string erroransv = net::getErrorAnswer(500);
					result << erroransv;
					return result;
				}

				std::string admin_value = PQgetvalue(res, 0, 0);

				if ((admin_value == "f") || (admin_value == "false"))
				{
					util::writeLogLine("Chek token: Access not allowed for user " + users_id, util::message_type.message, config::SOURCE_TYPE.api);

					PQclear(res);
					if (PQstatus(conn) == CONNECTION_OK)
						PQfinish(conn);

					std::string erroransv = net::getErrorAnswer(403, "Access not allowed for current user");
					result << erroransv;
					return result;

				}

				PQclear(res);
			}


			if (PQstatus(conn) == CONNECTION_OK)
				PQfinish(conn);

		}
		catch (...)
		{
			util::writeLogLine("Chek token: Exeption in chekToken function", util::message_type.failure, config::SOURCE_TYPE.api);
			std::string header = net::getErrorAnswer(500);
			result << header;

		}

		return result;
	}

	util::BinData getAuth(std::map<std::string, std::string> params)
	{
		util::BinData result;

		PGconn* conn = NULL;
		PGresult* res = NULL;

		try
		{

			std::map<std::string, std::string>::iterator login_it = params.find("login");
			std::map<std::string, std::string>::iterator password_it = params.find("password");

			if ((login_it == params.end()) || (password_it == params.end()))
			{
				std::string error_string = "Incorrect login parameters";
				util::writeLogLine(error_string, util::message_type.error, config::SOURCE_TYPE.api);
				std::string erroransv = net::getErrorAnswer(400, error_string);
				result << erroransv;
				return result;
			}

			std::string db_conn_string = config::PG_CONN;
			conn = PQconnectdb(db_conn_string.c_str());

			if (PQstatus(conn) != CONNECTION_OK)
			{
				util::writeLogLine("Login: Connection to database failed: " + std::string(PQerrorMessage(conn)), util::message_type.error, config::SOURCE_TYPE.api);
				std::string header = net::getErrorAnswer(500);
				result << header;
				return result;
			}

			if (PQsetClientEncoding(conn, "utf8") != 0)
			{
				util::writeLogLine("Login: Encoding no set: " + std::string(PQerrorMessage(conn)), util::message_type.error, config::SOURCE_TYPE.api);
				std::string header = net::getErrorAnswer(500);
				result << header;
				return result;
			}

			std::string login = params["login"];
			std::string password = params["password"];

			unsigned char md_res[MD5_DIGEST_LENGTH];
			MD5((unsigned char*)password.c_str(), password.size(), md_res);

			std::string md_psd = std::string((char*)md_res, MD5_DIGEST_LENGTH);
			std::string md_password = util::toHexStr(md_psd, false);
			
			std::string users_id = "", user_id = "", user_password = "", blocked = "", access_token = "";
			jsc::json body_res = jsc::json::object();


			int user_rows = 0;
			const char* user_params[1];
			user_params[0] = login.c_str();

			std::string user_sql = "SELECT id, user_id, password, blocked FROM esb.users WHERE login = $1";
			res = PQexecParams(conn, user_sql.c_str(), 1, NULL, user_params, NULL, NULL, 0);
			int status = PQresultStatus(res);
			if (status != PGRES_TUPLES_OK)
			{
				util::writeLogLine("Login: No select login: " + std::string(PQerrorMessage(conn)), util::message_type.error, config::SOURCE_TYPE.api);

				PQclear(res);
				if (PQstatus(conn) == CONNECTION_OK)
					PQfinish(conn);

				std::string header = net::getErrorAnswer(500);
				result << header;
				return result;
			}
			else
			{
				user_rows = PQntuples(res);
			}

			if (user_rows > 0)
			{
				users_id = std::string(PQgetvalue(res, 0, 0));
				user_id = std::string(PQgetvalue(res, 0, 1));
				user_password = std::string(PQgetvalue(res, 0, 2));
				blocked = std::string(PQgetvalue(res, 0, 3));

				PQclear(res);
				if (PQstatus(conn) == CONNECTION_OK)
					PQfinish(conn);

				std::string error_string = "";
				if ((blocked == "true") || (blocked == "t"))
				{
					error_string = "Access is blocked";
					util::writeLogLine(error_string, util::message_type.message, config::SOURCE_TYPE.api);
					body_res["status"] = "error";
					body_res["descr"] = error_string;

					std::string answer = body_res.to_string();
					util::BinData header = net::getOkAnswer("application/json; charset utf-8", answer.length());
					result << header;
					result << answer;

					return result;
				}

				if (user_password != md_password)
				{
					error_string = "Incorrect credentials";
					util::writeLogLine(error_string, util::message_type.message, config::SOURCE_TYPE.api);
					body_res["status"] = "error";
					body_res["descr"] = error_string;

					std::string answer = body_res.to_string();
					util::BinData header = net::getOkAnswer("application/json; charset utf-8", answer.length());
					result << header;
					result << answer;

					return result;
				}
			}
			else
			{
				//Получение пользователя из 1С=============================================================





			}

			if (users_id != "")
			{
				std::string token = getToken(users_id);
				if (token != "")
				{
					body_res["status"] = "success";
					body_res["token"] = token;
				}
				else
				{
					util::writeLogLine("Token not defined", util::message_type.message, config::SOURCE_TYPE.api);
					std::string header = net::getErrorAnswer(500);
					result << header;
					return result;
				}
			}
			else
			{
				std::string error_string = "Incorrect credentials";
				util::writeLogLine(error_string, util::message_type.message, config::SOURCE_TYPE.api);
				body_res["status"] = "error";
				body_res["descr"] = error_string;
			}

			std::string answer = body_res.to_string();
			util::BinData header = net::getOkAnswer("application/json; charset utf-8", answer.length());
			result << header;
			result << answer;

		}
		catch (...)
		{
			util::writeLogLine("Login: Exeption in getToken function", util::message_type.failure, config::SOURCE_TYPE.api);

			std::string header = net::getErrorAnswer(500);
			result << header;
			
		}

		return result;
	}

	util::BinData updateUser(util::BinData& body_data)
	{
		util::BinData result;

		PGconn* conn = NULL;
		PGresult* res = NULL;

		try
		{
			std::string jdata = body_data.toString();
			jsc::json j_body_data = jsc::json::object();

			try
			{
				jsc::strict_parse_error_handler err_handler;
				j_body_data = jsc::json::parse(jdata, err_handler);
			}
			catch (const jsc::parse_error& e)
			{
				std::string error_string = "Update user: Incorrect body data json";
				util::writeLogLine(error_string, util::message_type.error, config::SOURCE_TYPE.api);
				std::string erroransv = net::getErrorAnswer(400, error_string);
				result << erroransv;
				return result;
			}

			if ((!j_body_data.has_key("users")) || (!j_body_data["users"].is_array()))
			{
				std::string error_string = "Update user: Incorrect body data json. No required parameters";
				util::writeLogLine(error_string, util::message_type.error, config::SOURCE_TYPE.api);
				std::string erroransv = net::getErrorAnswer(400, error_string);
				result << erroransv;
				return result;
			}

			std::string db_conn_string = config::PG_CONN;
			conn = PQconnectdb(db_conn_string.c_str());

			if (PQstatus(conn) != CONNECTION_OK)
			{
				util::writeLogLine("Update user: Connection to database failed: " + std::string(PQerrorMessage(conn)), util::message_type.error, config::SOURCE_TYPE.api);
				std::string header = net::getErrorAnswer(500);
				result << header;
				return result;
			}

			if (PQsetClientEncoding(conn, "utf8") != 0)
			{
				util::writeLogLine("Update user: Encoding no set: " + std::string(PQerrorMessage(conn)), util::message_type.error, config::SOURCE_TYPE.api);
				std::string header = net::getErrorAnswer(500);
				result << header;
				return result;
			}

			jsc::json j_result = jsc::json::object();
			j_result["users"] = jsc::json::array();


			for (jsc::json user_row : j_body_data["users"].array_range())
			{
				if (!user_row.has_key("login"))
				{
					std::string error_string = "No required parameter login";
					util::writeLogLine("Update user: " + error_string, util::message_type.error, config::SOURCE_TYPE.api);

					jsc::json rep_user_item = res::getResultItem(util::result_type.error, error_string, "");
					j_result["users"].add(rep_user_item);

					continue;
				}

				std::string users_id = "", id_value;
				
				if (user_row.has_key("user_id"))
				{
					int usr_rows = 0;
					std::string usr_sql = "SELECT id FROM esb.users WHERE user_id = $1";
					std::string param_value = user_row["user_id"].as_string();
					id_value = param_value;

					const char* usr_params[1];
					usr_params[0] = param_value.c_str();

					res = PQexecParams(conn, usr_sql.c_str(), 1, NULL, usr_params, NULL, NULL, 0);
					int status = PQresultStatus(res);
					if (status != PGRES_TUPLES_OK)
					{
						std::string error_string = "No select user: " + std::string(PQerrorMessage(conn));
						util::writeLogLine("Update user: " + error_string, util::message_type.error, config::SOURCE_TYPE.api);

						jsc::json rep_user_item = res::getResultItem(util::result_type.error, error_string, param_value);
						j_result["users"].add(rep_user_item);

						PQclear(res);

						continue;
					}
					else
					{
						usr_rows = PQntuples(res);
					}
					
					if (usr_rows > 0)
					{
						users_id = PQgetvalue(res, 0, 0);
					}
					PQclear(res);
				}
				else
				{
					id_value = user_row["login"].as_string();
				}

				if (users_id == "")
				{
					std::string usr_sql = "SELECT id FROM esb.users WHERE login = $1";
					std::string param_value = user_row["login"].as_string();

					int usr_rows = 0;

					const char* usr_params[1];
					usr_params[0] = param_value.c_str();

					res = PQexecParams(conn, usr_sql.c_str(), 1, NULL, usr_params, NULL, NULL, 0);
					int status = PQresultStatus(res);
					if (status != PGRES_TUPLES_OK)
					{
						std::string error_string = "No select user: " + std::string(PQerrorMessage(conn));
						util::writeLogLine("Update user: " + error_string, util::message_type.error, config::SOURCE_TYPE.api);

						jsc::json rep_user_item = res::getResultItem(util::result_type.error, error_string, param_value);
						j_result["users"].add(rep_user_item);

						PQclear(res);

						continue;
					}
					else
					{
						usr_rows = PQntuples(res);
					}

					if (usr_rows > 0)
					{
						users_id = PQgetvalue(res, 0, 0);
					}
					PQclear(res);
				}

				std::string request_str = "", comma = "";

				if (users_id != "")
				{
					if (user_row.has_key("user_id")) request_str = "user_id = '" + user_row["user_id"].as_string() + "'";

					if (request_str != "") comma = ", ";
					if (user_row.has_key("login")) request_str += comma + "login = '" + user_row["login"].as_string() + "'";

					if (request_str != "") comma = ", ";
					if (user_row.has_key("password")) request_str += comma + "password = '" + user_row["password"].as_string() + "'";

					if (request_str != "") comma = ", ";
					if (user_row.has_key("blocked")) request_str += comma + "blocked = '" + user_row["blocked"].as_string() + "'";

					request_str = "UPDATE esb.users SET " + request_str + " WHERE id = '" + users_id + "'";
				}
				else
				{
					std::string fields_str = "", values_str = "";

					if (user_row.has_key("user_id"))
					{
						fields_str = "user_id";
						values_str = "'" + user_row["user_id"].as_string() + "'";
					}

					if (fields_str != "") comma = ", ";
					if (user_row.has_key("login")) 
					{
						fields_str += comma + "login";
						values_str += comma + "'" + user_row["login"].as_string() + "'";
					}

					if (fields_str != "") comma = ", ";
					if (user_row.has_key("password"))
					{
						fields_str += comma + "password";
						values_str += comma + "'" + user_row["password"].as_string() + "'";
					}

					if (fields_str != "") comma = ", ";
					if (user_row.has_key("blocked"))
					{
						fields_str += comma + "blocked";
						values_str += comma + "'" + user_row["blocked"].as_string() + "'";
					}

					request_str = "INSERT INTO esb.users (" + fields_str + ") VALUES (" + values_str + ")";
				}

				res = PQexec(conn, request_str.c_str());
				if (PQresultStatus(res) != PGRES_COMMAND_OK)
				{
					std::string error_string = "No insert or update user: " + std::string(PQerrorMessage(conn));
					util::writeLogLine("Update user: " + error_string, util::message_type.error, config::SOURCE_TYPE.api);

					jsc::json rep_user_item = res::getResultItem(util::result_type.error, error_string, id_value);
					j_result["users"].add(rep_user_item);
					PQclear(res);

					continue;
				}
				PQclear(res);


				jsc::json rep_user_item = res::getResultItem(util::result_type.success, "User is received successfuly", id_value);
				j_result["users"].add(rep_user_item);
			}

			std::string res_data = j_result.as_string();
			util::BinData header = net::getOkAnswer("application/json; charset utf-8", res_data.length());
			result << header;
			result << res_data;
		}
		catch (...)
		{
			util::writeLogLine("Update: Exeption in updateUser function", util::message_type.failure, config::SOURCE_TYPE.api);
			std::string header = net::getErrorAnswer(500);
			result << header;

			if (PQstatus(conn) == CONNECTION_OK)
				PQfinish(conn);
		}

		return result;
	}
}
#endif