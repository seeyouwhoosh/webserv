#include "WebServer.hpp"

#define BUFFER_SIZE 1024

WebServer::WebServer()
{
	epoll_events = new struct epoll_event[MAX_CONNECTIONS];
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
	if (epoll_ctl(epoll_instance, EPOLL_CTL_DEL, fd, 0) < 0)
	{
		free_all();
		return error_msg(errno, "epoll_ctl", NULL);
	}
	close(fd);
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

int	WebServer::epoll_add_listens()
{
	struct epoll_event	event;

	event.events = EPOLLIN;
	for (size_t i = 0; i < servers.size() ; i++)
	{
		for (size_t j = 0; j < servers[i].listens.size(); j++)
		{
			event.data.fd = servers[i].listens[j].sockfd;
			if (epoll_ctl(epoll_instance, EPOLL_CTL_ADD, servers[i].listens[j].sockfd, &event) < 0)
			{
				free_all();
				return error_msg(errno, "epoll_ctl", NULL);
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

	event.events = EPOLLIN;
	event.data.fd = fd;
	if (epoll_ctl(epoll_instance, EPOLL_CTL_ADD, fd, &event) < 0)
		return error_msg(errno, "epoll_ctl", NULL);
	return 0;
}

void	WebServer::create_new_client(int clientsock, int listener)
{
	t_client	new_client;
	
	new_client.fd = clientsock;
	new_client.last_activity = std::time(NULL);
	new_client.keep_alive = 0;
	new_client.max_requests = -1;
	new_client.n_requests = 0;
	
	HTTPmessage	client_message;
	for (size_t i = 0; i < servers.size() ; i++)
	{
		for (size_t j = 0; j < servers[i].listens.size(); j++)
		{
			if (listener == servers[i].listens[j].sockfd)
			{
				HTTPmessage client_message(servers[i]);
				client_message.server_port = servers[i].listens[j].port;
				this->clientBuffers.insert(std::pair<int, HTTPmessage>(clientsock, client_message));
				clients.push_back(new_client);
				return;
			}
		}
	}
	return;
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
	const char	*msg = "Connection to some server accepted\n";

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

	if (send(clientsock, msg, std::strlen(msg), 0) < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
		{
			close(clientsock); 
			return error_msg(errno, "send", NULL);
		}
	}
	create_new_client(clientsock, listener);

	num_connections++;

	return 0;
}

static std::vector<t_client>::iterator	find_client(int fd, std::vector<t_client> clients)
{
	for (std::vector<t_client>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		if (fd == it->fd)
			return it;
	}
	return clients.end();
}

/* Return 1 if the server should keep the connection alive, 0 otherwise */
int	WebServer::keep_alive_client(int fd)
{
	t_client	client = *find_client(fd, clients);

	if (client.keep_alive < 0)
		return 1;
	if (!client.keep_alive)
		return 0;
	time_t		now = time(NULL);
	if ((client.keep_alive > 0 && (now - client.last_activity > client.keep_alive)) || (client.max_requests > 0 && client.n_requests <= client.max_requests))
		return 1;
	return 0;
}

// HANDLE CLIENT REQUESTS

int	WebServer::handle_client_data(int client)
{
	HTTPmessage			&clientBuffer = clientBuffers[client];
	char				buff[BUFFER_SIZE];
	std::stringstream	ss;
	bool				close_connection = false;

	if (!keep_alive_client(client))
		close_connection = true;
	int read_size = recv(client, buff, BUFFER_SIZE - 1, 0);
	if (read_size > 0)
	{
		buff[BUFFER_SIZE] = '\0';
		
/* 		Parser	parser;
		int		parse_status;

		if ((parse_status = parser.parseRequest(clientBuffer)) == PARSE_ERROR)
			clientBuffer.setStatusCode(BAD_REQUEST);
		else if (parse_status == PARSE_INCOMPLETE)
			return 0;
		else if (parse_status == PARSE_COMPLETE)
		{
			clientBuffer.generateResponse();
			clientBuffer.sendResponse(client);
		} */
		


	/* ************************** ERASE ************************** */
	clientBuffer.rq_startline.method = "GET";
	clientBuffer.rq_startline.path = "/";
	clientBuffer.rq_startline.version = "HTTP/1.1";
	clientBuffer.setStatusCode(OK);
	/* ************************** ERASE *************************** */

		clientBuffer.generateResponse();
		clientBuffer.sendResponse(client);

	}
	if (!read_size || close_connection)
	{
		std::cout << "Client closed connection\n";
		if (epoll_ctl(epoll_instance, EPOLL_CTL_DEL, client, NULL) < 0)
		{
			free_all();
			return error_msg(errno, "epoll_ctl", NULL);
		}
		clientBuffers.erase(client);
		clients.erase(find_client(client, clients));
		close(client);
		num_connections--;
	}
	return 0;
}

int	WebServer::process_connections(int num_events)
{
	for (int i = 0; i < num_events ; i++)
	{
		if (epoll_events[i].events & EPOLLERR)
		{
			close_connection(epoll_events[i].data.fd);
			std::cout << "Client error\n";
			continue;
		}
		if (epoll_events[i].events & EPOLLHUP)
		{
			close_connection(epoll_events[i].data.fd);
			std::cout << "Client closed connection\n";
			continue;
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