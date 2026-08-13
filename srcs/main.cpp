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
#include <cstdlib> // exit

int	g_signal = 1;


// Para que el ctrl C no rompa el programa
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

	/* ************************************************************* */
	t_listen	listen;
	listen.host = "localhost";
	listen.port = "9999";

	ServerConfig server;

	server.listens.push_back(listen);
	webserv.servers.push_back(server);

	/* ************************************************************* */
	if (parse_file(av[1]) < 0)
		return 1;

	if (webserv.prepare_listeners() < 0)
	{
		webserv.free_all();
		return 1;
	}

	if ((webserv.epoll_instance = epoll_create(MAX_CONNECTIONS)) < 0)
	{
		webserv.free_all();
		return error_msg(errno, "epoll_create", NULL);
	}
	
	std::cout << "\033[1m[webserv]\33[0m Epoll instance created succesfully" << "\n";
	
	if (webserv.epoll_add_listens() < 0)
		return 1;

	std::cout << "\033[1m[webserv]\33[0m Epoll added listeners to watch. Total connections: " << webserv.num_connections << "\n";

	while (g_signal)
	{
		num_events = epoll_wait(webserv.epoll_instance, webserv.epoll_events, MAX_CONNECTIONS, -1);
		if (num_events < 0)
		{
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