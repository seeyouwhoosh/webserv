/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xwu <xwu@student.42urduliz.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/29 22:20:15 by xwu               #+#    #+#             */
/*   Updated: 2026/07/29 22:20:15 by xwu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

/**
 * @brief Checks all active client connections for inactivity timeouts.
 *
 * Compares the current timestamp against each client's last recorded activity.
 * If a client has been idle longer than the configured global timeout, its
 * connection is closed and removed from the active clients pool.
 * @note There are logs messages
 */
void WebServer::check_timeouts()
{
	time_t								now = std::time(NULL);
	std::map<int, Client>::iterator		it = clients.begin();
	
	while (it != clients.end())
	{
		int		client_fd = it->first;
		Client	&client = it->second;
		bool	deleted = false;
		
		
		if (client.cgi.isRunning() && static_cast<size_t>(now - client.cgi.last_activity) > client.getServer()->cgi_timeout)
		{
			client.cgi.killScript();
			
			if (client.generateErrorResponse(GATEWAY_TIMEOUT) >= 0)
				client.setResponse();
		}
		if (static_cast<size_t>(now - client.getLastActivity()) > client.getServer()->timeout)
		{
			if (SHOW_LOGS)
				std::cout << "Client " << client_fd << " timed out\n";
			
			if (client.cgi.isRunning())
				client.cgi.killScript();
				
			close_connection(client_fd);
			clients.erase(it++);
			deleted = true;
		}
		
		if (!deleted)
			++it;
	}
}

/**
 * @brief Created a new Client with its corresponding HTTPmessage initialization
 */
void WebServer::create_new_client(int clientsock, int listener, void *clientaddr)
{
	std::map<int, t_listener>::iterator it = listeners.find(listener);
	if (it == listeners.end())
		return;
	
	ServerConfig	*server = it->second.server;
	t_listen		*listen = it->second.listen;

	struct sockaddr	*addr = static_cast<struct sockaddr *>(clientaddr);

	Client	&client = clients[clientsock];
	client.setSocket(clientsock);
	client.cgi.setClientAddress(inet_ntop(addr->sa_family, addr));
	client.cgi.setServerPort(listen->port);
	client.initHTTPmessage(server);
}

/**
 * @brief Creates a socket for the new client, and sets it non blocking and adds it to epoll
 * @note There are logs messages
 */
int	WebServer::handle_new_connection(int listener)
{
	struct sockaddr_storage	clientaddr;
	socklen_t				clientlen = sizeof(clientaddr);
	int						clientsock;

	if ((clientsock = accept(listener,
							(struct sockaddr *) &clientaddr,
							(socklen_t *)&clientlen)) < 0)
		return error_msg(errno, "accept", NULL);

	if (setNonBlocking(clientsock) < 0)
			return error_msg(errno, "fcntl", NULL);

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

/**
 * @brief Sends messages from queue
 * 
 * Sends message from the response queue, is the sent_size < 0, closes connection; else see if the queue is done, if it is, deletes the EPOLLOUT and see if CLient should stay alive
 * @return 0 no error; <0 error
 * @note There are logs messages
 */
int	WebServer::send_data_client(int client)
{
	int	sent_size = clients[client].sendResponse();
	clients[client].setLastActivity(std::time(NULL));
	if (sent_size < 0)
	{
		if (close_connection(client) < 0)
			return error_msg(errno, "send", NULL);
		return -1;
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

/**
 * @brief Receives data from client and generates response
 * 
 * Checks first if client should have closed. Then, receives data and parse it, if not complete, continues open to reveive.
 * If complete, generates a response according to the statusCode. And cleans up after
 * 
 * @note Manages the cookies too
 * @note There are logs messages
 */
int	WebServer::handle_client_data(int client)
{
	Client	&clientUnit = clients[client];
	if (!clientUnit.askkeepAlive())
	{
		if (close_connection(client) < 0)
			return -1;
		if (SHOW_LOGS)
			std::cout << "Client " << client << " closed connection due to timeout\n";
		return 0;
	}
	char				buff[BUFFER_SIZE];
	std::stringstream	ss;

	int read_size = recv(client, buff, BUFFER_SIZE - 1, 0);
	if (!read_size)
	{
		clients.erase(client);
		if (close_connection(client) < 0)
			return -1;
		if (SHOW_LOGS)
			std::cout << "Client " << client << " closed connection\n";
		return 0;
	}
	if (read_size > 0)
	{
		clientUnit.setLastActivity(std::time(NULL));

		buff[read_size] = '\0';
		int		parse_status;

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
				sessions[sessionId].visits++;
		}

		int		response_status;
		if ((response_status = clientUnit.generateResponse()) < 0)
		{
			free_all();
			return error_msg(1, "generateResponse", "couldn't generate response");
		}
		else if (response_status == IS_CGI)
		{
			int pipe_in = clientUnit.cgi.getPipeIn();
			int pipe_out = clientUnit.cgi.getPipeOut();

			epoll_add_fd(pipe_out);
			epoll_addflag_fd(pipe_out, EPOLLIN);
			cgi_pipes[pipe_out] = &clientUnit;

			if (!clientUnit.cgi.input.empty())
			{
				epoll_add_fd(pipe_in);
				epoll_addflag_fd(pipe_in, EPOLLOUT);
				cgi_pipes[pipe_in] = &clientUnit;
			}
			else
			{
				close(pipe_in);
				clientUnit.cgi.setPipeIn(-1);
			}
			
			return 0;
		}
		if (clientUnit.setResponse() < 0)
			return -1;
		if (epoll_addflag_fd(client, EPOLLOUT) < 0)
		{
			free_all();
			return -1;
		}
	}
	return 0;
}

int	WebServer::send_cgi_data(int fd, Client *client)
{
	CGIHandler	*cgi = &client->cgi;

	ssize_t sent = write(fd, cgi->input.data(), cgi->input.size());

	if (sent < 0)
	{
		close_pipe(fd);
		close_pipe(cgi->getPipeOut());

		if (client->generateErrorResponse(INTERNAL_SERVER_ERROR) < 0 || client->setResponse() < 0 || epoll_addflag_fd(client->fd, EPOLLOUT) < 0)
			return -1;
		return 0;
	}

	if (sent > 0)
	{
		cgi->last_activity = std::time(NULL);
		cgi->input.erase(0, sent);
	}
	if (cgi->input.empty())
		close_pipe(fd);

	return 0;
}

/**
 * @brief Handles the response generated by the cgi script
 */
int WebServer::handle_cgi_data(int fd, Client *client)//change
{
	char buffer[BUFFER_SIZE + 1];
	ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE);

	// Todavia hay datos que leer del CGI.
	if (bytes_read > 0)
		client->cgi.output.append(buffer, bytes_read);
	else if (bytes_read < 0) // Se ha producido un error al leer el pipe.
	{
		close_pipe(fd);
		client->cgi.setRunning(false);

		if (client->generateErrorResponse(INTERNAL_SERVER_ERROR) < 0 || client->setResponse() < 0 || epoll_addflag_fd(client->fd, EPOLLOUT) < 0)
			return -1;
	}
	else // EOF: el CGI ha terminado de escribir su salida.
	{
		close_pipe(fd);
	
		int status;
		pid_t result = waitpid(client->cgi.getPID(), &status, 0);

		// Comprobamos que el proceso CGI ha terminado correctamente.
		if (result > 0 && WIFEXITED(status) && WEXITSTATUS(status) == 0)
		{
			client->cgi.setRunning(false);
			client->cgi.parseOutput(client->message);
			client->message.setBody(client->cgi.output);

			// Construimos la respuesta HTTP completa.
			if (client->setResponse() < 0)
				return -1;

			// IMPORTANTE: permitimos que epoll envíe la respuesta al cliente.
			if (epoll_addflag_fd(client->fd, EPOLLOUT) < 0)
				return -1;
		}
		else if (result < 0)
		{
			close_pipe(fd);
			client->cgi.setRunning(false);

			if (client->generateErrorResponse(INTERNAL_SERVER_ERROR) < 0 || client->setResponse() < 0 || epoll_addflag_fd(client->fd, EPOLLOUT) < 0)
				return -1;

			// IMPORTANTE: permitimos que epoll envíe la respuesta al cliente.
			if (epoll_addflag_fd(client->fd, EPOLLOUT) < 0)
				return -1;
		}
	}
	return 0;
}
