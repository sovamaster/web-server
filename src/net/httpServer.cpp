/*!
 * Copyright (c) 2015-2018, Andrei Chudinovich
 * Released under MIT license
 * Date: 03.09.2018
 */



#include "httpServer.hpp"
#include "../config.hpp"
#include "../api.hpp"
#include <map>



util::BinData net::getFileContent(std::string request_source_path)
{
	util::BinData result;
	try
	{
		//std::cout << "step 1" << std::endl;
		std::string file_path = request_source_path.substr(1);
		util::BinData content = util::getFileContent(file_path, true);
		if (content.size() > 0)
		{
			std::string mime = util::getMimeType(file_path);
			util::BinData header = net::getOkAnswer(mime, content.size());
			result << header;
			result << content;
		}
		else
		{
			std::string header = net::getErrorAnswer(404);
			result << header;
		}
	}
	catch (...)
	{
		util::writeLogLine("Exeption in setFileContent function", util::message_type.failure, config::SOURCE_TYPE.server);
		std::string header = net::getErrorAnswer(500);

		result << header;

	}
	return result;
}

void sendStream(SOCKET s_client, std::string request_source_path, util::BinData total_recv_data, std::string range_begin_str, std::string range_end_str)
{
	try
	{
		std::string file_path = request_source_path.substr(1);
		FILE* file = fopen(file_path.c_str(), "rb");
		if (config::LOG_DETALES == 1) util::writeLogLine("Begin read file to browser", util::message_type.message, config::SOURCE_TYPE.api);
		if (file != NULL)
		{
			std::string mime = util::getMimeType(file_path);

			int range_begin = atoi(range_begin_str.c_str());

			long filesize;

			fseek(file, 0, SEEK_END);
			filesize = ftell(file);
			fseek(file, 0, SEEK_SET);

			std::string tvend = (range_end_str == "") ? std::to_string(filesize - 1) : range_end_str;
			int range_end = (range_end_str == "") ? filesize - 1 : atoi(range_end_str.c_str());
			unsigned int content_lenth = range_end - range_begin + 1;

			std::string header = "HTTP/1.1 206 Partial Content\r\nServer: tv-sia-service\r\nContent-Type: " + mime + "\r\nAccept-Ranges:bytes\r\nAccess-Control-Allow-Origin: *\r\nContent-Length:" + std::to_string(content_lenth) + "\r\nContent-Range:bytes " + range_begin_str + "-" + tvend + "/" + std::to_string(filesize) + "\r\n\r\n";


			send(s_client, header.c_str(), (int)header.length(), 0);
			if (config::LOG_DETALES == 1) util::writeLogLine("Sended head for file to browser", util::message_type.message, config::SOURCE_TYPE.api);

			char tvbufer[net::SERVER_DATASIZE];

			fseek(file, range_begin, SEEK_SET);


			int cont = 0;
			while (true)
			{
				int cont_size = 0;
				if ((cont + sizeof(tvbufer)) < content_lenth) cont_size = sizeof(tvbufer);
				else cont_size = content_lenth - cont;
				int readsize = fread(tvbufer, 1, cont_size, file);
				if (readsize != 0)
				{
					cont = cont + readsize;
#ifdef _WIN32
					send(s_client, tvbufer, cont_size, 0);
#else											
					//send(tv_client, tvbufer, cont_size, MSG_EOR | MSG_NOSIGNAL);
					send(s_client, tvbufer, cont_size, 0);
#endif
					//if (config::LOG_DETALES == 1) util::writeLogLine("Sended file part to browser", false);
				}
				else
				{
					if (config::LOG_DETALES == 1) util::writeLogLine("No more file read", util::message_type.message, config::SOURCE_TYPE.api);
					break;
				}
			}

			fclose(file);
		}
		else
		{
			util::BinData answer;
			std::string header = net::getErrorAnswer(404);
			answer << header;


			send(s_client, (const char*)answer.data(), (int)answer.size(), 0);
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
			shutdown(s_client, SD_SEND);
			util::writeLogLine("File " + file_path + " not found", util::message_type.warning, config::SOURCE_TYPE.api);
		}
	}
	catch (...)
	{
		util::BinData answer;
		std::string header = net::getErrorAnswer(500);
		answer << header;


		send(s_client, (const char*)answer.data(), (int)answer.size(), 0);
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
		shutdown(s_client, SD_SEND);
		util::writeLogLine("Exeption in sendContent function", util::message_type.error, config::SOURCE_TYPE.api);
	}
}

void runHttpClient(SOCKET s_client)
{
	try
	{
		util::writeLogLine("New HTML client", util::message_type.message, config::SOURCE_TYPE.api);


		char recvbuf[net::SERVER_RECVDATASIZE];
		//std::string all_request = "";

		util::BinData total_recv;
		util::BinData total_recv_data;

		int recv_len = 1;
		int head_len = 0;
		unsigned int request_data_len = 0;
		bool is_socket_closed = false;


		std::string request_source_path = "", request_content_type = "";
		std::vector<std::string> permits;
		std::string range_begin_str = "", range_end_str = "";
		std::string connection_mode = "";
		std::string request_type = "";

		while (recv_len > 0)
		{
			try
			{
				memset(recvbuf, 0, net::SERVER_RECVDATASIZE);
				recv_len = recv(s_client, (char*)&recvbuf, sizeof(recvbuf), 0);

				if (recv_len > 0)
				{
					bool is_error = false;

					util::BinData brecv;
					brecv.fromChar((unsigned char*)recvbuf, recv_len);
					total_recv << brecv;

					if (head_len == 0)
					{
						for (unsigned int i = 0; i < total_recv.size(); i++)
						{
							if (i > 2000)
							{
								util::writeLogLine("Incorrect http header for getting data", util::message_type.error, config::SOURCE_TYPE.api);
								is_error = true;

								std::string header = net::getErrorAnswer(400);
								send(s_client, header.c_str(), (int)header.length(), 0);

								shutdown(s_client, SD_SEND);
								util::closeSocket(s_client);
								is_socket_closed = true;
								recv_len = -2;
								total_recv.clear();
								break;
							}

							if ((unsigned int)(i + 3) < total_recv.size())
							{
								if ((total_recv[i] == 13) && (total_recv[i + 1] == 10) && (total_recv[i + 2] == 13) && (total_recv[i + 3] == 10))
								{
									head_len = i + 4;
									break;
								}
							}
						}
					}

					if (is_error)
					{
						is_error = false;
						total_recv.clear();
						continue;
					}

					if ((head_len > 0) && (request_source_path == ""))
					{
						if (config::LOG_DETALES == 1) util::writeLogLine("Received new request...", util::message_type.message, config::SOURCE_TYPE.api);

						std::string recv_header = total_recv.toString(0, head_len);

						recv_header = util::strToLower(recv_header);

						size_t httppos = recv_header.find("http/1.1");
						if (std::string::npos == httppos)
						{
							util::writeLogLine("Invalid request header. No http header", util::message_type.error, config::SOURCE_TYPE.api);

							std::string header = net::getErrorAnswer(400);
							send(s_client, header.c_str(), (int)header.length(), 0);

							shutdown(s_client, SD_SEND);
							util::closeSocket(s_client);
							is_socket_closed = true;
							recv_len = -2;
							total_recv.clear();
							continue;
						}

						size_t hostpos = recv_header.find("host:");
						if (std::string::npos == hostpos)
						{
							util::writeLogLine("Invalid request header. No host header", util::message_type.error, config::SOURCE_TYPE.api);

							std::string header = net::getErrorAnswer(400);
							send(s_client, header.c_str(), (int)header.length(), 0);

							shutdown(s_client, SD_SEND);
							util::closeSocket(s_client);
							is_socket_closed = true;
							recv_len = -2;
							total_recv.clear();
							continue;
						}

						if (config::SERVER_AUTH == 1)
						{
							size_t authpos = recv_header.find("authorization:");
							if (std::string::npos != authpos)
							{
								size_t an = recv_header.find("\r\n", authpos);
								size_t anb = recv_header.find("Basic", authpos);
								if ((std::string::npos != an) && (std::string::npos != anb))
								{
									std::string auth_hash = util::strTrim(recv_header.substr(anb + 5, an - anb - 5));

									{
										std::lock_guard<std::mutex> lk(config::user_mutex);

										auto it = std::find_if(config::USER_LIST.begin(), config::USER_LIST.end(), [&](const config::http_user& user) {
											return (user.hash == auth_hash);
											});

										if (it != config::USER_LIST.end())
										{
											permits = (*it).permits;
										}
										else
										{
											util::writeLogLine("No valid login or password", util::message_type.warning, config::SOURCE_TYPE.api);


											std::string header = net::getErrorAnswer(401);
											send(s_client, header.c_str(), (int)header.length(), 0);

											shutdown(s_client, SD_SEND);
											util::closeSocket(s_client);
											is_socket_closed = true;
											recv_len = -2;
											total_recv.clear();
											continue;
										}
									}
								}
								else
								{
									util::writeLogLine("Invalid request header. No Basic header", util::message_type.error, config::SOURCE_TYPE.api);

									std::string header = net::getErrorAnswer(400);
									send(s_client, header.c_str(), (int)header.length(), 0);

									shutdown(s_client, SD_SEND);
									util::closeSocket(s_client);
									is_socket_closed = true;
									recv_len = -2;
									total_recv.clear();
									continue;
								}
							}
							else
							{
								util::writeLogLine("Unauthorized request", util::message_type.warning, config::SOURCE_TYPE.api);

								std::string header = net::getErrorAnswer(401);
								send(s_client, header.c_str(), (int)header.length(), 0);

								shutdown(s_client, SD_SEND);
								util::closeSocket(s_client);
								is_socket_closed = true;
								recv_len = -2;
								total_recv.clear();
								continue;
							}
						}


						size_t rangepos = recv_header.find("range: bytes=");
						if (std::string::npos == rangepos)
							rangepos = recv_header.find("range:bytes=");
						if (std::string::npos != rangepos)
						{
							size_t rangedelpos = recv_header.find("-", rangepos);
							size_t rangeendpos = recv_header.find("\r\n", rangedelpos);
							range_begin_str = recv_header.substr(rangepos + 13, rangedelpos - (rangepos + 13));
							range_end_str = recv_header.substr(rangedelpos + 1, rangeendpos - (rangedelpos + 1));
						}

						size_t postpos = recv_header.find("post");

						if (postpos != 0)
						{
							size_t getpos = recv_header.find("get");

							if (getpos == 0)
							{
								request_source_path = util::strTrim(recv_header.substr(getpos + 3, httppos - getpos - 3));
								request_type = "GET";
							}
							else
							{
								size_t goptpos = recv_header.find("options");

								if (goptpos == 0)
								{
									util::BinData header = net::getOkAnswer("text/plain", 0);
									send(s_client, (const char*)header.data(), (int)header.size(), 0);

									shutdown(s_client, SD_SEND);
									total_recv.clear();
									continue;
								}
								else
								{
									util::writeLogLine("Invalid request header. No OPTIONS, GET or POST header", util::message_type.error, config::SOURCE_TYPE.api);


									std::string header = net::getErrorAnswer(400);
									send(s_client, header.c_str(), (int)header.length(), 0);

									shutdown(s_client, SD_SEND);
									total_recv.clear();
									continue;
								}
							}
						}
						else
						{
							request_type = "POST";
							size_t contpos = recv_header.find("content-length:");
							if (std::string::npos != contpos)
							{
								size_t pn = recv_header.find("\r\n", contpos);
								std::string cont_size = util::strTrim(recv_header.substr(contpos + 15, pn - contpos - 15));
								request_data_len = atoi(cont_size.c_str());
							}
							else
							{
								util::writeLogLine("Invalid request header. No Content-Length header", util::message_type.error, config::SOURCE_TYPE.api);


								std::string header = net::getErrorAnswer(500);
								send(s_client, header.c_str(), (int)header.length(), 0);

								shutdown(s_client, SD_SEND);
								total_recv.clear();
								continue;
							}


							size_t typepos = recv_header.find("content-type:");
							if (std::string::npos != typepos)
							{
								size_t tn = recv_header.find("\r\n", typepos);
								request_content_type = util::strTrim(recv_header.substr(typepos + 13, tn - typepos - 13));
							}

							request_source_path = util::strTrim(recv_header.substr(postpos + 4, httppos - postpos - 4));

							std::lock_guard<std::mutex> lk(config::user_mutex);
							if (permits.size() > 0)
							{
								auto permit_result = std::find(permits.begin(), permits.end(), request_source_path);
								if (permit_result == permits.end()) {


									std::string header = net::getErrorAnswer(403);
									send(s_client, header.c_str(), (int)header.length(), 0);

									request_data_len = 0;
									head_len = 0;
									request_source_path = "";
									request_content_type = "";

									shutdown(s_client, SD_SEND);
									total_recv.clear();
									continue;
								}
							}

							util::BinData t_recv = total_recv(head_len, total_recv.size() - head_len);
							total_recv_data << t_recv;
						}


						size_t rangeconn = recv_header.find("connection:");
						if (std::string::npos != rangeconn)
						{
							size_t pcn = recv_header.find("\r\n", rangeconn);
							connection_mode = util::strTrim(recv_header.substr(rangeconn + 11, pcn - rangeconn - 11));
						}
					}
					else
					{
						if (request_data_len > 0)
						{
							int remain_len = request_data_len - total_recv_data.size();
							util::BinData b_recv = brecv(0, remain_len);
							total_recv_data << b_recv;
						}
					}
				}
				else
				{
					total_recv.clear();
					total_recv_data.clear();
					request_data_len = 0;
					head_len = 0;
					request_source_path = "";
					request_content_type = "";
					range_begin_str = "";
					range_end_str = "";

				}


				//Request processing ----------------------------------------------------

				if ((request_data_len == total_recv_data.size()) && (head_len > 0))
				{
					
					std::vector<std::string> request_path_parts = util::split(request_source_path, "?");
					std::string request_params = "";
					if (request_path_parts.size() > 1)
					{
						request_params = request_path_parts[1];
					}
					request_source_path = request_path_parts[0];
					size_t rp = request_source_path.find("/", request_source_path.size() - 1);
					if (rp != std::string::npos)
					{
						request_source_path = request_source_path.substr(0, rp);
					}
					std::vector<std::string> params_parts = util::split(request_params, "&");

					std::map<std::string, std::string> params;

					for (unsigned int i = 0; i < params_parts.size(); i++)
					{
						std::vector<std::string> param_parts = util::split(params_parts[i], "=");
						if (param_parts.size() > 1)
						{
							params.insert(std::pair<std::string, std::string>(param_parts[0], param_parts[1]));
						}
					}

					util::BinData answer = api::getResponce(total_recv_data, request_type, request_source_path, params);
					if (answer.size() == 0)
					{
						if ((range_begin_str != "") && (range_end_str != ""))
						{
							std::thread sendStreamThread(sendStream, s_client, request_source_path, total_recv_data, range_begin_str, range_end_str);
							sendStreamThread.detach();

							total_recv.clear();
							total_recv_data.clear();
							request_data_len = 0;
							head_len = 0;
							request_source_path = "";
							request_content_type = "";
							range_begin_str = "";
							range_end_str = "";

							continue;
						}
						else
						{
							answer = net::getFileContent(request_source_path);
						}

					}

					send(s_client, (const char*)answer.data(), (int)answer.size(), 0);
					std::this_thread::sleep_for(std::chrono::milliseconds(300));

					size_t erp = answer.find("HTTP/1.1 200");
					bool is_error_answer = (erp != 0);

					if ((connection_mode != "keep-alive") || (is_error_answer))
					{
						util::closeSocket(s_client);
						is_socket_closed = true;
						recv_len = -2;
					}


					total_recv.clear();
					total_recv_data.clear();
					request_data_len = 0;
					head_len = 0;
					request_source_path = "";
					request_content_type = "";
					range_begin_str = "";
					range_end_str = "";

				}

			}
			catch (...)
			{
				util::writeLogLine("Exeption in runHTMLClient while block", util::message_type.failure, config::SOURCE_TYPE.api);
			}
		}

		if (!is_socket_closed)
		{
			shutdown(s_client, SD_BOTH);
			util::closeSocket(s_client);
		}

		util::writeLogLine("TCP client closed", util::message_type.message, config::SOURCE_TYPE.api);
	}
	catch (...)
	{
		util::writeLogLine("Exeption in runTCPClient function", util::message_type.failure, config::SOURCE_TYPE.api);

		shutdown(s_client, SD_BOTH);
		util::closeSocket(s_client);
	}
}

void net::runTCPServer()
{
	SOCKET h_server;
	try
	{

		SOCKET s_client = 0;
		sockaddr_in ssin;

		h_server = socket(AF_INET, SOCK_STREAM, 0);
		if (h_server == SOCKET_ERROR)
		{
			util::writeLogLine("TCP socket not created", util::message_type.error, config::SOURCE_TYPE.api);
			return;
		}

		util::writeLogLine("TCP server start", util::message_type.message, config::SOURCE_TYPE.api);

		ssin.sin_family = AF_INET;
		ssin.sin_addr.s_addr = INADDR_ANY;
		ssin.sin_port = htons(config::SERVER_PORT);

		if (bind(h_server, (sockaddr*)&ssin, sizeof(ssin)) == SOCKET_ERROR)
		{
#ifdef _WIN32
			WSACleanup();
#endif
			util::closeSocket(h_server);

			util::writeLogLine("TCP socket not binded", util::message_type.error, config::SOURCE_TYPE.api);
			return;
		}


		while (listen(h_server, 50) < 0)
		{
			util::writeLogLine("TCP listen failed", util::message_type.error, config::SOURCE_TYPE.api);
			std::this_thread::sleep_for(std::chrono::seconds(5));
		}

		while (true)
		{
			try
			{
				while ((s_client = accept(h_server, NULL, NULL)) == INVALID_SOCKET)
				{
					util::writeLogLine("Invalid TCP socket", util::message_type.error, config::SOURCE_TYPE.api);
					std::this_thread::sleep_for(std::chrono::seconds(5));
				}

				std::thread htmlClientThread(runHttpClient, s_client);
				//videoClientThread.join();
				htmlClientThread.detach();

			}
			catch (...)
			{
				util::writeLogLine("Exeption in runHTMLServer while block", util::message_type.error, config::SOURCE_TYPE.api);

				util::closeSocket(s_client);
			}
		}

#ifdef _WIN32
		WSACleanup();
#endif
		util::closeSocket(h_server);
		util::writeLogLine("TCP server stop", util::message_type.message, config::SOURCE_TYPE.api);
	}
	catch (...)
	{
		util::writeLogLine("Exeption in runHTMLServer function. TCP server is stopped", util::message_type.failure, config::SOURCE_TYPE.api);
#ifdef _WIN32
		WSACleanup();
#endif
		util::closeSocket(h_server);
	}
}

util::BinData net::getOkAnswer(const std::string content_type, int content_length)
{
	std::time_t seconds = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	tm* timeinfo = localtime(&seconds);
	const std::string format = "%a, %d %b %Y %H:%M:%S GMT";
	char buffer[80];
	strftime(buffer, 80, format.c_str(), timeinfo);
	std::string setdate = std::string(buffer);

	std::string dataanswer = "HTTP/1.1 200 OK\r\nServer: " + config::SERVER_NAME + "-server-" + config::PROGRAMM_VERSION + "\r\nContent-Length:" + std::to_string(content_length) + "\r\nDate: " + setdate + "\r\nContent-Type: " + content_type + config::CORS_HEADERS + "\r\n\r\n";

	util::BinData result;
	result << dataanswer;
	return result;
}

std::string net::getErrorAnswer(int error_id)
{
	std::string result;
	switch (error_id)
	{
	case 400:
		result = "HTTP/1.1 400 Bad Request\r\n" + config::HTTP_SERVER_HEADER + config::CORS_HEADERS + "\r\n\r\n";
		break;
	case 500:
		result = "HTTP/1.1 500 Internal Server Error\r\n" + config::HTTP_SERVER_HEADER + config::CORS_HEADERS + "\r\n\r\n";
		break;
	case 401:
		result = "HTTP/1.1 401 Unauthorized\r\n" + config::HTTP_SERVER_HEADER + config::CORS_HEADERS + "\r\nWWW-Authenticate: Basic realm=\"" + config::HTTP_SERVER_HEADER + +"\"\r\n\r\n";
		break;
	case 403:
		result = "HTTP/1.1 403 Forbidden\r\n" + config::HTTP_SERVER_HEADER + config::CORS_HEADERS + "\r\n\r\n";
		break;
	case 404:
		result = "HTTP/1.1 404 Not Found\r\n" + config::HTTP_SERVER_HEADER + config::CORS_HEADERS + "\r\n\r\n";
		break;
	case 200:
		result = "HTTP/1.1 200 OK\r\n" + config::HTTP_SERVER_HEADER + config::CORS_HEADERS + "\r\n\r\n";
		break;
	default:
		result = "HTTP/1.1 500 Internal Server Error\r\n" + config::HTTP_SERVER_HEADER + config::CORS_HEADERS + "\r\n\r\n";
		break;
	}

	return result;
}

std::string net::getErrorAnswer(int error_id, std::string error_text)
{
	std::string result;
	std::string data = "{\"error\": \"" + error_text + "\"}";
	std::string data_header = "Content-Length:" + std::to_string(data.length()) + "\r\nContent-Type: application/json; charset utf-8\r\n";

	switch (error_id)
	{
	case 400:
		result = "HTTP/1.1 400 Bad Request\r\n" + data_header + config::HTTP_SERVER_HEADER + config::CORS_HEADERS + "\r\n\r\n" + data;
		break;
	case 500:
		result = "HTTP/1.1 500 Internal Server Error\r\n" + data_header + config::HTTP_SERVER_HEADER + config::CORS_HEADERS + "\r\n\r\n" + data;
		break;
	case 401:
		result = "HTTP/1.1 401 Unauthorized\r\n" + data_header + config::HTTP_SERVER_HEADER + config::CORS_HEADERS + "\r\nWWW-Authenticate: Basic realm=\"" + config::HTTP_SERVER_HEADER + +"\"\r\n\r\n" + data;
		break;
	case 403:
		result = "HTTP/1.1 403 Forbidden\r\n" + data_header + config::HTTP_SERVER_HEADER + config::CORS_HEADERS + "\r\n\r\n" + data;
		break;
	case 404:
		result = "HTTP/1.1 404 Not Found\r\n" + data_header + config::HTTP_SERVER_HEADER + config::CORS_HEADERS + "\r\n\r\n" + data;
		break;
	case 200:
		result = "HTTP/1.1 200 OK\r\n" + data_header + config::HTTP_SERVER_HEADER + config::CORS_HEADERS + "\r\n\r\n" + data;
		break;
	default:
		result = "HTTP/1.1 500 Internal Server Error\r\n" + data_header + config::HTTP_SERVER_HEADER + config::CORS_HEADERS + "\r\n\r\n" + data;
		break;
	}

	return result;
}

