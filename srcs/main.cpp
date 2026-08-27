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
#include <iostream>
#include <fstream>
#include <signal.h>

int	g_signal = 1;


void	sigint_handler(int sig)
{
	(void)sig;
	signal(SIGINT, SIG_IGN);
	g_signal = 0;
}

int	parse_file(char *file)
{
	(void)file;
	return 0;
}

int	main(int ac, char **av)
{
	(void)av;
	if (ac != 2)
	{
		std::cout << "\033[1mUsage\33[0m: ./webserv [config file]\n";
		return 1;
	}

	signal(SIGINT, sigint_handler);

	WebServer	webserv;
	int			num_events;

	/* ************************** ERASE ************************** */
	t_listen	listen;
	listen.host = "0.0.0.0";
	listen.port = "9999";

	ServerConfig server;
	LocationConfig	location;
	
	location.name = "/test";
	location.root = "/test";
	location.autoindex = AUTOINDEX_ON;
	location.error_pages[NOT_FOUND] = "404.html";
	
	server.autoindex = AUTOINDEX_OFF;
	server.root = "/";
	server.server_name = "prueba.com";
	server.locations.push_back(location);
	server.listens.push_back(listen);
	webserv.servers.push_back(server);

	/* ************************** ERASE *************************** */

	if (parse_file(av[1]) < 0)
		return 1;
	if (webserv.prepare_listeners() < 0)
	{
		webserv.free_all();
		return 1;
	}
	if (start_epoll_instance(webserv) < 0)
		return 1;

	init_MIMEtypes();
	init_responses();
	while (g_signal)
	{
		num_events = epoll_wait(webserv.epoll_instance, webserv.epoll_events, MAX_CONNECTIONS, -1);
		if (num_events < 0)
		{
			if (errno == EINTR)
				break;
			webserv.free_all();
			return error_msg(errno, "epoll_wait", NULL);
		}
		if (num_events == 0)
			continue;
		
		if (webserv.process_connections(num_events) < 0)
			break;
	}

	webserv.free_all();
	return 0;
}