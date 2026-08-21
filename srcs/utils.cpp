#include <iostream>
#include <cstring>
#include "HTTPmessage.hpp"

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

std::string	getMIMEtype(std::string ext)
{
	stringmap::iterator it = MIME_TYPES.find(ext);

	if (it == MIME_TYPES.end())
		return "application/octet-stream";

	return it->second;
}