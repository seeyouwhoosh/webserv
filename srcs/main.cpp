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

//
std::string	arv;
//

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
	location.name = "/";
	location.root = "/sgoinfre/students/xwu/webserv/test";
	location.indexes.push_back("index.html");
	location.autoindex = AUTOINDEX_ON;
	location.error_pages[NOT_FOUND] = "404.html";
	location.upload_store = "/sgoinfre/students/xwu/webserv/test/uploads";
	
	server.autoindex = AUTOINDEX_OFF;
	server.root = "/sgoinfre/students/xwu/webserv/test";
	server.server_name = "prueba.com";
	server.indexes.push_back("picture.jpeg");
	server.locations.push_back(location);
	server.listens.push_back(listen);
	server.cgi_handlers["cgi"] = "/usr/bin/perl";
	webserv.servers.push_back(server);

	arv = av[1];

	/* ************************** ERASE *************************** */

	if (parse_file(av[1]) < 0)
		return 1;
	if (webserv.prepare_listeners() < 0)
	{
		webserv.free_all();
		return 1;
	}
	if (webserv.epoll_start_instance() < 0)
		return 1;

	init_MIMEtypes();
	init_responses();
	init_allowedMethods();
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