#include "WebServer.hpp"

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

int	update_client(t_client &client)
{
	client.last_activity = std::time(NULL);
	client.n_requests++;
	return 0;
}

void	WebServer::create_new_client(int clientsock, int listener, void *clientaddr)
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
				client_message.setClientAddress(inet_ntop(static_cast<struct sockaddr *>(clientaddr)->sa_family, clientaddr));
				this->clientRequests[clientsock] = client_message;
				this->clientParsers[clientsock] = client_parser;
				clients[clientsock] = new_client;
				return;
			}
		}
	}
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

	create_new_client(clientsock, listener, &clientaddr);

	return 0;
}
//
extern std::string	arv;
//

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
	clientBuffer.rq_startline.method = "DELETE";
	clientBuffer.rq_startline.raw_path = arv;
	clientBuffer.rq_startline.version = "HTTP/1.1";
	clientBuffer.setStatusCode(OK);
	std::cout << "file: " << arv << std::endl;

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
		update_client(clients[client]);
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
