#include "WebServer.hpp"
#include <unistd.h> // close
#include <fcntl.h> // fcntl
#include <algorithm>
#include <sstream>
#include <cerrno>

#define BUFFER_SIZE 1024

WebServer::WebServer() {}

WebServer::~WebServer() {}

WebServer::WebServer(WebServer &other)
{
	*this = other;
}

WebServer &WebServer::operator=(WebServer &other)
{
	(void)other;
	return *this;
}

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
	struct addrinfo	hints = {};

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
			listeners.push_back(servers[i].listens[j].sockfd);
		}
	}
	return 0;
}

int	WebServer::epoll_add_listens()
{
	struct epoll_event	event;

	event.events = EPOLLIN;
	for (size_t i = 0; i < servers.size() ; i++)
	{
		for (size_t j = 0; i < servers[i].listens.size(); j++)
		{
			event.data.fd = servers[i].listens[j].sockfd;
			if (epoll_ctl(epoll_instance, EPOLL_CTL_ADD, servers[i].listens[j].sockfd, &event) < 0)
			{
				free_all();
				return error_msg(errno, NULL);
			}
		}
	}
	num_connections = listeners.size();
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

int	WebServer::epoll_add_fd(int fd)
{
	struct epoll_event	event;

	event.events = EPOLLIN | EPOLLET;
	event.data.fd = fd;
	if (epoll_ctl(epoll_instance, EPOLL_CTL_ADD, fd, &event) < 0)
		return error_msg(errno, NULL);
	return 0;
}

int	WebServer::handle_new_connection(int listener)
{
	struct sockaddr_storage	clientaddr;
	socklen_t				clientlen = sizeof(clientaddr);
	int						clientsock;
	const char	*msg = "Connection to some server accepted\n";

	if ((clientsock = accept(listener, (struct sockaddr *)&clientaddr, (socklen_t *)&clientlen)) < 0)
		return error_msg(errno, NULL);
	if (fcntl(clientsock, F_SETFL, O_NONBLOCK) < 0)
		return error_msg(errno, NULL);

	if (send(clientsock, msg, std::strlen(msg), 0) < 0)
		return error_msg(errno, NULL);

	clients.push_back(clientsock);
	num_connections++;
	epoll_add_fd(clientsock);
	return 0;
}

int	WebServer::handle_client_data(int client)
{
	char				buff[BUFFER_SIZE];
	std::stringstream	ss;

	int read_size = recv(client, buff, BUFFER_SIZE - 1, 0);
	if (read_size > 0)
	{
		buff[read_size] = '\0';
		ss << "Client fd " << client << ": " << buff;
		std::cout << ss.str();

		for (size_t i = 0; i < clients.size(); i++)
		{
			if (clients[i] == client)
				continue;
			if (send(clients[i], ss.str().c_str(), ss.str().length(), 0) < 0)
				return error_msg(errno, NULL);
		}
	}
	else if (read_size == 0)
	{
		std::cout << "Client closed connection\n";
		close(client);
		if (epoll_ctl(epoll_instance, EPOLL_CTL_DEL, client, 0) < 0)
		{
			free_all();
			return error_msg(errno, NULL);
		}
		clients.erase(std::find(clients.begin(), clients.end(), client));
		num_connections--;
	}
	else
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return 0;
		return error_msg(errno, NULL);
	}
	return 0;
}

int	WebServer::process_connections(int num_events)
{
	for (int i = 0; i < num_events ; i++)
	{
		if (epoll_events[i].events & EPOLLHUP)
		{
			std::cout << "Client closed connection\n";
			continue;
		}
		if (epoll_events[i].events & EPOLLIN)
		{
			if (is_listener(epoll_events[i].data.fd) == true && handle_new_connection(epoll_events[i].data.fd) < 0)
				return -1;
			else if (handle_client_data(epoll_events[i].data.fd) < 0)
				return -1;
		}
	}
	return 0;
}