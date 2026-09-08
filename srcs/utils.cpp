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

/* convert IPv4 and IPv6 addresses from binary to text form */
std::string	inet_ntop(int af, const void *src)
{
	std::stringstream ss;
	std::string s;

	if (!src)
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

void	toupperString(std::string &s)
{
	for (size_t i = 0; i < s.length(); i++)
		s[i] = std::toupper(s[i]);
}

int	pipePipes(int pipe_in[2], int pipe_out[2])
{
	if (pipe(pipe_out) < 0)
		return -1;

	if (pipe(pipe_in) < 0)
	{
		close(pipe_out[0]);
		close(pipe_out[1]);
		return -1;
	}

	return 0;
}

void	closePipes(int pipe_in[2], int pipe_out[2])
{
	close(pipe_in[0]);
	close(pipe_in[1]);
	close(pipe_out[0]);
	close(pipe_out[1]);
}