#include <iostream>
#include <cstring>
#include <cerrno>

#include "HTTPmessage.hpp"
#include "WebServer.hpp"

/* outputs error message if msg is not NULL (else, picks from strerror) and return -err_code */
int	error_msg(int err_code, const char *func, const char *msg)
{
	std::cerr << "\033[1;31m[Error] \033[0;31m";
	if (func)
		std::cerr << func << ": ";
	if (msg)
		std::cerr << msg << "\n";
	else
		std::cerr << strerror(err_code) << "\n\033[0m";
	return -err_code;
}

std::string	getFileExtension(std::string file)
{
	size_t dot = file.find_last_of('.');
	size_t slash = file.find_last_of('/');

	if (dot == std::string::npos || dot < slash + 1)
		return "plain";

	return file.substr(dot + 1);
}

int	hexToAscii(std::string s)
{
	int result = 0;
	int base = 1;

	for (std::string::reverse_iterator it = s.rbegin(); it != s.rend(); ++it)
	{
		char c = std::toupper(static_cast<unsigned char>(*it));
		int val = 0;

		if (c >= 'A' && c <= 'F')
			val = c - 'A' + 10;
		else if (c >= '0' && c <= '9')
			val = c - '0';
		else
			return -1;
		result += val * base;
		base *= 16;
	}
	return result;
}

bool file_is_empty(std::fstream &file)
{
	return file.peek() == std::ios::traits_type::eof();
}

std::string	getDate()
{
	time_t		rawtime;
	struct tm	*tm;

	time(&rawtime);
	tm = gmtime(&rawtime);

	if (!tm)
		return "";

	char buffer[30];
	if (strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", tm) == 0)
		return "";

	return std::string(buffer);
}

static int	valid_ip(int af, const char *src)
{
	size_t	char_count = 0;

	if (af == AF_INET)
	{
		for (size_t i = 0; src[i] != '\0'; i++)
		{
			if (!std::isdigit(src[i] - '0') && src[i] != '.')
				return 0;
			if (src[i] == '.')
				char_count++;
		}
		if (char_count != 4)
			return 0;
		return 1;
	}
	else if (af == AF_INET6)
	{
		for (size_t i = 0; src[i] != '\0'; i++)
		{
			if (!std::isxdigit(src[i] - '0') && src[i] != ':')
				return 0;
			if (src[i] == ':')
				char_count++;
		}
		if (char_count != 6)
			return 0;
		return 1;
	}
	else
		return 0;
}

static std::string	parseIPv6(const char *src)
{
	std::string	ip(src);
	size_t		pos = ip.find("::");

	if (pos != std::string::npos)
	{
		int dotdot = std::count(ip.begin(), ip.end(), ':');
		int to_add = 8 - (dotdot - 1);
		std::string blank = ":";
		for (int i = 0; i < to_add; ++i)
			blank += "0:";	
		ip.replace(pos, 2, blank);
	}
	return ip;
}


/* convert IPv4 and IPv6 addresses from binary to text form */
std::string	inet_ntop(int af, const void *src)
{
	std::stringstream ss;
	std::string s;

	if (!srs)
		return "";

	if (af == AF_INET)
	{
		const struct in_addr	*addr4 = static_cast<const struct in_addr *>(src);
		uint32_t				addr = ntohl(addr4->s_addr);

		ss << ((addr >> 24) & 255 ) << "." << ((addr >> 16) & 255 ) << "." << ((addr >> 8) & 255 ) << "." << (addr & 255 );
	}
	else if (af == AF_INET6)
	{
		const struct in6_addr *addr6 = static_cast<const struct in6_addr *>(src);
	
		for (size_t i = 0; i < 16; i+=2)
		{
			uint16_t group = (addr6->s6_addr[i] << 8) | addr6->s6_addr[i + 1];
			ss << std::hex << group;
			if (i != 14)
				ss << ":";
		}
	}
	else
		return "";
	s = ss.str();

	return s;
}
