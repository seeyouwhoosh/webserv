#include "sockets.hpp"
#include "WebServer.hpp"

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

int	prepare_listeners(const WebServer &webserv)
{
	std::vector<ServerConfig>	servers = webserv.servers;

	for (size_t i = 0; i < ; i++)
	{
		/* code */
	}
	
}