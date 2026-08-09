#include "WebServer.hpp"
#include <unistd.h> // close
#include <fcntl.h> // fcntl

WebServer::WebServer() {}

WebServer::~WebServer() {}

WebServer::WebServer(WebServer &other) {}

WebServer &WebServer::operator=(WebServer &other) {}

void	WebServer::free_all()
{
	for (size_t i = 0; i < servers.size() ; i++)
	{
		for (size_t j = 0; i < servers[i].listens.size(); j++)
		{
			if (servers[i].listens[j].serversock_info)
				freeaddrinfo(servers[i].listens[j].serversock_info);
			if (fcntl(servers[i].listens[j].sockfd, F_GETFD) > -1)
				close(servers[i].listens[j].sockfd);
		}
	}
}

void	WebServer::free_all_addrinfos()
{
	for (size_t i = 0; i < servers.size() ; i++)
	{
		for (size_t j = 0; i < servers[i].listens.size(); j++)
		{
			if (servers[i].listens[j].serversock_info)
				freeaddrinfo(servers[i].listens[j].serversock_info);
		}
	}
}

/* prepares listening socket, if it fails, return value < 0, else returns socket */
int	listen_socket(struct addrinfo *addr, int listen_num)
{
	int	serversock;
	int	status;
	int	yes = 1;

	if (!addr)
		return error_msg(-1, "not a valid struct");

	for (struct addrinfo *p = addr; p != NULL; p = p->ai_next)
	{
		if ((serversock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
		{
			status = errno;
			continue;
		}
		setsockopt(serversock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
		if (bind(serversock, p->ai_addr, p->ai_addrlen) < 0)
		{
			status = errno;
			close(serversock);
			continue;
		}
		if (listen(serversock, listen_num) < 0)
		{
			close(serversock);
			return error_msg(errno, NULL);
		}
		return serversock;
	}
	return error_msg(status, NULL);
}

int	WebServer::prepare_listeners()
{
	int	status;
	struct addrinfo	hints = {0};

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	for (size_t i = 0; i < servers.size() ; i++)
	{
		for (size_t j = 0; i < servers[i].listens.size(); j++)
		{
			if ((status = getaddrinfo(
				servers[i].listens[j].host.c_str(),
				servers[i].listens[j].port.c_str(),
				&hints,
				&servers[i].listens[j].serversock_info)) < 0)
			{
				free_all_addrinfos();
				return (errno, NULL);
			}
			if ((servers[i].listens[j].sockfd = listen_socket(servers[i].listens[j].serversock_info, MAX_CONNECTIONS) < 0))
			{
				free_all();
				return (errno, NULL);
			}
		}
	}
	
}
