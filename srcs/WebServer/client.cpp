#include "WebServer.hpp"

void WebServer::create_new_client(int clientsock, int listener, void *clientaddr)
{
	for (size_t i = 0; i < servers.size(); i++)
	{
		for (size_t j = 0; j < servers[i].listens.size(); j++)
		{
			if (listener == servers[i].listens[j].sockfd)
			{
				Client client(clientsock);

				client.initHTTPmessage(
					servers[i].listens[j].port,
					inet_ntop(static_cast<struct sockaddr *>(clientaddr)->sa_family, clientaddr),
					servers[i]
				);

				clients[clientsock] = client;
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

int	WebServer::handle_client_data(int client)
{
	Client				&clientUnit = clients[client];
	char				buff[BUFFER_SIZE];
	std::stringstream	ss;

	int read_size = recv(client, buff, BUFFER_SIZE - 1, 0);
	if (read_size > 0)
	{
		buff[read_size] = '\0';
		int		parse_status;
		int		response_status;

		clientUnit.appendData(std::string(buff, read_size));

		parse_status = clientUnit.parse();

		if (parse_status == PARSE_ERROR)
			clientUnit.setStatusCode(BAD_REQUEST);
		else if (parse_status == PARSE_INCOMPLETE)
			return 0;
		else if (parse_status == PARSE_COMPLETE)
			clientUnit.setStatusCode(OK);

		response_status = clientUnit.generateResponse();
		if (response_status < 0)
		{
			free_all();
			return error_msg(1, "generateResponse", "couldn't generate response");
		}

		if (epoll_addflag_fd(client, EPOLLOUT) < 0)
		{
			free_all();
			return -1;
		}
		clientUnit.updateActivity();
	}
	if (!read_size)
	{
		std::cout << "Client closed connection\n";

		clients.erase(client);
		if (close_connection(client) < 0)
			return -1;
	}
	return 0;
}
