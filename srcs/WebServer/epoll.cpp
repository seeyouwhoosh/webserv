/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   epoll.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xwu <xwu@student.42urduliz.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/29 22:20:15 by xwu               #+#    #+#             */
/*   Updated: 2026/10/09 18:41:01 by xwu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

/**
 * @brief Creates a epoll instance and adds the listens
 * @return 0 if success; <0 otherwise
 * @note Shows log messages
 */
int	WebServer::epoll_start_instance()
{
	if ((epoll_instance = epoll_create(MAX_CONNECTIONS)) < 0)
	{
		free_all();
		return error_msg(errno, "epoll_create", NULL);
	}
	
	if (SHOW_LOGS)
		std::cout << "\033[1m[webserv]\33[0m " <<
		"Epoll instance created succesfully" << "\n";

	if (epoll_add_listens() < 0)
		return -1;

	if (SHOW_LOGS)
		std::cout << "\033[1m[webserv]\33[0m " <<
		"Epoll added listeners to watch. Total connections: " <<
		num_connections << "\n";
	return 0;
}

int	WebServer::epoll_add_listens()
{
	for (std::map<int, t_listener>::iterator it = listeners.begin();
	it != listeners.end(); it++)
	{
		if (epoll_add_fd(it->second.listen->sockfd) < 0)
		{
			free_all();
			return -1;
		}
	}
	return 0;
}

int	WebServer::epoll_add_fd(int fd)
{
	struct epoll_event	event;

	event.events = EPOLLIN;
	event.data.fd = fd;
	if (epoll_ctl(epoll_instance, EPOLL_CTL_ADD, fd, &event) < 0)
		return error_msg(errno, "epoll_ctl", NULL);
	num_connections++;
	return 0;
}

int	WebServer::epoll_del_fd(int fd)
{
	if (epoll_ctl(epoll_instance, EPOLL_CTL_DEL, fd, NULL) < 0)
		return error_msg(errno, "epoll_ctl", NULL);
	return 0;
}

int	WebServer::epoll_addflag_fd(int fd, int flag)
{
	struct epoll_event	event;
	event.data.fd = fd;
	event.events = EPOLLIN | flag;

	if (epoll_ctl(epoll_instance, EPOLL_CTL_MOD, fd, &event) < 0)
		return error_msg(errno, "epoll_ctl", NULL);
	return 0;
}

int	WebServer::epoll_delflag_fd(int fd, int flag)
{
	struct epoll_event event;

	event.data.fd = fd;
	event.events = EPOLLIN & ~flag;

	if (epoll_ctl(epoll_instance, EPOLL_CTL_MOD, fd, &event) < 0)
		return error_msg(errno, "epoll_ctl", NULL);
		
	return 0;
}
