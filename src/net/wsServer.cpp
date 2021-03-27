/*!
 * Copyright (c) 2015-2018, Andrei Chudinovich
 * Released under MIT license
 * Date: 03.09.2018
 */


#include "wsServer.hpp"
#include "../config.hpp"

void ws::sendWebSockMessage(SOCKET& s_client, std::string responce)
{
	try
	{

		int all_message_size = (int)responce.length(); 

		int message_frames = (all_message_size / 65530) + 1;
		int sb = 0;


		char out_data[65535] = { 0, };


		for (int i = 0; i < message_frames; i++)
		{
			int message_size;
			std::string massage;


			if (responce.length() <= 65530)
			{
				message_size = (int)responce.length();
				massage = responce;
			}
			else
			{
				massage = responce.substr(((size_t)i * (size_t)65530), 65530);
				message_size = (int)massage.length();
			}


			if (message_size < 126)
			{
				int x = 0x81;
				out_data[0] = x; 
				out_data[1] = (char)message_size;
				memcpy(out_data + 2, massage.c_str(), message_size);

				sb = send(s_client, out_data, message_size + 2, 0);
			}
			else
			{
				memcpy(out_data + 4, massage.c_str(), message_size);
				int x = 0x81;
				out_data[0] = x; 
				out_data[1] = 126; 
				out_data[3] = message_size & 0xFF; 
				out_data[2] = (message_size >> 8) & 0xFF; 

				sb = send(s_client, out_data, message_size + 4, 0);
			}

			memset(out_data, 0, sizeof(out_data));

			if (sb < 0)
				break;
		}
	}
	catch (...)
	{
		util::writeLogLine("Exeption in sendWebSockMessage function", util::message_type.failure, config::SOURCE_TYPE.api);
	}

}

void ws::sendMessageAllClientsWS(std::string& massage)
{
	util::writeLogLine("Begin send message all Web socket clients...", util::message_type.message, config::SOURCE_TYPE.api);
	{
		std::lock_guard<std::mutex> wsn(config::websocket_mutex);

		for (unsigned int i = 0; i < config::WEBSOCKET_LIST.size(); i++)
		{
			sendWebSockMessage(config::WEBSOCKET_LIST[i], massage);
		}
	}
	util::writeLogLine("End sending message all Web socket clients...", util::message_type.message, config::SOURCE_TYPE.api);
}

void runWebSocketClient(SOCKET s_client)
{
	try
	{
		unsigned int clients_count = 0;
		{
			std::lock_guard<std::mutex> wsn(config::websocket_mutex);
			config::WEBSOCKET_LIST.push_back(s_client);
			clients_count = config::WEBSOCKET_LIST.size();
		}

		util::writeLogLine("New Web socket client. Opened " + std::to_string(clients_count) + " clients.", util::message_type.message, config::SOURCE_TYPE.api);


		char recvbuf[RECVWSDATASIZE];

		util::BinData total_recv;

		int recv_len = 1;

		while (recv_len > 0)
		{
			try
			{
				memset(recvbuf, 0, RECVWSDATASIZE);
				recv_len = recv(s_client, (char*)&recvbuf, sizeof(recvbuf), 0);

				if (recv_len > 0)
				{
					util::BinData brecv;
					brecv.fromChar((unsigned char*)recvbuf, recv_len);
					total_recv << brecv;

					std::string resdata = total_recv.toString();

					std::string header_reqv = util::strToLower(resdata);

					if (std::string::npos != header_reqv.find("http/1.1"))
					{

						size_t webspos = header_reqv.find("sec-websocket-key:");
						if (std::string::npos != webspos)
						{
							size_t pn = resdata.find("\r\n", webspos);

							std::string wskey = resdata.substr(webspos + 18, pn - webspos - 18);
							wskey = util::strTrim(wskey);
							wskey = wskey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

							unsigned char sha[20];
							util::calc_SHA1(wskey.c_str(), wskey.length(), sha);

							std::string reshash = util::base64_encode((const unsigned char*)&sha[0], sizeof(sha));


							size_t hostpos = header_reqv.find("host:");
							if (std::string::npos == hostpos)
							{
								util::writeLogLine("Invalid request header", util::message_type.error, config::SOURCE_TYPE.api);
								continue;
							}

							size_t pnh = resdata.find("\r\n", hostpos);
							std::string host = resdata.substr(hostpos + 5, pnh - hostpos - 5);
							host = util::strTrim(host);


							std::string header = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nSec-WebSocket-Accept: " + reshash + "\r\nConnection: Upgrade\r\nSec-WebSocket-Version: 13\r\nOrigin: http://" + host + "\r\n\r\n";

							send(s_client, header.c_str(), header.length(), 0);

							total_recv.clear();

						}
						else
						{
							util::writeLogLine("Invalid WebSocket request header", util::message_type.error, config::SOURCE_TYPE.api);
							continue;
						}
					}
					else
					{
						char masking_key[4] = { 0, }; 
						char opcode; 
						int payload_len;  

						opcode = recvbuf[0] & 0x0F;

						payload_len = recvbuf[1] & 0x7F;

						masking_key[0] = recvbuf[2];
						masking_key[1] = recvbuf[3];
						masking_key[2] = recvbuf[4];
						masking_key[3] = recvbuf[5];

						if (opcode == 0x01)
						{

							char message[128] = { 0, };
							int i = 6, pl = 0;
							for (; pl < payload_len; i++, pl++)
							{
								message[pl] = recvbuf[i] ^ masking_key[pl % 4]; 
							}

							std::string mess(message);
							ws::sendMessageAllClientsWS(mess);

						}
						else if (opcode == 0x08)
						{
							util::writeLogLine("Web socket message 0x08", util::message_type.message, config::SOURCE_TYPE.api);
							break;
						}
						else
						{
							util::writeLogLine("Web socket unknown message " + std::to_string(opcode), util::message_type.warning, config::SOURCE_TYPE.api);
							break;
						}
					}
				}
				else
				{
					total_recv.clear();
				}
			}
			catch (...)
			{
				util::writeLogLine("Exeption in runWebSocketClient while block", util::message_type.error, config::SOURCE_TYPE.api);
			}
		}


		{
			std::lock_guard<std::mutex> wsn(config::websocket_mutex);

			config::WEBSOCKET_LIST.erase(std::remove_if(config::WEBSOCKET_LIST.begin(), config::WEBSOCKET_LIST.end(), [s_client](SOCKET& element)
				{return element == s_client; }), config::WEBSOCKET_LIST.end());

			clients_count = config::WEBSOCKET_LIST.size();
		}

		shutdown(s_client, SD_BOTH);
		util::closeSocket(s_client);

		util::writeLogLine("Web socket client closed. Opened " + std::to_string(clients_count) + " clients.", util::message_type.message, config::SOURCE_TYPE.api);
	}
	catch (...)
	{
		util::writeLogLine("Exeption in runWebSocketClient function", util::message_type.failure, config::SOURCE_TYPE.api);

		shutdown(s_client, SD_BOTH);

		util::closeSocket(s_client);
	}
}

void ws::runWebSocketServer()
{
	SOCKET ws_server;
	try
	{

#ifdef WIN32
		WSADATA wsadata;
		WSAStartup(MAKEWORD(2, 2), &wsadata);
#endif

		SOCKET ws_client = 0;
		sockaddr_in ssin;

		ws_server = socket(AF_INET, SOCK_STREAM, 0);
		if (ws_server == SOCKET_ERROR)
		{
			util::writeLogLine("Web socket not created", util::message_type.error, config::SOURCE_TYPE.api);
			return;
		}

		util::writeLogLine("WebSocket server start", util::message_type.message, config::SOURCE_TYPE.api);

		ssin.sin_family = AF_INET;
		ssin.sin_addr.s_addr = INADDR_ANY;
		ssin.sin_port = htons(config::WEBSOCKET_PORT);

		if (bind(ws_server, (sockaddr*)&ssin, sizeof(ssin)) == SOCKET_ERROR)
		{
#ifdef _WIN32
			WSACleanup();
#endif
			util::closeSocket(ws_server);

			util::writeLogLine("Web socket not binded", util::message_type.error, config::SOURCE_TYPE.api);
			return;
		}


		while (listen(ws_server, 50) < 0)
		{
			util::writeLogLine("WebSocket listen failed", util::message_type.error, config::SOURCE_TYPE.api);
			std::this_thread::sleep_for(std::chrono::seconds(5));
		}

		while (true)
		{
			try
			{
				while ((ws_client = accept(ws_server, NULL, NULL)) == INVALID_SOCKET)
				{
					util::writeLogLine("Invalid Web socket", util::message_type.error, config::SOURCE_TYPE.api);
					std::this_thread::sleep_for(std::chrono::seconds(5));
				}

				std::thread webSocketClientThread(runWebSocketClient, ws_client);
				webSocketClientThread.detach();

			}
			catch (...)
			{
				util::writeLogLine("Exeption in runWebSocketServer while block", util::message_type.error, config::SOURCE_TYPE.api);

				util::closeSocket(ws_client);
			}
		}

#ifdef _WIN32
		WSACleanup();
#endif
		util::closeSocket(ws_server);
		util::writeLogLine("WebSocket server stop", util::message_type.message, config::SOURCE_TYPE.api);
	}
	catch (...)
	{
		util::writeLogLine("Exeption in runWebSocketServer function. WebSocket server is stopped", util::message_type.failure, config::SOURCE_TYPE.api);
#ifdef _WIN32
		WSACleanup();
#endif
		util::closeSocket(ws_server);
	}
}
