/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xwu <xwu@student.42urduliz.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/29 22:20:15 by xwu               #+#    #+#             */
/*   Updated: 2026/10/16 01:18:03 by xwu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

/* Orthodox Canonical Form */

WebServer::WebServer()
{
	epoll_events = new struct epoll_event[MAX_CONNECTIONS];
	num_connections = 0;
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

/* Closing functions */

void	WebServer::free_all()
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
	if (epoll_events)
	{
		delete[] epoll_events;
		epoll_events = NULL;
	}
}
/**
 * @brief Deletes connection from epoll, closes file descriptor and decrements
 * the number of connections
 * @return 0 when success; -1 otherwise
 */
int	WebServer::close_connection(int fd)
{
	if (epoll_del_fd(fd) < 0)
	{
		free_all();
		return -1;
	}
	close(fd);
	clients.erase(fd);
	if (num_connections > 0)
		num_connections--;
	return 0;
}

/**
 * @brief Looks if fd is a pipe and deletes in from epoll, closes it and deletes it from cgi_pipes
 */
int WebServer::close_pipe(int fd)
{
	if (cgi_pipes.find(fd) != cgi_pipes.end())
	{
		cgi_pipes.erase(fd);
		epoll_del_fd(fd);
		close(fd);
	}
	if (num_connections > 0)
		num_connections--;
	return 0;
}

void	WebServer::free_all_addrinfos()
{
	for (std::map<int, t_listener>::iterator it = listeners.begin(); 
			it != listeners.end(); it++)
	{
		if (it->second.listen->serversock_info)
			freeaddrinfo(it->second.listen->serversock_info);
	}
	listeners.clear();
}

/**
 * @brief Searches in the listens vector one with the corresponding fd
 * @return std::string port if success; "" otherwise
 */
std::string	WebServer::getPort(int listener)
{
	return listeners[listener].listen->port;
}

/* Parser */

/**
 * @brief Calls for the function parseConfig from the class ConfigParser
 * @param file file to parse, doesn't take into account extensions
 * @return 0 when success; -1 otherwise
 */
int WebServer::parse_config(char *file)
{
	return globalconf.parseConfig(file, &servers);
}

/* Main client handler function */

/**
 * @brief Processes active events returned by epoll_wait.
 *
 * Iterates through triggered epoll events to handle socket errors, hangups,
 * incoming read data (new client connections or request payloads), and 
 * outgoing write buffers. Checks for timed-out client sockets before returning.
 * 
 * @return 0 on success, -1 otherwise
 * @note Closes connections automatically.
 */
int	WebServer::process_connections(int num_events) //change
{
	int	send_status;
	for (int i = 0; i < num_events ; i++)
	{
		int fd = epoll_events[i].data.fd;
		if (is_cgi_pipe(fd))
		{
			if (epoll_events[i].events & EPOLLOUT)
			{
				if ((send_status = send_cgi_data(fd, cgi_pipes[fd])) < 0)
					return -1;
			}
			if (epoll_events[i].events & (EPOLLIN | EPOLLHUP))
			{

				if (handle_cgi_data(fd, cgi_pipes[fd]) < 0)
					return -1;
			}
			if (epoll_events[i].events & EPOLLERR)
			{
				cgi_pipes[fd]->cgi.setRunning(false);
				if (close_pipe(fd) < 0)
					return -1;
				continue;
			}
		}
		else
		{
			if (epoll_events[i].events & (EPOLLERR | EPOLLHUP))
			{
				if (close_connection(fd) < 0)
					return -1;
				continue;
			}
			if (epoll_events[i].events & EPOLLOUT)
			{
				if ((send_status = send_data_client(fd)) < 0)
						return -1;
			}
			if (epoll_events[i].events & EPOLLIN)
			{
				if (is_listener(epoll_events[i].data.fd))
				{
					if (handle_new_connection(fd) < 0)
						return -1;
				}
				else if (handle_client_data(fd) < 0)
					return -1;
			}
		}
	}
	check_timeouts();
	return 0;
}

/* Bonus */

std::string WebServer::createSession()
{
	std::stringstream ss;
	std::time_t now = std::time(NULL);

	ss << now << sessions.size();

	std::string id = ss.str();

	Session session;
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