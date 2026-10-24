/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xwu <xwu@student.42urduliz.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/29 22:20:15 by xwu               #+#    #+#             */
/*   Updated: 2026/10/23 23:56:18 by xwu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <netdb.h> /* getaddrinfo, freeaddrinfo */
#include <sys/epoll.h> /* epoll_create, epoll_ctl, epoll_wait */
#include <arpa/inet.h> /* htons, ntohs */
#include <fcntl.h> /* fcntl */

#include <cstring>
#include <cerrno>
#include <ctime>

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <list>
#include <map>

#include "ConfigParser.hpp"
#include "Client.hpp"
#include "utils.hpp"

#ifndef MAX_CONNECTIONS
# define MAX_CONNECTIONS 1024
#endif

#ifndef EPOLL_TIMEOUT
# define EPOLL_TIMEOUT 1024
#endif


#ifndef BUFFER_SIZE
# define BUFFER_SIZE 1024
#endif

typedef struct s_listener
{
	ServerConfig	*server;
	t_listen		*listen;
}					t_listener;

typedef struct s_session
{
	std::string	id;
	std::time_t	last_activity;
	int			visits;
}				t_session;

class WebServer
{
private:
	/* Epoll */
	int									epoll_instance;
	struct epoll_event					*epoll_events;
	std::map<int, uint32_t>				fd_events;

	/* Configuration classes */
	ConfigParser						globalconf;
	std::vector<ServerConfig>			servers;

	/* Connection variables */
	std::list<Client *>				unfinished_cgis;
	std::map<int, Client *>				cgi_pipes;
	std::map<int, Client>				clients;
	std::map<int, t_listener>			listeners;
	std::map<std::string, t_session>	sessions; // Bonus

	/* Epoll handlers */
	int			epoll_add_fd(int fd);
	int			epoll_del_fd(int fd);
	int			epoll_addflag_fd(int fd, int flag);
	int			epoll_delflag_fd(int fd, int flag);

	/* Sockets and clients handlers */
	int			sendCgiData(int fd, Client *client);
	int			handleCgiData(int fd, Client *client);
	void		checkFinishedCgi();
	int			handleNewConnection(int listener);
	int			sendClientData(int client);
	int			handleClientData(int client);

	/* Bonus */
	void		handleSession(Client &client);
	std::string	createSession();
	bool		hasSession(const std::string &sessionId) const;

public:
	/* Orthodox Canonical Form */
	WebServer();
	WebServer(const WebServer &other);
	WebServer &operator=(const WebServer &other);
	~WebServer();

	/* Parser */
	int	parseConfig(char *file);

	/* Sockets and clients handlers */
	int		prepareListeners();
	int		processConnections(int num_events);
	void	checkTimeouts();

	/* Closing functions */
	int		closeConnection(int fd);
	int		closePipe(int fd);
	void	clear();

	/* Epoll handlers */
	int		epoll_wait(int maxevents, int timeout);
	int		epoll_start_instance();
	
	/* Getters and setters and booleans */
	bool	isListener(int fd);
	bool	isCgiPipe(int fd);
};
