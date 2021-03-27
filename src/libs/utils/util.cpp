/*!
 * Copyright (c) 2015-2018, Andrei Chudinovich
 * Released under MIT license
 * Date: 03.09.2018
 */



#define _CRT_SECURE_NO_WARNINGS
#include "util.h"

#include <random>

using namespace util;

int util::MAIN_LOG = 0;
std::string util::MAIN_LOG_FILE = "app.log";
res_type util::result_type;
ms_type util::message_type;

void util::closeSocket(SOCKET& s_socket)
{
#ifdef _WIN32
	closesocket(s_socket);
#else
	close(s_socket);
#endif
}

std::string util::getDateStampStr(const TimeStampFormat& tsFormat)
{
	const std::time_t timer = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	char datestr[40];
	struct tm *u = localtime(&timer);
	strftime(datestr, 40, timeStampFormats[(int)tsFormat].c_str(), u);
	return datestr;
}

std::string util::getDateStrByStamp(std::time_t timer, const TimeStampFormat& tsFormat)
{
	char datestr[40];
	struct tm* u = localtime(&timer);
	strftime(datestr, 40, timeStampFormats[(int)tsFormat].c_str(), u);
	return datestr;
}

void util::writeLogLine(std::string logmess, std::string mes_type, std::string source_type)
{
	try
	{
		{
			std::lock_guard<std::mutex> lk(log_mutex);

			if (util::MAIN_LOG == 1)
			{
				char dates[40];

				const std::time_t timer = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
				struct tm* u = localtime(&timer);
				strftime(dates, 40, "%d.%m.%Y %H:%M:%S", u);


				std::string logstr = std::string(dates) + "; " + mes_type + "; " + source_type + "; " + logmess;
				std::ofstream logfile;

				logfile.open(util::MAIN_LOG_FILE, std::ios::out | std::ios::app);

				logfile << logstr << "\n";
				logfile.close();
				logstr.clear();
				memset(&dates, 0, 40);
			}
			else
			{
				char dates[40];
				char dt[30];

				const std::time_t timer = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
				struct tm* u = localtime(&timer);
				strftime(dates, 40, "%d.%m.%Y %H:%M:%S", u);
				strftime(dt, 30, "%d-%m-%Y", u);


				std::string logstr = std::string(dates) + "; " + mes_type + "; " + source_type + "; " + logmess;
				std::ofstream logfile;
				logfile.open("logs/" + std::string(dt) + "-log.txt", std::ios::out | std::ios::app);
				logfile << logstr << "\n";
				logfile.close();
				logstr.clear();
				memset(&dates, 0, 40);
				memset(&dt, 0, 30);
			}

		}

		std::cout << logmess << std::endl;

	}
	catch (...)
	{

		std::cout << "Exeption in writeLogLine function" << std::endl;
	}
}

std::string util::lineFeedErase(const std::string& str)
{
	std::string trimmed = str;
	try
	{
		
		size_t ns;
		while ((ns = trimmed.find('\n')) != std::string::npos)
		{
			trimmed.erase(ns, 1);
		}

		size_t rs;
		while ((rs = trimmed.find('\r')) != std::string::npos)
		{
			trimmed.erase(rs, 1);
		}

		
		
		//std::string::size_type pos = str.find_first_not_of("\r");
		//std::string::size_type pos1 = str.find_last_not_of("\r");
		//trimmed = str.substr(pos, pos1 - pos + 1);

		//pos = str.find_first_not_of("\n");
		//pos1 = str.find_last_not_of("\n");
		//trimmed = str.substr(pos, pos1 - pos + 1);


	}
	catch (std::exception e)
	{
		util::writeLogLine("Exeption in lineFeedErase function: " + std::string(e.what()), message_type.failure, "utils      ");
	}
	return trimmed;
}

std::string util::strTrim(const std::string& str)
{
	std::string trimmed = str;
	try
	{
		//std::string::size_type pos = str.find_first_not_of(" ");
		//std::string::size_type pos1 = str.find_last_not_of(" ");
		//trimmed = str.substr(pos, pos1 - pos + 1);


		std::string whitespaces(" \t\f\v\n\r");

		std::size_t last_found = trimmed.find_last_not_of(whitespaces);
		if (last_found != std::string::npos)
			trimmed.erase(last_found + 1);
		else
			trimmed.clear();            // str is all whitespace

		std::size_t first_found = trimmed.find_first_not_of(whitespaces);

		if (first_found != std::string::npos)
			trimmed = trimmed.substr(first_found);
		else
			trimmed.clear();            // str is all whitespace
	}
	catch (std::exception e)
	{
		util::writeLogLine("Exeption in strTrim function: " + std::string(e.what()), message_type.failure, "utils      ");
	}
	return trimmed;
}

std::vector<std::string> util::split(const std::string &string, const std::string &delims)
{
	size_t current_idx = 0;
	size_t to = 0;
	size_t len;
	std::vector<std::string> result;
	try
	{
		while (to != std::string::npos)
		{

			size_t from = current_idx;
			to = string.find(delims, from);
			if (to == std::string::npos)
			{
				len = string.length() - from;
			}
			else
			{
				len = to - from;
			}

			result.push_back(string.substr(from, len));

			current_idx = to + 1;
		}
	}
	catch (std::exception e)
	{
		util::writeLogLine("Exeption in split function: " + std::string(e.what()), message_type.failure, "utils      ");
	}

	return result;
}

int util::getCurrentUnixTime()
{
	return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

long long util::getRand64()
{
	std::random_device rd;
	std::mt19937_64 mersenne(rd());
	return mersenne();
}

int util::getRand()
{
	std::random_device rd;
	std::mt19937 mersenne(rd());
	return mersenne();
}

bool util::isNumber(const std::string&)
{
	return true;
}

signed char util::HexDigit(char c)
{
	return p_util_hexdigit[(unsigned char)c];
}

bool util::IsHex(const std::string& str)
{
	for (std::string::const_iterator it(str.begin()); it != str.end(); ++it)
	{
		if (HexDigit(*it) < 0)
			return false;
	}
	return (str.size() > 0) && (str.size() % 2 == 0);
}

std::vector<unsigned char> util::ParseHex(const char* psz)
{
	// convert hex dump to vector
	std::vector<unsigned char> vch;
	while (true)
	{
		while (isspace(*psz))
			psz++;
		signed char c = HexDigit(*psz++);
		if (c == (signed char)-1)
			break;
		unsigned char n = (c << 4);
		c = HexDigit(*psz++);
		if (c == (signed char)-1)
			break;
		n |= c;
		vch.push_back(n);
	}
	return vch;
}

util::BinData util::ParseHexToBin(const char* psz)
{
	util::BinData res;
	std::vector<unsigned char> p_res = ParseHex(psz);
	res.assign(p_res.begin(), p_res.end());
	return res;
}

std::string util::toHexStr(std::string& str, bool spaces = false)
{
	std::string hex;
	static const char hexmap[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	hex.reserve((str.end() - str.begin()) * 3);
	for (std::string::iterator it = str.begin(); it < str.end(); ++it)
	{
		unsigned char val = (unsigned char)(*it);
		if (spaces && it != str.begin())
			hex.push_back(' ');
		hex.push_back(hexmap[val >> 4]);
		hex.push_back(hexmap[val & 15]);
	}

	return hex;
}

std::string util::getFileFromPath(const std::string& path)
{
	size_t ls = path.find_last_of("\\") + 1;
	size_t rs = path.find_last_of("/") + 1;
	size_t pos = (ls != std::string::npos && ls > rs) ? ls : rs;
	std::string file = path.substr(pos, std::string::npos);
	return file;
}

void util::writeFile(const util::BinData& data, const std::string& filename)
{
	FILE *messfile = fopen(filename.c_str(), "wb");

	if (messfile != NULL)
	{
		fwrite(data.data(), data.size(), 1, messfile);
		fflush(messfile);
		fclose(messfile);
	}
	else
	{
		util::writeLogLine("file " + filename + " not created", message_type.failure, "utils      ");
	}
}

void util::addToFile(const util::BinData& data, const std::string& filename)
{
	FILE *messfile = fopen(filename.c_str(), "ab");

	if (messfile != NULL)
	{
		fwrite(data.data(), data.size(), 1, messfile);
		fflush(messfile);
		fclose(messfile);
	}
	else
	{
		util::writeLogLine("file " + filename + " not created", message_type.failure, "utils      ");
	}
}

util::BinData util::getFileContent(const std::string& path, bool binary = false)
{
	util::BinData res;

	try
	{
		std::string flag = "r";
		if (binary) flag += "b";
		FILE *fdata = fopen(path.c_str(), flag.c_str());
		if (fdata == NULL) return res;

		fseek(fdata, 0, SEEK_END);
		long fsize = ftell(fdata);
		rewind(fdata);

		char * buffer = new char[fsize];
		memset(buffer, 0, fsize);
		fread(buffer, 1, fsize, fdata);

		for (long i = 0; i < fsize; i++)
		{
			res.push_back(buffer[i]);
		}


		fclose(fdata);
		delete[] buffer;
		buffer = NULL;


		return res;
	}
	catch (...)
	{
		util::writeLogLine("getFileList process failed", message_type.failure, "utils      ");
		return res;
	}
}

std::string util::getMakAddress()
{
	std::string DEV_ADDRES = "00:00:00:00:00:00";
#ifdef _WIN32
	char str_addr[19];

	IP_ADAPTER_INFO AdapterInfo[16];
	DWORD dwBufLen = sizeof(AdapterInfo);
	DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
	if (dwStatus == ERROR_SUCCESS)
	{
		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
		while (pAdapterInfo)
		{

			if (pAdapterInfo->Type == MIB_IF_TYPE_ETHERNET)
			{
				for (int i = 0; i < (int)pAdapterInfo->AddressLength; ++i)
				{
					sprintf(&str_addr[i * 3], "%02X%c", pAdapterInfo->Address[i], (i < 5) ? ':' : '\0');
				}
				DEV_ADDRES = str_addr;

				if ((DEV_ADDRES != "") && (DEV_ADDRES != "00:00:00:00:00:00")) break;
			}
			else if (pAdapterInfo->Type == IF_TYPE_IEEE80211)
			{
				for (int i = 0; i < (int)pAdapterInfo->AddressLength; ++i)
				{
					sprintf(&str_addr[i * 3], "%02X%c", pAdapterInfo->Address[i], (i < 5) ? ':' : '\0');
				}
				DEV_ADDRES = str_addr;

			}
			pAdapterInfo = pAdapterInfo->Next;
		}
	}
#else
	char str_addr[19];
	int fd;
	struct ifreq ifr;
	unsigned char *mac;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	ifr.ifr_addr.sa_family = AF_INET;

	const char *iface1 = "eth0";
	strncpy(ifr.ifr_name, iface1, IFNAMSIZ - 1);
	ioctl(fd, SIOCGIFHWADDR, &ifr);
	mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;

	if (!(((mac[3] == 255) || (mac[3] == 0)) && ((mac[4] == 255) || (mac[4] == 0)) && ((mac[5] == 255) || (mac[5] == 0)))) 
	//if (!((!mac[3]) && (!mac[4]) && (!mac[5])))
	{
		sprintf(str_addr, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		//std::cout  << str_addr << std::endl;
		DEV_ADDRES = str_addr;
		close(fd);
		return DEV_ADDRES;
	}

	const char *iface = "enp2s0";
	strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
	ioctl(fd, SIOCGIFHWADDR, &ifr);
	mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;

	if (!(((mac[3] == 255) || (mac[3] == 0)) && ((mac[4] == 255) || (mac[4] == 0)) && ((mac[5] == 255) || (mac[5] == 0)))) 
	//if (!((!mac[3]) && (!mac[4]) && (!mac[5])))
	{
		sprintf(str_addr, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		//std::cout << str_addr <<  std::endl;
		DEV_ADDRES = str_addr;
		close(fd);
		return DEV_ADDRES;
	}

	const char *iface2 = "wlp1s0";
	strncpy(ifr.ifr_name, iface2, IFNAMSIZ - 1);
	ioctl(fd, SIOCGIFHWADDR, &ifr);
	mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;

	if (!(((mac[3] == 255) || (mac[3] == 0)) && ((mac[4] == 255) || (mac[4] == 0)) && ((mac[5] == 255) || (mac[5] == 0)))) 
	//if (!((!mac[3]) && (!mac[4]) && (!mac[5])))
	{
		sprintf(str_addr, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		//std::cout << str_addr << std::endl;
		DEV_ADDRES = str_addr;
		close(fd);
		return DEV_ADDRES;
	}

	const char *iface3 = "l4tbr0";
	strncpy(ifr.ifr_name, iface3, IFNAMSIZ - 1);
	ioctl(fd, SIOCGIFHWADDR, &ifr);
	mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;

	if (!(((mac[3] == 255) || (mac[3] == 0)) && ((mac[4] == 255) || (mac[4] == 0)) && ((mac[5] == 255) || (mac[5] == 0)))) 
	//if (!((!mac[3]) && (!mac[4]) && (!mac[5])))
	{
		sprintf(str_addr, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		//std::cout << str_addr << std::endl;
		DEV_ADDRES = str_addr;
		close(fd);
		return DEV_ADDRES;
	}

	const char *iface0 = "wlan0";
	strncpy(ifr.ifr_name, iface0, IFNAMSIZ - 1);
	ioctl(fd, SIOCGIFHWADDR, &ifr);
	mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;

	if (!(((mac[3] == 255) || (mac[3] == 0)) && ((mac[4] == 255) || (mac[4] == 0)) && ((mac[5] == 255) || (mac[5] == 0)))) 
	//if (!((!mac[3]) && (!mac[4]) && (!mac[5])))
	{
		sprintf(str_addr, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		//std::cout << str_addr << std::endl;
		DEV_ADDRES = str_addr;
		close(fd);
		return DEV_ADDRES;
	}

	close(fd);

#endif

	return DEV_ADDRES;

}

std::string util::base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len)
{
	std::string ret = "";
	try
	{

		int i = 0;
		int j = 0;
		unsigned char char_array_3[3];
		unsigned char char_array_4[4];
		const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

		while (in_len--) {
			char_array_3[i++] = *(bytes_to_encode++);
			if (i == 3) {
				char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
				char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
				char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
				char_array_4[3] = char_array_3[2] & 0x3f;

				for (i = 0; (i < 4); i++)
					ret += base64_chars[char_array_4[i]];
				i = 0;
			}
		}

		if (i)
		{
			for (j = i; j < 3; j++)
				char_array_3[j] = '\0';

			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (j = 0; (j < i + 1); j++)
				ret += base64_chars[char_array_4[j]];

			while ((i++ < 3))
				ret += '=';

		}
	}
	catch (...)
	{
		util::writeLogLine("Exeption in base64_encode function.", message_type.failure, "utils      ");
	}
	return ret;
}

//Hash methods-------------------------------------------------------

inline const unsigned int rol(const unsigned int value, const unsigned int steps)
{
	return ((value << steps) | (value >> (32 - steps)));
}

inline void clearWBuffert(unsigned int* buffert)
{
	for (int pos = 16; --pos >= 0;)
	{
		buffert[pos] = 0;
	}
}

void innerHash(unsigned int* result, unsigned int* w)
{
	unsigned int a = result[0];
	unsigned int b = result[1];
	unsigned int c = result[2];
	unsigned int d = result[3];
	unsigned int e = result[4];

	int round = 0;

#define sha1macro(func,val) \
					{ \
                const unsigned int t = rol(a, 5) + (func) + e + val + w[round]; \
				e = d; \
				d = c; \
				c = rol(b, 30); \
				b = a; \
				a = t; \
					}

	while (round < 16)
	{
		sha1macro((b & c) | (~b & d), 0x5a827999)
			++round;
	}
	while (round < 20)
	{
		w[round] = rol((w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1);
		sha1macro((b & c) | (~b & d), 0x5a827999)
			++round;
	}
	while (round < 40)
	{
		w[round] = rol((w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1);
		sha1macro(b ^ c ^ d, 0x6ed9eba1)
			++round;
	}
	while (round < 60)
	{
		w[round] = rol((w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1);
		sha1macro((b & c) | (b & d) | (c & d), 0x8f1bbcdc)
			++round;
	}
	while (round < 80)
	{
		w[round] = rol((w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1);
		sha1macro(b ^ c ^ d, 0xca62c1d6)
			++round;
	}

#undef sha1macro

	result[0] += a;
	result[1] += b;
	result[2] += c;
	result[3] += d;
	result[4] += e;
}

void util::calc_SHA1(const void* src, const int bytelength, unsigned char* hash)
{
	// Init the result array.
	unsigned int result[5] = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0 };

	// Cast the void src pointer to be the byte array we can work with.
	const unsigned char* sarray = (const unsigned char*)src;

	// The reusable round buffer
	unsigned int w[80];

	// Loop through all complete 64byte blocks.
	const int endOfFullBlocks = bytelength - 64;
	int endCurrentBlock;
	int currentBlock = 0;

	while (currentBlock <= endOfFullBlocks)
	{
		endCurrentBlock = currentBlock + 64;

		// Init the round buffer with the 64 byte block data.
		for (int roundPos = 0; currentBlock < endCurrentBlock; currentBlock += 4)
		{
			// This line will swap endian on big endian and keep endian on little endian.
			w[roundPos++] = (unsigned int)sarray[currentBlock + 3]
				| (((unsigned int)sarray[currentBlock + 2]) << 8)
				| (((unsigned int)sarray[currentBlock + 1]) << 16)
				| (((unsigned int)sarray[currentBlock]) << 24);
		}
		innerHash(result, w);
	}

	// Handle the last and not full 64 byte block if existing.
	endCurrentBlock = bytelength - currentBlock;
	clearWBuffert(w);
	int lastBlockBytes = 0;
	for (; lastBlockBytes < endCurrentBlock; ++lastBlockBytes)
	{
		w[lastBlockBytes >> 2] |= (unsigned int)sarray[lastBlockBytes + currentBlock] << ((3 - (lastBlockBytes & 3)) << 3);
	}
	w[lastBlockBytes >> 2] |= 0x80 << ((3 - (lastBlockBytes & 3)) << 3);
	if (endCurrentBlock >= 56)
	{
		innerHash(result, w);
		clearWBuffert(w);
	}
	w[15] = bytelength << 3;
	innerHash(result, w);

	// Store hash in result pointer, and make sure we get in in the correct order on both endian models.
	for (int hashByte = 20; --hashByte >= 0;)
	{
		hash[hashByte] = (result[hashByte >> 2] >> (((3 - hashByte) & 0x3) << 3)) & 0xff;
	}
}

//-------------------------------------------------------------------------

void util::toHexString(const unsigned char* hash, char* hexstring)
{
	const char hexDigits[] = { "0123456789abcdef" };

	for (int hashByte = 20; --hashByte >= 0;)
	{
		hexstring[hashByte << 1] = hexDigits[(hash[hashByte] >> 4) & 0xf];
		hexstring[(hashByte << 1) + 1] = hexDigits[hash[hashByte] & 0xf];
	}
	hexstring[40] = 0;
}

std::string util::strToUpper(std::string& str)
{
	std::string res = str;
	for (size_t i = 0; i < str.length(); i++)
	{
		res[i] = toupper(str[i]);
	}
	return res;
}

std::string util::strToLower(std::string& str)
{
	std::string res = str;
	for (size_t i = 0; i < str.length(); i++)
	{
		res[i] = tolower(str[i]);
	}
	return res;
}


/*
std::string util::cp1251_to_utf8(const char *str)
{
	std::string res;
	int result_u, result_c;
	result_u = MultiByteToWideChar(1251, 0, str, -1, 0, 0);
	if (!result_u){ return 0; }
	wchar_t *ures = new wchar_t[result_u];
	if (!MultiByteToWideChar(1251, 0, str, -1, ures, result_u)){
		delete[] ures;
		return 0;
	}
	result_c = WideCharToMultiByte(CP_UTF8, 0, ures, -1, 0, 0, 0, 0);
	if (!result_c){
		delete[] ures;
		return 0;
	}
	char *cres = new char[result_c];
	if (!WideCharToMultiByte(CP_UTF8, 0, ures, -1, cres, result_c, 0, 0)){
		delete[] cres;
		return 0;
	}
	delete[] ures;
	res.append(cres);
	delete[] cres;
	return res;
}
*/

std::string util::readFile(std::string file, bool binary = false)
{
	std::string flag = "r";
	if (binary) flag += "b";

	FILE * fresult = fopen(file.c_str(), flag.c_str());
	if (fresult == NULL)
	{
		return ">@m-error@<";
	}

	fseek(fresult, 0, SEEK_END);                            // устанавливаем позицию в конец файла
	long lSize = ftell(fresult);                            // получаем размер в байтах
	rewind(fresult);                                        // устанавливаем указатель в конец файла
	char * buffer = (char*)malloc(sizeof(char) * lSize);    // выделить память для хранения содержимого файла
	if (buffer == NULL)
	{
		return ">@m-error@<";
	}
	long sresult = fread(buffer, 1, lSize, fresult);      // считываем файл в буфер
	if (sresult != lSize)
	{
		return ">@m-error@<";
	}
	std::string result = std::string(buffer, sresult);

	fclose(fresult);
	free(buffer);

	return result;
}

bool util::writeFile(std::string file, std::string flags, std::string& content)
{
	FILE *fout = fopen(file.c_str(), flags.c_str());
	if (fout != NULL)
	{
		fputs(content.c_str(), fout);
		fclose(fout);
		return true;
	}
	else
	{
		return false;
	}

}

void util::charToHex(unsigned char c, unsigned char &hex1, unsigned char &hex2)
{
	hex1 = c / 16;
	hex2 = c % 16;
	hex1 += hex1 <= 9 ? '0' : 'a' - 10;
	hex2 += hex2 <= 9 ? '0' : 'a' - 10;
}

const std::string util::urlEncode(const std::string& str)
{
	std::ostringstream os;
	char t7[3] = { 0 };

	for (std::string::const_iterator ci = str.begin(); ci != str.end(); ++ci)
	{
		if ((*ci >= 'a' && *ci <= 'z') ||
			(*ci >= 'A' && *ci <= 'Z') ||
			(*ci >= '0' && *ci <= '9'))
		{
			os << *ci;
		}
		else if (*ci == ' ')
		{
			os << '+';
		}
		else
		{
			unsigned char h1, h2;
			util::charToHex(*ci, h1, h2);

			t7[0] = toupper(h1);
			t7[1] = toupper(h2);
		
			os << '%' << t7;
		}
	}

	return os.str();
}

std::string util::getNetExt(const std::string& st) {
	size_t pos = st.find_last_of("./");
	if (std::string::npos == pos) return "";
	if (pos <= 0 || st[pos] == '/') return "";
	return st.substr(pos + 1, std::string::npos);
}

std::string util::getMimeType(const std::string& str) {

	std::string ext = util::getNetExt(str);
	std::string result = "";
	if ((ext == "txt") || (ext == "css") || (ext == "html") || (ext == "csv"))
	{
		if (ext == "txt") result = "text/plain";
		else result = "text/" + ext;
	}
	else if ((ext == "json") || (ext == "js") || (ext == "xml") || (ext == "pdf"))
	{
		if (ext == "js") result = "application/javascript";
		else result = "application/" + ext;
	}
	else if ((ext == "gif") || (ext == "jpeg") || (ext == "png") || (ext == "tiff") || (ext == "webp"))
	{
		result = "image/" + ext;
	}
	else if (ext == "svg")
	{
		result = "image/svg+xml";
	}
	else if ((ext == "mpeg") || (ext == "mp4") || (ext == "ogg") || (ext == "quicktime") || (ext == "webm"))
	{
		result = "video/" + ext;
	}

	return result;
}

int util::getTimepointFromData(std::string& date_str)
{
	int year = 0, mon = 0, day = 0, hour = 0, min = 0, sec = 0;

	std::vector<std::string> date_parts = util::split(date_str, "-");
	if (date_parts.size() == 3)
	{
		std::vector<std::string> time_sector = util::split(date_parts[2], " ");
		std::vector<std::string> time_parts;
		if (time_sector.size() == 2)
		{
			time_parts = util::split(time_sector[1], ":");
		}


		year = atoi(date_parts[0].substr(2, 2).c_str()) + 100;
		mon = atoi(date_parts[1].c_str()) - 1;
		day = atoi(time_sector[0].c_str());

		if (time_parts.size() > 0) hour = atoi(time_parts[0].c_str());
		if (time_parts.size() > 1) min = atoi(time_parts[1].c_str());
		if (time_parts.size() > 2) sec = atoi(time_parts[2].c_str());
		time_parts.clear();
	}
	else
	{
		date_parts = util::split(date_str, ".");
		if (date_parts.size() == 3)
		{
			std::vector<std::string> time_sector = util::split(date_parts[2], " ");
			std::vector<std::string> time_parts;
			if (time_sector.size() == 2)
			{
				time_parts = util::split(time_sector[1], ":");
			}

			year = atoi(time_sector[0].substr(2, 2).c_str()) + 100;
			mon = atoi(date_parts[1].c_str()) - 1;
			day = atoi(date_parts[0].c_str());
			if(time_parts.size() > 0) hour = atoi(time_parts[0].c_str());
			if (time_parts.size() > 1) min = atoi(time_parts[1].c_str());
			if (time_parts.size() > 2) sec = atoi(time_parts[2].c_str());
			time_parts.clear();
		}
	}

	struct  tm timeinfo;
	
	timeinfo.tm_year = year;
	timeinfo.tm_mon = mon;
	timeinfo.tm_mday = day;
	timeinfo.tm_hour = hour;
	timeinfo.tm_min = min;
	timeinfo.tm_sec = sec;
	timeinfo.tm_isdst = -1;

	time_t tp = mktime(&timeinfo);

	date_parts.clear();
	
	return tp;
}

std::string util::strUnescape(std::string escaped_str)
{
	std::string unescaped_str = escaped_str;
	try
	{
		size_t n = 0;
		while ((n = unescaped_str.find("\\u", n)) != std::string::npos)
		{
			std::string unicode_char = unescaped_str.substr(n + 2, 4);
			std::vector<unsigned char> simbols = util::ParseHex(unicode_char.c_str());
			std::string simbol;
			simbol.push_back(simbols[1]);
			unescaped_str = unescaped_str.replace(n, 6, simbol);

			//n = n + 6;
		}

		n = 0;
		while ((n = unescaped_str.find("\\\"", n)) != std::string::npos)
		{
			unescaped_str = unescaped_str.erase(n, 1);
			//n = n + 1;
		}

		n = 0;
		while ((n = unescaped_str.find("\\/", n)) != std::string::npos)
		{
			unescaped_str = unescaped_str.erase(n, 1);
			//n = n + 1;
		}

		n = 0;
		while ((n = unescaped_str.find("\\n", n)) != std::string::npos)
		{
			unescaped_str = unescaped_str.erase(n, 2);
			//n = n + 2;
		}

		n = 0;
		while ((n = unescaped_str.find("\\t", n)) != std::string::npos)
		{
			unescaped_str = unescaped_str.erase(n, 2);
			//n + 2;
		}

		n = 0;
		while ((n = unescaped_str.find("\\r", n)) != std::string::npos)
		{
			unescaped_str = unescaped_str.erase(n, 2);
			//n + 2;
		}

	}
	catch (...)
	{
		util::writeLogLine("Exeption in strUnescape function", util::message_type.failure, "utils      ");
	}

	return unescaped_str;
}

int util::getUnixTimeByTime(std::string& time_str, int& timezone)
{
	int unix_time = 0;

	try
	{

		std::vector<std::string> time_parts = util::split(time_str, ":");

		if (time_parts.size() == 3)
		{
			const std::time_t timer = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			struct tm* timeinfo = localtime(&timer);
			timeinfo->tm_hour = atoi(time_parts[0].c_str());
			timeinfo->tm_min = atoi(time_parts[1].c_str());;
			timeinfo->tm_sec = atoi(time_parts[2].c_str());;

			unix_time = mktime(timeinfo);
			unix_time = unix_time - (timezone * 3600);
		}
	}
	catch (...)
	{
		util::writeLogLine("Exeption in getUnixTimeByTime function", util::message_type.failure, "utils      ");
		return 0;
	}

	return unix_time;
}

//srand(util::getCurrentUnixTime());
//return rand();

