#pragma once
#include <sstream>
#include <iostream>
#include <netdb.h>
#include <string>
#include <cstring>


class Socket
{
private:
	struct addrinfo	hints;
	struct addrinfo	*res;
public:
	Socket();
	Socket(const char *name, const char *port);
	Socket(Socket &other);
	Socket	&operator=(Socket &other);
	~Socket();
};

Socket::Socket()
{
	std::memset(&hints, 0, sizeof(hints));
	this->hints.ai_family = AF_UNSPEC;
	this->hints.ai_socktype = SOCK_STREAM;
}

Socket::~Socket()
{
}

int	listen_socket(struct sockaddr *sockaddr, int listen_num);

/* Network byte host to string and viceversa */

int inet_pton(int af, const char *src, void *dst);

const char	*inet_ntop(int af, const void *src, char dst[size], socklen_t size);
