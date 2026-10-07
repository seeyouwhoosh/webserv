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

	if (epoll_add_fd(clientsock) < 0)
	{
		close(clientsock);
		return -1;
	}

	create_new_client(clientsock, listener, &clientaddr);

	if (SHOW_LOGS)
		std::cout << "Opened connection for client " << clientsock << std::endl;

	return 0;
}

int	WebServer::send_data_client(int client)
{
	int	sent_size = clients[client].sendResponse();

	if (sent_size < 0)
	{
		if (close_connection(client) < 0)
			return -1;
		return 0;
	}
	if (clients[client].responseComplete())
	{
		epoll_delflag_fd(client, EPOLLOUT);
		if (!clients[client].askkeepAlive())
		{
			if (close_connection(client) < 0)
				return -1;
			if (SHOW_LOGS)
				std::cout << "Client " << client << " closed connection\n";
		}
	}
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
		{
			clientUnit.setStatusCode(OK);

			std::string sessionId = clientUnit.getSessionId();

			if (sessionId.empty() || !hasSession(sessionId))
			{
				sessionId = createSession();
				clientUnit.setSessionCookie(sessionId);
			}
			else
			{
				sessions[sessionId].visits++;
			}
		}

		response_status = clientUnit.generateResponse();
		if (response_status < 0)
		{
			free_all();
			return error_msg(1, "generateResponse", "couldn't generate response");
		}

		clientUnit.setResponse();
		clientUnit.clearParser();
		
		if (epoll_addflag_fd(client, EPOLLOUT) < 0)
		{
			free_all();
			return -1;
		}

		clientUnit.updateActivity();
	}
	if (!read_size)
	{
		clients.erase(client);
		if (close_connection(client) < 0)
			return -1;
		if (SHOW_LOGS)
			std::cout << "Client " << client << " closed connection\n";
	}

	return 0;
}
