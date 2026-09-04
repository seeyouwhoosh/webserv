#include "WebServer.hpp"

#define BUFFER_SIZE 1024

WebServer::WebServer()
{
	epoll_events = new struct epoll_event[MAX_CONNECTIONS];
	num_connections = 0;
}

WebServer::~WebServer() {}

WebServer::WebServer(const WebServer &other)
{
	*this = other;
}

WebServer &WebServer::operator=(const WebServer &other)
{
	(void)other;
	return *this;
}

void	WebServer::free_all()
{
	for (size_t i = 0; i < servers.size() ; i++)
	{
		for (size_t j = 0; j < servers[i].listens.size(); j++)
		{
			if (servers[i].listens[j].serversock_info)
			{
				freeaddrinfo(servers[i].listens[j].serversock_info);
				servers[i].listens[j].serversock_info = NULL;
			}
			if (fcntl(servers[i].listens[j].sockfd, F_GETFD) > -1)
				close(servers[i].listens[j].sockfd);
		}
	}
	if (epoll_events)
	{
		delete[] epoll_events;
		epoll_events = NULL;
	}
}

int	WebServer::close_connection(int fd)
{
	if (epoll_del_fd(fd) < 0)
	{
		free_all();
		return -1;
	}
	close(fd);
	num_connections--;
	return 0;
}

void	WebServer::free_all_addrinfos()
{
	for (size_t i = 0; i < servers.size() ; i++)
	{
		for (size_t j = 0; j < servers[i].listens.size(); j++)
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
		return error_msg(-1, "listen_socket", "not a valid struct");

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
			return error_msg(errno, "listen", NULL);
		}
		return serversock;
	}
	return error_msg(status, "listen_socket", NULL);
}

int	WebServer::prepare_listeners()
{
	int				status;
	struct addrinfo	hints = {};

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	for (size_t i = 0; i < servers.size() ; i++)
	{
		for (size_t j = 0; j < servers[i].listens.size(); j++)
		{
			if ((status = getaddrinfo(
				servers[i].listens[j].host.c_str(),
				servers[i].listens[j].port.c_str(),
				&hints,
				&servers[i].listens[j].serversock_info)) < 0)
			{
				free_all_addrinfos();
				return error_msg(errno, "getaddrinfo", NULL);
			}
			if ((servers[i].listens[j].sockfd = listen_socket(servers[i].listens[j].serversock_info, MAX_CONNECTIONS)) < 0)
			{
				free_all();
				return servers[i].listens[j].sockfd;
			}
			std::cout << "\033[1m[webserv]\33[0m " << servers[i].server_name << " listening on " << servers[i].listens[j].host << ":" << servers[i].listens[j].port << "\n";
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

void	WebServer::create_new_client(int clientsock, int listener, struct sockaddr *clientaddr)
{
	t_client	new_client = {
		.fd = clientsock,
		.last_activity = std::time(NULL),
		.keep_alive = 0,
		.max_requests = -1,
		.n_requests = 0
	};
	
	for (size_t i = 0; i < servers.size() ; i++)
	{
		for (size_t j = 0; j < servers[i].listens.size(); j++)
		{
			if (listener == servers[i].listens[j].sockfd)
			{
				HTTPmessage client_message(&servers[i]);
				Parser		client_parser;
				
				client_message.setServerPort(servers[i].listens[j].port);
				client_message.setClientAddress(inet_ntop(clientaddr->ss_family, clientaddr));
				this->clientRequests[clientsock] = client_message;
				this->clientParsers[clientsock] = client_parser;
				clients[clientsock] = new_client;
				return;
			}
		}
	}
}

std::string	WebServer::getPort(int listener)
{
	for (size_t i = 0; i < servers.size() ; i++)
	{
		for (size_t j = 0; j < servers[i].listens.size(); j++)
		{
			if (listener == servers[i].listens[j].sockfd)
				return servers[i].listens[j].port;
		}
	}
	return "";
}

int	WebServer::handle_new_connection(int listener)
{
	struct sockaddr_storage	clientaddr;
	socklen_t				clientlen = sizeof(clientaddr);
	int						clientsock;

	if ((clientsock = accept(listener, (struct sockaddr *)&clientaddr, (socklen_t *)&clientlen)) < 0)
		return error_msg(errno, "accept", NULL);

	if (fcntl(clientsock, F_SETFL, O_NONBLOCK) < 0)
	{
		close(clientsock);
		return error_msg(errno, "fcntl", NULL);
	}

	/******************** TO ERASE ******************* */
	char str[INET6_ADDRSTRLEN] = {0};
	int port = 0;

	if (clientaddr.ss_family == AF_INET)
	{
		struct sockaddr_in *addr4 = (struct sockaddr_in *)&clientaddr;
		inet_ntop(AF_INET, &addr4->sin_addr, str, sizeof(str));
		port = ntohs(addr4->sin_port);
	}
	else if (clientaddr.ss_family == AF_INET6)
	{
		struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&clientaddr;
		inet_ntop(AF_INET6, &addr6->sin6_addr, str, sizeof(str));
		port = ntohs(addr6->sin6_port);
	}
	
	std::cout << "\033[1m[webserv]\33[0m New incoming connection from " 
				<< str << ":" << port << std::endl;
	/******************** TO ERASE ******************* */

	if (epoll_add_fd(clientsock) < 0)
	{
		close(clientsock);
		return -1;
	}

	create_new_client(clientsock, listener, clientaddr);

	return 0;
}

/* Return 1 if the server should keep the connection alive, 0 otherwise */
int	WebServer::keep_alive_client(int fd)
{
	t_client	client = clients[fd];

	if (client.keep_alive < 0)
		return 1;
	if (!client.keep_alive)
		return 0;
	time_t		now = time(NULL);
	if ((client.keep_alive > 0 && (now - client.last_activity > client.keep_alive)) || (client.max_requests > 0 && client.n_requests <= client.max_requests))
		return 1;
	return 0;
}

int	update_client(t_client client)
{
	client.last_activity = std::time(NULL);
	client.n_requests++;
	return 0;
}

// HANDLE CLIENT REQUESTS

int	WebServer::handle_client_data(int client)
{
	HTTPmessage			&clientBuffer = clientRequests[client];
	char				buff[BUFFER_SIZE];
	std::stringstream	ss;

	int read_size = recv(client, buff, BUFFER_SIZE - 1, 0);
	if (read_size > 0)
	{
		buff[read_size] = '\0';
		
	/* 	Parser	&clientParser = clientParsers[clients];
		int		parse_status;

		if ((parse_status = clientParser.parseRequest(clientBuffer)) == PARSE_ERROR)
			clientBuffer.setStatusCode(BAD_REQUEST);
		else if (parse_status == PARSE_INCOMPLETE)
			return 0;
		else if (parse_status == PARSE_COMPLETE)
		{
			clientBuffer.setStatusCode(OK);
			clientBuffer.generateResponse();
			clientBuffer.sendResponse(client);
		} */

	/* ************************** ERASE ************************** */
	clientBuffer.rq_startline.method = "GET";
	clientBuffer.rq_startline.path = "/test/";
	clientBuffer.rq_startline.version = "HTTP/1.1";
	clientBuffer.setStatusCode(OK);
	/* ************************** ERASE *************************** */

		if (clientBuffer.generateResponse() < 0)
		{
			free_all();
			return error_msg(1, "generateResponse", "couldn't generate response");
		}
		if (epoll_addflag_fd(client, EPOLLOUT) < 0)
		{
			free_all();
			return -1;
		}
	}
	if (!read_size)
	{
		std::cout << "Client closed connection\n";

		clientRequests.erase(client);
		clientParsers.erase(client);
		clients.erase(client);
		if (close_connection(client) < 0)
			return -1;
	}
	return 0;
}

int	WebServer::process_connections(int num_events)
{
	for (int i = 0; i < num_events ; i++)
	{
		int fd = epoll_events[i].data.fd;
		if (epoll_events[i].events & EPOLLERR)
		{
			if (close_connection(fd) < 0)
				return -1;
			std::cout << "Client error\n";
			continue;
		}
		if (epoll_events[i].events & EPOLLHUP)
		{
			if (close_connection(fd) < 0)
				return -1;
			std::cout << "Client closed connection\n";
			continue;
		}
		if (epoll_events[i].events & EPOLLOUT)
		{
			HTTPmessage &clientBuffer = clientRequests[fd];
			int	sent_size = clientBuffer.sendResponse(fd);

			if (sent_size < 0)
			{
				if (close_connection(fd) < 0)
					return -1;
				continue;
			}
			if (clientBuffer.isComplete())
			{
				epoll_delflag_fd(fd, EPOLLOUT);
				if (!keep_alive_client(fd))
				{
					if (close_connection(fd) < 0)
						return -1;
				}
				else
					clientBuffer.clear();
			}
		}
		if (epoll_events[i].events & EPOLLIN)
		{
			if (is_listener(epoll_events[i].data.fd))
			{
				if (handle_new_connection(epoll_events[i].data.fd) < 0)
					return -1;
			}
			else if (handle_client_data(epoll_events[i].data.fd) < 0)
				return -1;
		}
	}
	return 0;
}
