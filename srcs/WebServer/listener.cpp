#include "WebServer.hpp"

int	WebServer::prepare_listeners()
{
	struct addrinfo	hints = {};

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	for (size_t i = 0; i < servers.size() ; i++)
	{
		for (size_t j = 0; j < servers[i].listens.size(); j++)
		{
			if ((getaddrinfo(
				servers[i].listens[j].host.c_str(),
				servers[i].listens[j].port.c_str(),
				&hints,
				&servers[i].listens[j].serversock_info) < 0))
			{
				free_all_addrinfos();
				return error_msg(errno, "getaddrinfo", NULL);
			}
			if ((servers[i].listens[j].sockfd = listen_socket(servers[i].listens[j].serversock_info, MAX_CONNECTIONS)) < 0)
			{
				free_all();
				return -1;
			}
			std::cout << "\033[1m[webserv]\33[0m " << servers[i].server_name[0] << " listening on " << servers[i].listens[j].host << ":" << servers[i].listens[j].port << "\n";
			listeners.push_back(servers[i].listens[j].sockfd);
		}
	}
	return 0;
}

/* If it's an existing fd (listener), returns true, else returns false */
bool	WebServer::is_listener(int fd)
{
	for (size_t i = 0; i < listeners.size(); i++)
	{
		if (fd == listeners[i])
			return true;
	}
	return false;
}