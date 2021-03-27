/*!
 * Copyright (c) 2015-2018, Andrei Chudinovich
 * Released under MIT license
 * Date: 03.09.2018
 */


#ifndef UTIL_H
#define UTIL_H

#pragma once

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>
#include <mutex>
#include <vector>
#include <memory>
#include <utility>
#include <list>
#include <sstream>
#include <ios>
#include <cstring>


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

#define SOCKET int
#endif

namespace util
{

		extern int MAIN_LOG;
		extern std::string MAIN_LOG_FILE;

		struct ms_type {
			std::string message = ":) Message";
			std::string warning = ":| Warning";
			std::string error = ":( Error  ";
			std::string failure = ":O Failure";

		};

		struct res_type {
			std::string success = "success";
			std::string error = "error";
		};

		extern res_type result_type;
		extern ms_type message_type;

		bool isNumber(const std::string&);

		const signed char p_util_hexdigit[256] =
		{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1,
		-1, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, };

		signed char HexDigit(char);

		bool IsHex(const std::string&);

		std::vector<unsigned char> ParseHex(const char*);

		std::string toHexStr(std::string&, bool);

		class BinData : public std::vector<unsigned char>
		{
		public:


			BinData& operator<<(const std::string& str)
			{
				(*this).insert((*this).end(), str.c_str(), str.c_str() + str.length());
				return (*this);
			}

			BinData& operator=(std::string& str)
			{
				(*this).clear();
				(*this).assign(str.begin(), str.end());
				return (*this);
			}

			BinData& operator<<(std::vector<unsigned char>& vData)
			{
				(*this).insert((*this).end(), vData.data(), vData.data() + vData.size());
				return (*this);
			}

			BinData& operator=(std::vector<unsigned char>& vData)
			{
				(*this).clear();
				(*this).assign(vData.begin(), vData.end());
				return (*this);
			}

			BinData& operator<<(BinData& vData)
			{
				(*this).insert((*this).end(), vData.data(), vData.data() + vData.size());
				return (*this);
			}

			BinData& operator<<(unsigned char chr)
			{
				(*this).push_back(chr);
				return (*this);
			}

			//BinData& operator()(unsigned char* chr, int size)
			//{
			//	(*this).clear();
			//	for (int i = 0; i < size; i++)
			//	{
			//		(*this).push_back(chr[i]);
			//	}
			//	return (*this);
			//}


			BinData& fromChar(unsigned char* chr, int size)
			{
				(*this).clear();
				for (int i = 0; i < size; i++)
				{
					(*this).push_back(chr[i]);
				}
				return (*this);
			}

			//BinData& operator()(unsigned char* chr, unsigned int begin, int size)
			//{
			//	(*this).clear();
			//	for (unsigned int i = begin; i < (begin + size); i++)
			//	{
			//		(*this).push_back(chr[i]);
			//	}
			//	return (*this);
			//}

			BinData& fromChar(unsigned char* chr, unsigned int begin, int size)
			{
				(*this).clear();
				for (unsigned int i = begin; i < (begin + size); i++)
				{
					(*this).push_back(chr[i]);
				}
				return (*this);
			}


			BinData& operator>>(std::string& str)
			{
				str.insert(str.end(), (*this).data(), (*this).data() + (*this).size());
				return (*this);
			}

			BinData& operator>>(unsigned char chr[])
			{
				unsigned int end = 0;
				for (unsigned int i = 0; i < (*this).size(); i++)
				{
					if (chr[i] == 0) { end = i; break; }
				}

				for (unsigned int i = end; i < (*this).size(); i++)
				{
					if ((*this).size() == (i - end)) break;
					chr[i] = (*this)[i - end];
				}
				return (*this);
			}

			const BinData operator()(int begin, int length) const
			{
				BinData res;
				int _length = 0;
				if ((size_t)(begin + length) > (*this).size()) _length = (*this).size();
				else _length = begin + length;

				res.assign((*this).begin() + begin, (*this).begin() + _length);
				return res;
			}

			const BinData operator()(int begin) const
			{
				BinData res;
				res.assign((*this).begin() + begin, (*this).end());
				return res;
			}

			const std::string toString(int begin, int length) const
			{
				std::string res;
				int _length = 0;
				if ((unsigned int)(begin + length) > (*this).size()) _length = (*this).size();
				else _length = length;

				for (int i = 0; i < _length; i++)
				{
					res.push_back((*this)[begin + i]);
				}

				return res;
			}

			const std::string toString() const
			{
				std::string res;
				res.assign((*this).begin(), (*this).end());
				return res;
			}

			const int strToInt(int begin, int length) const
			{
				int res;
				if (util::isNumber((*this).toString(begin, length))) res = atoi((*this).toString(begin, length).c_str());
				return res;
			}

			const int strToInt() const
			{
				int res;
				if (isNumber((*this).toString())) res = atoi((*this).toString().c_str());
				return res;
			}

			const long strToLong(int begin, int length) const
			{
				long res;
				if (util::isNumber((*this).toString(begin, length))) res = atol((*this).toString(begin, length).c_str());
				return res;
			}

			const long strToLong() const
			{
				long res;
				if (util::isNumber((*this).toString())) res = atol((*this).toString().c_str());
				return res;
			}

			const std::string toHexStr(bool spaces = false)
			{
				std::string hex;
				static const char hexmap[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
				hex.reserve(((*this).end() - (*this).begin()) * 3);
				for (BinData::iterator it = (*this).begin(); it < (*this).end(); ++it)
				{
					unsigned char val = (unsigned char)(*it);
					if (spaces && it != (*this).begin())
						hex.push_back(' ');
					hex.push_back(hexmap[val >> 4]);
					hex.push_back(hexmap[val & 15]);
				}

				return hex;
			}

			const BinData fromHexStr(const std::string& str)
			{
				std::string hex_str(str);   
				int adv_zero_count = 8 - hex_str.length();
				if (adv_zero_count > 0)
				{
					for (int i = 0; i < adv_zero_count; i++)
					{
						hex_str = "0" + hex_str;
					}
				}

				std::vector<unsigned char> res = ParseHex(hex_str.c_str());
				(*this).assign(res.begin(), res.end());
				return (*this);
			}

			const BinData fromInt8(const uint8_t& val)
			{
				BinData vData;
				vData.push_back(val);

				(*this).insert((*this).end(), vData.begin(), vData.end());
				return (*this);
			}

			const BinData fromInt16_LE(const uint16_t& val)
			{
				BinData vData;
				vData.push_back(val);
				vData.push_back(val >> 8);

				(*this).insert((*this).end(), vData.begin(), vData.end());
				return (*this);
			}

			const BinData fromInt32_LE(const int32_t& val)
			{
				BinData vData;
				vData.push_back(val);
				vData.push_back(val >> 8);
				vData.push_back(val >> 16);
				vData.push_back(val >> 24);

				(*this).insert((*this).end(), vData.begin(), vData.end());
				return (*this);
			}

			const BinData fromInt64_LE(const uint64_t& val)
			{
				BinData vInts[2];
				vInts[0].fromInt32_LE(val);
				vInts[1].fromInt32_LE(val >> 32);

				std::vector<unsigned char> vData;
				(*this).insert((*this).end(), vInts[0].begin(), vInts[0].end());
				(*this).insert((*this).end(), vInts[1].begin(), vInts[1].end());

				return (*this);
			}

			const BinData fromInt16_BE(const uint16_t& val)
			{
				BinData vData;
				vData.push_back(val >> 8);
				vData.push_back(val);

				(*this).insert((*this).end(), vData.begin(), vData.end());
				return (*this);
			}

			const BinData fromInt32_BE(const uint32_t& val)
			{
				BinData vData;
				vData.push_back(val >> 24);
				vData.push_back(val >> 16);
				vData.push_back(val >> 8);
				vData.push_back(val);

				(*this).insert((*this).end(), vData.begin(), vData.end());
				return (*this);
			}

			const BinData fromInt64_BE(const uint64_t& val)
			{
				BinData vInts[2];
				vInts[0].fromInt32_BE(val >> 32);
				vInts[1].fromInt32_BE(val);

				std::vector<unsigned char> vData;
				(*this).insert((*this).end(), vInts[0].begin(), vInts[0].end());
				(*this).insert((*this).end(), vInts[1].begin(), vInts[1].end());

				return (*this);
			}


			const uint8_t toInt8() const
			{
				uint32_t res;
				res = (*this)[0];
				return res;
			}

			const uint16_t toInt16_LE() const
			{
				uint32_t res;
				res = ((*this)[1] << 8) + (*this)[0];
				return res;
			}

			const uint32_t toInt32_LE() const
			{
				uint32_t res;
				res = ((*this)[3] << 24) + ((*this)[2] << 16) + ((*this)[1] << 8) + (*this)[0];
				return res;
			}

			const uint64_t toInt64_LE() const
			{
				uint32_t nFirst, nSecond;
				uint64_t res;

				nFirst = (*this)(0, 4).toInt32_LE();
				nSecond = (*this)(4, 4).toInt32_LE();
				res = (nFirst | ((uint64_t)nSecond << 32));

				return res;
			}

			const uint32_t toInt16_BE() const
			{
				uint32_t res;
				res = ((*this)[0] << 8) + (*this)[1];
				return res;
			}

			const uint32_t toInt32_BE() const
			{
				uint32_t res;
				res = ((*this)[0] << 24) + ((*this)[1] << 16) + ((*this)[2] << 8) + (*this)[3];
				return res;
			}

			const uint64_t toInt64_BE() const
			{
				uint32_t nFirst, nSecond;
				uint64_t res;

				nFirst = (*this)(0, 4).toInt32_BE();
				nSecond = (*this)(4, 4).toInt32_BE();
				res = (((uint64_t)nFirst << 32) | (nSecond));

				return res;
			}

			const size_t find(BinData data)
			{
				size_t res = std::string::npos;;
				for (unsigned int i = 0; i < (*this).size(); i++)
				{
					unsigned int res_count = 0;
					for (unsigned int ix = 0; ix < data.size(); ix++)
					{
						if ((*this)[i + ix] == data[ix])
						{
							if (ix == 0) res = i;
							res_count++;
						}
						else break;
					}

					if (res_count == data.size()) break;
				}
				return res;
			}

			const size_t find(BinData data, int begin)
			{
				size_t res = std::string::npos;
				bool finded = false;
				for (unsigned int i = begin; i < (*this).size(); i++)
				{
					unsigned int res_count = 0;
					for (unsigned int ix = 0; ix < data.size(); ix++)
					{
						if ((*this)[i + ix] == data[ix])
						{
							if (ix == 0) res = i;
							res_count++;
						}
						else break;
					}

					if (res_count == data.size())
					{
						finded = true;
						break;
					}
				}
				if (finded) return res;
				else return std::string::npos;
				
			}

			const size_t find(std::string str)
			{
				size_t res = std::string::npos;
				bool finded = false;
				for (unsigned int i = 0; i < (*this).size(); i++)
				{
					unsigned int res_count = 0;
					for (unsigned int ix = 0; ix < str.length(); ix++)
					{
						if ((*this)[i + ix] == str[ix])
						{
							if (ix == 0) res = i;
							res_count++;
						}
						else break;
					}

					if (res_count == str.length())
					{
						finded = true;
						break;
					}
				}

				if (finded) return res;
				else return std::string::npos;
			}

			const size_t find(std::string str, int begin)
			{
				size_t res = std::string::npos;;
				for (unsigned int i = begin; i < (*this).size(); i++)
				{
					unsigned int res_count = 0;
					for (unsigned int ix = 0; ix < str.length(); ix++)
					{
						if ((*this)[i + ix] == str[ix])
						{
							if (ix == 0) res = i;
							res_count++;
						}
						else break;
					}

					if (res_count == str.length()) break;
				}
				return res;
			}

			const void replace(int begin, int num, std::string& data_str)
			{

				(*this).erase((*this).begin() + begin, (*this).begin() + (num + begin));

				for (unsigned int ix = 0; ix < data_str.length(); ix++)
				{
					(*this).insert((*this).begin() + (begin + ix), data_str[ix]);
				}
			}
		};

		BinData ParseHexToBin(const char* psz);


		enum class TimeStampFormat : int {
			X_DATE = 0,
			X_DATETIME = 1,
			X_TIME = 2,
			XX_DATE = 3,
			XX_DATETIME = 4,
		};
		static std::string timeStampFormats[] = {
			"%d.%m.%Y",
			"%d.%m.%Y %H:%M:%S",
			"%H:%M:%S",
			"%Y-%m-%d",
			"%Y-%m-%d %H:%M:%S"
		};

		static std::mutex log_mutex;

		template <typename T>
		bool inList(std::list<T>& list, const T& element)
		{
			auto it = std::find(list.begin(), list.end(), element);
			return it != list.end();
		}

		template <typename T>
		bool inVector(std::vector<T>& list, const T& element)
		{
			auto it = std::find(list.begin(), list.end(), element);
			return it != list.end();
		}

		template <typename I>
		std::string intToHex(I w, size_t hex_len = sizeof(I) << 1) {
			static const char* digits = "0123456789ABCDEF";
			std::string rc(hex_len, '0');
			for (size_t i = 0, j = (hex_len - 1) * 4; i<hex_len; ++i, j -= 4)
				rc[i] = digits[(w >> j) & 0x0f];
			return rc;
		}

		std::string getDateStampStr(const TimeStampFormat&);
		std::string getDateStrByStamp(std::time_t, const TimeStampFormat&);
		void writeLogLine(std::string, std::string, std::string);
		void closeSocket(SOCKET&);
		std::string strTrim(const std::string&);
		std::string lineFeedErase(const std::string&);
		std::vector<std::string> split(const std::string&, const std::string&);
		int getCurrentUnixTime();
		long long getRand64();
		int getRand();
		bool isNumber(const std::string&);
		std::string getFileFromPath(const std::string&);
		void writeFile(const BinData&, const std::string&);
		void addToFile(const BinData&, const std::string&);
		BinData getFileContent(const std::string&, const bool);
		std::string getMakAddress();
		std::string base64_encode(unsigned char const*, unsigned int);
		void calc_SHA1(const void*, const int, unsigned char*);
		void toHexString(const unsigned char*, char*);
		std::string strToUpper(std::string&);
		std::string strToLower(std::string&);
		//std::string cp1251_to_utf8(const char *);
		std::string readFile(std::string, const bool);
		bool writeFile(std::string, std::string, std::string&);
		const std::string urlEncode(const std::string&);
		std::string getNetExt(const std::string&);
		std::string getMimeType(const std::string&);
		void charToHex(unsigned char, unsigned char&, unsigned char&);
		int getTimepointFromData(std::string&);
		std::string strUnescape(std::string);
		int getUnixTimeByTime(std::string&, int&);

}
#endif