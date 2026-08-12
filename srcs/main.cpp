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

// Para que el ctrl C no rompa el programa
void	sigint_handler(int sig)
{
	(void)sig;
	signal(SIGINT, SIG_IGN);
	// cosas
	exit(0);
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

	/*																*/
	t_listen	listen;
	listen.host = "localhost";
	listen.port = "9999";

	ServerConfig	coso;
	coso.listens.push_back(listen);
	webserv.servers.push_back(coso);

	/*																*/

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
		return error_msg(errno, NULL);
	}
	
	std::cout << "\033[1m[webserv]\33[0m: epoll instance created succesfully" << "\n";
	
	if (webserv.epoll_add_listens() < 0)
		return 1;

	std::cout << "\033[1m[webserv]\33[0m: epoll added listeners to watch" << "\n";

	while (true)
	{
		if ((num_events = epoll_wait(webserv.epoll_instance, webserv.epoll_events, MAX_CONNECTIONS, -1)) < 0)
		{
			webserv.free_all();
			return error_msg(errno, NULL);
		}
		else if (num_events == 0)
			continue;
		else if (webserv.process_connections(num_events) < 0)
			break;
	}

	webserv.free_all();
	return 0;
}