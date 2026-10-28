/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: uelizegi <uelizegi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/29 22:20:15 by xwu               #+#    #+#             */
/*   Updated: 2026/10/28 13:51:18 by uelizegi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

/* ----------------------- Orthodox Canonical Form ----------------------- */

WebServer::WebServer()
{
	epoll_events = new struct epoll_event[MAX_CONNECTIONS];
}

WebServer::~WebServer()
{
	if (epoll_events)
		delete[] epoll_events;
}

WebServer::WebServer(const WebServer &other)
{
	*this = other;
}

WebServer &WebServer::operator=(const WebServer &other)
{
	(void)other;
	return *this;
}

/* ------------------ Getters and setters and booleans ------------------ */

bool	WebServer::isListener(int fd)
{
	if (listeners.find(fd) != listeners.end())
		return true;
	return false;
}

bool	WebServer::isCgiPipe(int fd)
{
	return cgi_pipes.find(fd) != cgi_pipes.end();
}

/* --------------------------- Epoll handlers --------------------------- */

/**
 * @brief Creates a epoll instance and adds the listens
 * @return 0 if success; <0 otherwise
 * @note Shows log messages
 */
int	WebServer::epoll_start_instance()
{
	if ((epoll_instance = epoll_create(MAX_CONNECTIONS)) < 0)
	{
		clear();
		return sendError(errno, "epoll_create", NULL);
	}
	
	if (SHOW_LOGS)
		std::cout << "\033[1m[webserv]\33[0m " <<
		"Epoll instance created succesfully" << "\n";

	for (std::map<int, t_listener>::iterator it = listeners.begin();
	it != listeners.end(); it++)
	{
		if (epoll_add_fd(it->second.listen->sockfd) < 0)
		{
			clear();
			return -1;
		}
	}

	if (SHOW_LOGS)
		std::cout << "\033[1m[webserv]\33[0m " <<
		"Epoll added listeners to watch. Total connections: " <<
		servers.size() << "\n";
	return 0;
}

int	WebServer::epoll_wait(int maxevents, int timeout)
{
	return ::epoll_wait(epoll_instance, epoll_events, maxevents, timeout);
}

/**
 * @brief Adds fd with EPOLLIN flag as default
 * Adds fd to fd_events
 * @return Returns 0 if no errors where encountered
 * @note Shows log messages
 */
int	WebServer::epoll_add_fd(int fd)
{
	struct epoll_event	event;

	event.events = EPOLLIN;
	event.data.fd = fd;
	if (epoll_ctl(epoll_instance, EPOLL_CTL_ADD, fd, &event) < 0)
		return sendError(errno, "epoll_ctl", NULL);
	fd_events[fd] = event.events;
	return 0;
}

/**
 * @brief Deletes fd
 * Deletes fd to fd_events
 * @return Returns 0 if no errors where encountered
 * @note Shows log messages
 */
int	WebServer::epoll_del_fd(int fd)
{
	if (epoll_ctl(epoll_instance, EPOLL_CTL_DEL, fd, NULL) < 0)
		return sendError(errno, "epoll_ctl", NULL);
	fd_events.erase(fd);
	return 0;
}

/**
 * @brief Adds flag to fd
 * @return Returns 0 if no errors where encountered
 * @note Shows log messages
 */
int	WebServer::epoll_addflag_fd(int fd, int flag)
{
	struct epoll_event	event;
	event.data.fd = fd;
	event.events = fd_events[fd] | flag;

	if (epoll_ctl(epoll_instance, EPOLL_CTL_MOD, fd, &event) < 0)
		return sendError(errno, "epoll_ctl", NULL);
	return 0;
}

/**
 * @brief Deletes flag to fd
 * @return Returns 0 if no errors where encountered
 * @note Shows log messages
 */
int	WebServer::epoll_delflag_fd(int fd, int flag)
{
	struct epoll_event event;

	event.data.fd = fd;
	event.events = fd_events[fd] & ~flag;

	if (epoll_ctl(epoll_instance, EPOLL_CTL_MOD, fd, &event) < 0)
		return sendError(errno, "epoll_ctl", NULL);
		
	return 0;
}

/* -------------------------- Closing functions -------------------------- */

/**
 * @brief Frees every addrinfo, closes every pipe, deletes epoll_events
 */
void	WebServer::clear()
{
	for (std::map<int, t_listener>::iterator it = listeners.begin();
			it != listeners.end(); it++)
	{
		if (it->second.listen->serversock_info)
			freeaddrinfo(it->second.listen->serversock_info);
		if (it->second.listen->sockfd >= 0)
			close(it->second.listen->sockfd);
	}
	listeners.clear();
	for (std::map<int, Client *>::iterator it = cgi_pipes.begin();
			it != cgi_pipes.end(); it++)
	{
		if (it->first >= 0)
			close(it->first);
	}
	cgi_pipes.clear();
	for (std::map<int, Client>::iterator it = clients.begin();
			it != clients.end(); it++)
	{
		if (it->first >= 0)
			close(it->first);
	}
	clients.clear();
	if (epoll_events)
	{
		delete[] epoll_events;
		epoll_events = NULL;
	}
}
/**
 * @brief Deletes connection from epoll, closes file descriptor and
 * decrements the number of connections
 * @return 0 when success; -1 otherwise
 */
int	WebServer::closeConnection(int fd)
{
	if (epoll_del_fd(fd) < 0)
	{
		clear();
		return -1;
	}
	close(fd);
	clients.erase(fd);
	return 0;
}

/**
 * @brief Looks if fd is a pipe and deletes in from epoll, closes it and deletes it from cgi_pipes
 */
int WebServer::closePipe(int fd)
{
	if (cgi_pipes.find(fd) != cgi_pipes.end())
	{
		cgi_pipes.erase(fd);
		epoll_del_fd(fd);
		close(fd);
	}
	return 0;
}

/* ------------------------------- Parser ------------------------------- */

/**
 * @brief Calls for the function parseConfig from the class ConfigParser
 * @param file file to parse, doesn't take into account extensions
 * @return 0 when success; -1 otherwise
 */
int WebServer::parseConfig(char *file)
{
	return globalconf.parseConfig(file, &servers);
}

/* ------------------- Main client handler functions ------------------- */

/**
 * @brief Create and opens sockets for servers
 * 
 * Iterates through the list of server informations and sends to create a
 * socket and open it
 * 
 * @return 0 if success, <0 otherwise
 * @note Toggle to show logs
 */
int	WebServer::prepareListeners()
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
				clear();
				return sendError(errno, "getaddrinfo", NULL);
			}
			if ((servers[i].listens[j].sockfd = listenSocket(
				servers[i].listens[j].serversock_info, MAX_CONNECTIONS)) < 0)
			{
				clear();
				freeaddrinfo(servers[i].listens[j].serversock_info);
				return -1;
			}
			if (SHOW_LOGS)
				std::cout << "\033[1m[webserv]\33[0m " << 
				servers[i].server_name[0] << " listening on " << 
				servers[i].listens[j].host << ":" <<
				servers[i].listens[j].port << "\n";
			listeners[servers[i].listens[j].sockfd].server = &servers[i];
			listeners[servers[i].listens[j].sockfd].listen = &servers[i].listens[j];
		}
	}
	return 0;
}

void	WebServer::checkFinishedCgi()
{
	std::list<Client *>::iterator it = unfinished_cgis.begin();

	while (it != unfinished_cgis.end())
	{
		Client	*client = *it;
		int status = 0;
		pid_t result = waitpid(client->cgi.getPID(), &status, WNOHANG);

		if (result == 0)
		{
			++it;
			continue;
		}

		std::list<Client *>::iterator current = it;
		++it;
		unfinished_cgis.erase(current);
		client->cgi.setRunning(false);

		if (result < 0)
		{
			client->generateErrorResponse(INTERNAL_SERVER_ERROR);
			client->setResponse();
			epoll_addflag_fd(client->fd, EPOLLOUT);
			++it;
			continue;
		}
		if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		{
			client->cgi.parseOutput(client->message);
			client->message.setBody(client->cgi.output);

			if (client->setResponse() < 0)
				continue;

			if (epoll_addflag_fd(client->fd, EPOLLOUT) < 0)
				continue;
		}
		else
		{
			if (client->generateErrorResponse(INTERNAL_SERVER_ERROR) < 0
				|| client->setResponse() < 0
				|| epoll_addflag_fd(client->fd, EPOLLOUT) < 0)
			{
				closeConnection(client->fd);
				continue;
			}
			continue;
		}
	}
}

/**
 * @brief Processes active events returned by epoll_wait.
 *
 * Iterates through triggered epoll events to handle socket errors,
 * hangups, incoming read data (new client connections or request
 * payloads), and outgoing write buffers. Checks for timed-out client 
 * sockets before returning.
 * 
 * @return 0 on success, -1 otherwise
 * @note Closes connections automatically.
 */
int	WebServer::processConnections(int num_events)
{
	int	i = 0;

	while (i < num_events)
	{
		int fd = epoll_events[i].data.fd;
		if (isCgiPipe(fd))
		{
			if (epoll_events[i].events & EPOLLOUT)
			{
				if (sendCgiData(fd, cgi_pipes[fd]) < 0)
					return -1;
			}
			if (epoll_events[i].events & (EPOLLIN | EPOLLHUP))
			{

				if (handleCgiData(fd, cgi_pipes[fd]) < 0)
					return -1;
			}
			if (epoll_events[i].events & EPOLLERR)
			{
				cgi_pipes[fd]->cgi.setRunning(false);
				if (closePipe(fd) < 0)
					return -1;
				continue;
			}
		}
		else
		{
			if (epoll_events[i].events & (EPOLLERR | EPOLLHUP))
			{
				if (closeConnection(fd) < 0)
					return -1;
				continue;
			}
			if (epoll_events[i].events & EPOLLOUT)
			{
				if (sendClientData(fd) < 0)
					return -1;
			}
			if (epoll_events[i].events & EPOLLIN)
			{
				if (isListener(epoll_events[i].data.fd))
				{
					while (i < num_events && (epoll_events[i].events & EPOLLIN) && isListener(epoll_events[i].data.fd))
					{
						if (handleNewConnection(fd) < 0)
						{
							i++;
							continue;
						}
						i++;
					}
					continue;
				}
				else if (handleClientData(fd) < 0)
					return -1;
			}
		}
		i++;
	}
	checkFinishedCgi();
	return 0;
}

int	WebServer::sendCgiData(int fd, Client *client)
{
	CGIHandler	*cgi = &client->cgi;

	ssize_t		sent = write(fd, cgi->input.data(), cgi->input.size());

	if (sent < 0)
	{
		closePipe(fd);
		closePipe(cgi->getPipeOut());

		if (client->generateErrorResponse(INTERNAL_SERVER_ERROR) < 0 || client->setResponse() < 0 || epoll_addflag_fd(client->fd, EPOLLOUT) < 0)
			return -1;
		return 0;
	}

	if (sent > 0)
	{
		cgi->last_activity = std::time(NULL);
		cgi->input.erase(0, sent);
	}
	if (sent == 0 || cgi->input.empty())
		closePipe(fd);

	return 0;
}

/**
 * @brief Handles the response generated by the cgi script
 * 
 * Uses the event EPOLLIN to read the response generated by the script
 */
int WebServer::handleCgiData(int fd, Client *client)
{
	char buffer[BUFFER_SIZE];
	ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE);

	if (bytes_read > 0)
		client->cgi.output.append(buffer, bytes_read);
	else if (bytes_read < 0)
	{
		closePipe(fd);
		client->cgi.setRunning(false);

		if (client->generateErrorResponse(INTERNAL_SERVER_ERROR) < 0 || client->setResponse() < 0 || epoll_addflag_fd(client->fd, EPOLLOUT) < 0)
			return -1;
	}
	else
	{
		closePipe(fd);
	
		int status;
		pid_t result = waitpid(client->cgi.getPID(), &status, WNOHANG);

		if (result == 0)
		{
			unfinished_cgis.push_back(client);
			return 0;
		}

		client->cgi.setRunning(false);
		if (result > 0 && WIFEXITED(status) && WEXITSTATUS(status) == 0)
		{
			client->cgi.parseOutput(client->message);
			client->message.setBody(client->cgi.output);

			if (client->setResponse() < 0)
				return -1;

			if (epoll_addflag_fd(client->fd, EPOLLOUT) < 0)
				return -1;
		}
		else if (result < 0 || WEXITSTATUS(status) != 0)
		{
			if (client->generateErrorResponse(INTERNAL_SERVER_ERROR) < 0 || client->setResponse() < 0 || epoll_addflag_fd(client->fd, EPOLLOUT) < 0)
				return -1;
		}
	}
	return 0;
}

/**
 * @brief Creates a socket for the new client, and sets it non
 * blocking and adds it to epoll
 * @note There are logs messages
 */
int	WebServer::handleNewConnection(int listener)
{
	/* Create fd for client */
	struct sockaddr_storage	clientaddr;
	struct sockaddr			*addr = reinterpret_cast<struct sockaddr *>(&clientaddr);

	socklen_t				clientlen = sizeof(clientaddr);
	int						clientsock;

	if ((clientsock = accept(listener, addr, &clientlen)) < 0)
		return sendError(errno, "accept", NULL);

	if (setNonBlocking(clientsock) < 0)
		return sendError(errno, "fcntl", NULL);

	if (epoll_add_fd(clientsock) < 0)
	{
		close(clientsock);
		return -1;
	}

	/* Init of Client */
	t_listener	&srv_listen = listeners[listener];
	Client		&client = clients[clientsock];

	client.setSocket(clientsock);
	client.cgi.setClientAddress(inet_ntop(addr->sa_family, addr));
	client.cgi.setServerPort(srv_listen.listen->port);
	client.initHTTPmessage(srv_listen.server);

	if (SHOW_LOGS)
		std::cout << "Opened connection for client " << clientsock << std::endl;

	return 0;
}

/**
 * @brief Sends messages from queue
 * 
 * Sends message from the response queue, is the sent_size < 0, closes
 * connection; else see if the queue is done, if it is, deletes the
 * EPOLLOUT and see if CLient should stay alive.
 * 
 * @return 0 no error; <0 error
 * @note There are logs messages
 */
int	WebServer::sendClientData(int clientsock)
{
	Client	&client = clients[clientsock];
	int		sent_size = client.sendResponse();
	if (sent_size < 0)
	{
		if (closeConnection(clientsock) < 0)
			return sendError(errno, "send", NULL);
		return -1;
	}
	client.setLastActivity(std::time(NULL));

	if (client.responseComplete())
	{
		epoll_delflag_fd(clientsock, EPOLLOUT);
		if (!client.askkeepAlive())
		{
			if (closeConnection(clientsock) < 0)
				return -1;
			if (SHOW_LOGS)
				std::cout << "Client " << clientsock << " closed connection\n";
		}
	}
	return 0;
}

/**
 * @brief Receives data from client and generates response
 * 
 * Checks first if client should have closed. Then, receives data and parse
 * it, if not complete, continues open to reveive.
 * If complete, generates a response according to the statusCode. And cleans
 * up after
 * 
 * @note Manages the cookies too
 * @note There are logs messages
 */
int	WebServer::handleClientData(int client)
{
	Client	&clientUnit = clients[client];

	/* Should the client be alive? */
	if (!clientUnit.askkeepAlive())
	{
		if (closeConnection(client) < 0)
			return -1;
		if (SHOW_LOGS)
			std::cout << "Client " << client << " closed connection due to timeout\n";
		return 0;
	}

	/* Read input */
	char				buff[BUFFER_SIZE];
	std::stringstream	ss;

	int read_size = recv(client, buff, BUFFER_SIZE - 1, 0);
	if (!read_size)
	{
		if (closeConnection(client) < 0)
			return -1;
		if (SHOW_LOGS)
			std::cout << "Client " << client << " closed connection\n";
		return 0;
	}
	if (read_size > 0)
	{
		clientUnit.appendData(std::string(buff, read_size));
		clientUnit.setLastActivity(std::time(NULL));

		/* Parse */
		int	parse_status = clientUnit.parse();

		switch (parse_status)
		{
		case PARSE_ERROR:
			clientUnit.setStatusCode(BAD_REQUEST);
			break;
		case PARSE_INCOMPLETE:
			clientUnit.setStatusCode(BAD_REQUEST);
			return 0;
		case PARSE_COMPLETE:
			clientUnit.setStatusCode(OK);
			handleSession(clientUnit);
			break;
		default:
			break;
		}

		/* Generate response */
		int		response_status;
		if ((response_status = clientUnit.generateResponse()) < 0)
		{
			clear();
			return sendError(1, "generateResponse", "couldn't generate response");
		}
		else if (response_status == IS_CGI)
		{
			/* Handle cgi */
			int pipe_in = clientUnit.cgi.getPipeIn();
			int pipe_out = clientUnit.cgi.getPipeOut();

			epoll_add_fd(pipe_out);
			cgi_pipes[pipe_out] = &clientUnit;

			if (!clientUnit.cgi.input.empty())
			{
				epoll_add_fd(pipe_in);
				epoll_delflag_fd(pipe_in, EPOLLIN);
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
		/* Adds it to the response queue */
		if (clientUnit.setResponse() < 0)
		{
			if (closeConnection(client) < 0)
				return -1;
		}
		if (epoll_addflag_fd(client, EPOLLOUT) < 0)
		{
			if (closeConnection(client) < 0)
				return -1;
		}
	}
	return 0;
}

/**
 * @brief Checks all active client connections for inactivity timeouts.
 *
 * Compares the current timestamp against each client's last recorded activity.
 * 
 * If a cgi script is still running, kills the script and sends a 504 message.
 * 
 * If a client has been idle longer than the configured global timeout, its
 * connection is closed and removed from the active clients pool.
 * @note There are logs messages
 */
void WebServer::checkTimeouts()
{
	time_t							now = std::time(NULL);
	std::map<int, Client>::iterator	it = clients.begin();
	
	while (it != clients.end())
	{
		int				client_fd = it->first;
		Client			&client = it->second;
		ServerConfig	*server = client.getServer();
		++it;

		if (client.cgi.isRunning() && std::difftime(now,
			client.cgi.last_activity) > server->cgi_timeout)
		{
			client.cgi.killScript();
			
			if (client.generateErrorResponse(GATEWAY_TIMEOUT) >= 0)
			{
				client.setResponse();
				epoll_addflag_fd(client_fd, EPOLLOUT);
				continue;
			}
		}
		if (std::difftime(now, client.getLastActivity()) > server->timeout)
		{
			if (SHOW_LOGS)
				std::cout << "Client " << client_fd << " timed out\n";
			
			if (client.cgi.isRunning())
				client.cgi.killScript();

			closeConnection(client_fd);
		}
	}
}

/* ------------------------------- Bonus ------------------------------- */

void		WebServer::handleSession(Client &client)
{
	std::string sessionId = client.getSessionId();

	if (sessionId.empty() || !hasSession(sessionId))
	{
		sessionId = createSession();
		client.setSessionCookie(sessionId);
	}
	else
		sessions[sessionId].visits++;
	std::cout << "[SESSION] ID: " << sessionId
			  << " | Visitas: " << sessions[sessionId].visits
			  << std::endl;
}

std::string WebServer::createSession()
{
	std::stringstream ss;
	std::time_t now = std::time(NULL);

	ss << now << sessions.size();

	std::string id = ss.str();

	t_session session;
	session.id = id;
	session.last_activity = now;
	session.visits = 1;

	sessions[id] = session;

	return id;
}

bool WebServer::hasSession(const std::string &sessionId) const
{
	if (sessions.find(sessionId) == sessions.end())
		return false;

	return true;
}