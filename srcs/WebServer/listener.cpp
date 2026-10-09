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
 * @brief Create and opens sockets for servers
 * 
 * Iterates through the list of server informations and sends to create a
 * socket and open it
 * 
 * @return 0 if success, <0 otherwise
 * @note Toggle to show logs
 */
int	WebServer::prepare_listeners()
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
				free_all_addrinfos();
				return error_msg(errno, "getaddrinfo", NULL);
			}
			if ((servers[i].listens[j].sockfd = listen_socket(
				servers[i].listens[j].serversock_info, MAX_CONNECTIONS)) < 0)
			{
				free_all();
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

/**
 * @return If it's an existing fd (listener), returns true, else returns false */
bool	WebServer::is_listener(int fd)
{
	if (listeners.find(fd) != listeners.end())
		return true;
	return false;
}