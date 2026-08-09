#pragma once
#include <vector>
#include <sstream>
#include <iostream>
#include <netdb.h>
#include <string>
#include <cstring>

#include "Config.hpp"

#define MAX_CONNECTIONS	512

class WebServer
{
private:
	
public:
	WebServer();
	WebServer(WebServer &other);
	WebServer &operator=(WebServer &other);
	~WebServer();

	GlobalConfig				global;
	std::vector<ServerConfig>	servers;

	struct pollfd				*poll_fds;

	int		prepare_listeners();
	void	free_all();
	void	free_all_addrinfos();
};

int	listen_socket(struct addrinfo *addr, int listen_num);

int inet_pton(int af, const char *src, void *dst);

const char	*inet_ntop(int af, const void *src, char *dst, socklen_t size);


/* GENERAL UTILS */

int	error_msg(int err_code, const char *msg);
