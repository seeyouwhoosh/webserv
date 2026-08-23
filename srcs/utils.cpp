#include <iostream>
#include <cstring>
#include "HTTPmessage.hpp"
#include "WebServer.hpp"
#include <cerrno>

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


int	start_epoll_instance(WebServer &webserv)
{
	if ((webserv.epoll_instance = epoll_create(MAX_CONNECTIONS)) < 0)
	{
		webserv.free_all();
		return error_msg(errno, "epoll_create", NULL);
	}
	
	std::cout << "\033[1m[webserv]\33[0m Epoll instance created succesfully" << "\n";

	if (webserv.epoll_add_listens() < 0)
		return -1;

	std::cout << "\033[1m[webserv]\33[0m Epoll added listeners to watch. Total connections: " << webserv.num_connections << "\n";
	return 0;
}