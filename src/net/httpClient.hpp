/*!
 * Copyright (c) 2015-2018, Andrei Chudinovich
 * Released under MIT license
 * Date: 03.09.2018
 */


#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP


#include <string>
#include <vector>
#include <openssl/ssl.h>

#include <utils/util.h>
#include "../config.hpp"

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





namespace net
{
	const int CLIENT_RECVDATASIZE = 4200;

	struct reqType {
		std::string post = "POST";
		std::string get = "GET";
		std::string put = "PUT";
		std::string del = "DELETE";
	} requestType;

	struct respStatus {
		std::string success = "success";
		std::string error = "error";
	} responceStatus;
	
	struct httpRequest {
		std::string type;
		std::string path = "/";
		std::string host;
		std::string protocol = "HTTP/1.1";
		std::string content_type;
		std::vector<std::string> headers;
		util::BinData data;
	};
	
	struct httpResponce {
		std::string status;
		std::string descr;
		std::string headers;
		util::BinData data;
		std::string result;
	};
	
	class httpClient {
	private:
		SOCKET x_socket;
		bool x_connection;
		std::string x_src_type;
		bool x_is_ssl;
		SSL* ssl;
		SSL_CTX* ctx;
		std::string x_host;
		std::string x_port;

		void InitSSL_CTX()
		{
			const SSL_METHOD* method;
			/*SSL_CTX* ctx;*/

#if	(OPENSSL_VERSION_NUMBER < 0x10100000L)
			method = TLSv1_2_client_method();
#else
			method = TLS_client_method();
#endif	

			ctx = SSL_CTX_new(method);
			ssl = SSL_new(ctx);
		}

		httpResponce getRecvData()
		{
			httpResponce res;
			try
			{
				int recv_error = 0, data_size = 0;
				int recv_len = 1, data_recv_len = 0;
				char data_recvbuf[net::CLIENT_RECVDATASIZE];
				std::string result = "", chunked_result = "";
				bool chunked = false;

				int remine_data_size = 0;

				std::string tempstr = "";


				while (true)
				{
					memset(&data_recvbuf, 0, sizeof(data_recvbuf));
					if (recv_error > 100)
					{
						util::writeLogLine("Data not resived completely", util::message_type.error, x_src_type);
						res.status = "error";
						return res;
					}

					int head_len = 0;

					if (x_is_ssl)
					{
						recv_len = SSL_read(ssl, data_recvbuf, sizeof(data_recvbuf));
					}
					else
					{
						recv_len = recv(x_socket, (char*)&data_recvbuf, sizeof(data_recvbuf), 0);
					}


					if (recv_len < 0)
					{
						util::writeLogLine("Data not resived. Connection lost", util::message_type.error, x_src_type);
						res.status = "error";

						util::closeSocket(x_socket);
						if (x_is_ssl)
						{
							if (ssl != NULL) SSL_free(ssl);
							if (ctx != NULL) SSL_CTX_free(ctx);
						}

						x_connection = false;
						return res;
					}

					if (recv_len == 0)
					{
						recv_error++;
						continue;
					}

					recv_error = 0;

					if (std::string(data_recvbuf, 4) == "HTTP")
					{
						for (int i = 0; i < recv_len; i++)
						{
							if ((i + 3) >= net::CLIENT_RECVDATASIZE)
							{
								util::writeLogLine("Incorrect http header for getting data", util::message_type.error, x_src_type);
								res.headers = data_recvbuf;
								res.status = "error";
								return res;
							}

							if ((data_recvbuf[i] == 13) && (data_recvbuf[i + 1] == 10) && (data_recvbuf[i + 2] == 13) && (data_recvbuf[i + 3] == 10))
							{
								head_len = i + 4;
								break;
							}
						}

						std::string headdata = std::string(data_recvbuf).substr(0, head_len);
						res.headers = headdata;
						headdata = util::strToLower(headdata);

						if (std::string::npos != headdata.find("404 not found"))
						{
							util::writeLogLine("Answer Data not found", util::message_type.error, x_src_type);
							res.result = "404";
							res.descr = "404 Data not found";
							res.status = "error";
						}
						else if (std::string::npos != headdata.find("400 bad request"))
						{
							util::writeLogLine("Answer Bad Request", util::message_type.error, x_src_type);
							res.result = "400";
							res.descr = "400 Bad Request";
							res.status = "error";
						}
						else if (std::string::npos != headdata.find("401 unauthorized"))
						{
							util::writeLogLine("Answer Unauthorized", util::message_type.error, x_src_type);
							res.result = "401";
							res.descr = "401 Unauthorized";
							res.status = "error";
						}
						else if (std::string::npos != headdata.find("403 forbidden"))
						{
							util::writeLogLine("Answer Forbidden", util::message_type.error, x_src_type);
							res.result = "403";
							res.descr = "403 Forbidden";
							res.status = "error";
						}
						else if (std::string::npos != headdata.find("500 internal server error"))
						{
							util::writeLogLine("Answer Internal Server Error", util::message_type.error, x_src_type);
							res.result = "500";
							res.descr = "500 Internal Server Error";
							res.status = "error";
						}
						else if (std::string::npos != headdata.find("413 request entity too large"))
						{
							util::writeLogLine("Answer Request Entity Too Large", util::message_type.error, x_src_type);
							res.result = "413";
							res.descr = "413 Request Entity Too Large";
							res.status = "error";
						}
						else if (std::string::npos != headdata.find("504 gateway time-out"))
						{
							util::writeLogLine("Answer Gateway Time-Out", util::message_type.error, x_src_type);
							res.result = "504";
							res.descr = "504 Gateway Time-Out";
							res.status = "error";
						}
						else if (std::string::npos != headdata.find("200 ok"))
						{
							res.status = "200";
							res.descr = "200 OK";
							res.status = "success";
						}
						else
						{
							util::writeLogLine("Unknown answer error", util::message_type.error, x_src_type);
							res.result = "0";
							res.descr = "Unknown answer error";
							res.status = "error";
						}
						size_t contpos = headdata.find("content-length:");
						if (std::string::npos != contpos)
						{
							size_t pn = headdata.find("\r\n", contpos);
							std::string cont_size = util::strTrim(headdata.substr(contpos + 15, pn - contpos - 15));
							data_size = atoi(cont_size.c_str());
						}
						else
						{
							size_t chunkpos = headdata.find("transfer-encoding: chunked");
							if (std::string::npos != chunkpos)
							{
								chunked = true;
							}
						}
					}

					data_recv_len += recv_len - head_len;

					result += std::string(data_recvbuf).substr(head_len, recv_len - head_len);


					if (chunked)
					{
						int mess_remain_data_size = recv_len;

						std::string sh_result = std::string(data_recvbuf).substr(head_len, recv_len - head_len);

						tempstr += sh_result;
						size_t size_pt = 0;
						while (true)
						{
							if (remine_data_size > 0)
							{
								std::string data_part = "";
								if (sh_result.length() >= (unsigned int)remine_data_size)
								{
									data_part = sh_result.substr(0, remine_data_size);
								}
								else
								{
									data_part = sh_result.substr(0, net::CLIENT_RECVDATASIZE);
								}

								chunked_result += data_part;
								remine_data_size -= data_part.length();
								mess_remain_data_size -= data_part.length();

								if (remine_data_size > 0)
									break;

							}
							else
							{
								int begin_size = recv_len - mess_remain_data_size;
								size_pt = sh_result.find("\r\n", begin_size + 1);

								if (std::string::npos != size_pt)
								{
									std::string hex_chunk_size = util::strTrim(sh_result.substr(begin_size, size_pt - begin_size));

									size_t x_size = hex_chunk_size.find("\r\n");
									if (std::string::npos != x_size) hex_chunk_size.erase(x_size, 2);

									util::BinData bin_chunk_size;
									bin_chunk_size.fromHexStr(hex_chunk_size);

									int chunk_size = bin_chunk_size.toInt32_BE();
									remine_data_size = chunk_size;

									if (chunk_size == 0)
									{
										res.data << chunked_result;
										return res;
									}

									std::string data_part = "";
									if (sh_result.length() >= (unsigned int)remine_data_size)
									{
										data_part = sh_result.substr(size_pt + 2, remine_data_size);
									}
									else
									{
										data_part = sh_result.substr(size_pt + 2, sh_result.length());
									}

									chunked_result += data_part;
									remine_data_size = remine_data_size - data_part.length();

									if (remine_data_size > 0)
										break;

									mess_remain_data_size = mess_remain_data_size - (data_part.length() + hex_chunk_size.length() + 4);
								}
								else
								{
									break;
								}
							}
						}

					}
					else
					{
						if (data_size == data_recv_len)
						{
							res.data << result;
							return res;
						}

					}
				}
			}
			catch (...)
			{
				util::writeLogLine("Exeption in getHttpRecvData function", util::message_type.failure, x_src_type);
				res.status = "error";
				res.descr = "Exeption in getHttpRecvData function";
			}
			return res;
		}

	public:
		httpClient(std::string host, std::string port, std::string source_type, bool is_tls = false)
		{
			x_connection = false;
			x_src_type = source_type;
			x_is_ssl = is_tls;
			x_host = host;
			x_port = port;

			open();
		};

		bool open()
		{
			if (!x_connection)
			{
				struct addrinfo addr_hints, * addr_result = NULL;

				memset(&addr_hints, 0, sizeof(addr_hints));
				addr_hints.ai_family = AF_INET;
				addr_hints.ai_socktype = SOCK_STREAM;
				if (x_is_ssl) addr_hints.ai_protocol = IPPROTO_TCP;

				getaddrinfo(x_host.c_str(), x_port.c_str(), &addr_hints, &addr_result);
				if (addr_result == NULL)
				{
					util::writeLogLine("DNS address not resolved", util::message_type.error, x_src_type);
					freeaddrinfo(addr_result);
					return false;
				}

				x_socket = socket(addr_hints.ai_family, addr_hints.ai_socktype, 0);
				if (x_socket < 0)
				{
					util::writeLogLine("Server socket not binded", util::message_type.error, x_src_type);
					freeaddrinfo(addr_result);
					return false;
				}

				int con = connect(x_socket, addr_result->ai_addr, addr_result->ai_addrlen);
				if (con < 0)
				{
					util::closeSocket(x_socket);

					util::writeLogLine("Server socket not connect", util::message_type.error, x_src_type);
					freeaddrinfo(addr_result);
					return false;
				}

				if (x_is_ssl)
				{
					InitSSL_CTX();
					int ssl_status = 0;
					if (ssl != NULL)
					{
						SSL_set_fd(ssl, x_socket);
						SSL_ctrl(ssl, SSL_CTRL_SET_TLSEXT_HOSTNAME, TLSEXT_NAMETYPE_host_name, (void*)x_host.c_str());
						ssl_status = SSL_connect(ssl);
					}

					if (ssl_status != 1)
					{
						util::closeSocket(x_socket);

						util::writeLogLine("SSL_connect to data server failed", util::message_type.error, x_src_type);

						if (ssl != NULL) SSL_free(ssl);
						if (ctx != NULL) SSL_CTX_free(ctx);
					}
					else
					{
						x_connection = true;
					}
				}
				else
				{
					x_connection = true;
				}
			}

			return x_connection;
		}
		
		~httpClient()
		{
			if (x_connection)
			{
				util::closeSocket(x_socket);
				if (x_is_ssl)
				{
					if (ssl != NULL) SSL_free(ssl);
					if (ctx != NULL) SSL_CTX_free(ctx);
				}

				x_connection = false;
			}
		}

		void close()
		{
			if (x_connection)
			{
				util::closeSocket(x_socket);
				if (x_is_ssl)
				{
					if (ssl != NULL) SSL_free(ssl);
					if (ctx != NULL) SSL_CTX_free(ctx);
				}
				x_connection = false;
			}
		}

		httpResponce execQuery(httpRequest request)
		{
			httpResponce res;
			
			if (!x_connection)
			{
				util::writeLogLine("No connection to server", util::message_type.warning, x_src_type);
				res.descr = "No connection to server";
				res.status = "error";
				return res;
			}

			util::BinData http_message;
			http_message << request.type + " " + request.path + " " + request.protocol + "\r\nHost: " + x_host + "\r\nUser-Agent: " + config::SERVER_NAME + "-server-" + config::PROGRAMM_VERSION + "\r\nAccept: */*";

			for (unsigned int i = 0; i < request.headers.size(); i++)
			{
				http_message << "\r\n" + request.headers[i];
			}

			if (request.data.size() > 0)
			{
				http_message << "\r\nContent-Length: " + std::to_string(request.data.size()) + "\r\nContent-Type: " + request.content_type + "\r\n\r\n" << request.data;
			}
			else
			{
				http_message << "\r\n\r\n";
			}

			std::string test = http_message.toString();
				
			unsigned int plist_sended = 0;
			if (x_is_ssl)
			{
				plist_sended = SSL_write(ssl, http_message.data(), http_message.size());
			}
			else
			{
				plist_sended = send(x_socket, (const char*)http_message.data(), http_message.size(), 0);
			}

			if (plist_sended != http_message.size())
			{
				util::writeLogLine("Request for get api data not sended completely", util::message_type.error, x_src_type);
			}

			return getRecvData();
		}
		
		const bool& connection = x_connection;
	};

	httpResponce execRequest(std::string host, std::string port, std::string type, std::string request, std::string source, bool ssl)
	{
		net::httpResponce http_res;
		net::httpClient client(host, port, source, ssl);
		if (!client.connection)
		{
			util::writeLogLine("No connection to server " + host + ":" + port, util::message_type.error, source);
			return http_res;
		}

		net::httpRequest req;
		req.type = type;
		req.path = request;
		http_res = client.execQuery(req);

		client.close();
		return http_res;

	}
}
#endif