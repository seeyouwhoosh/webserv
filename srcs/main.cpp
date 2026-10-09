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

static char	*handle_args(int ac, char **av)
{
	if (ac < 2)
	{
		if (SHOW_LOGS)
		{
			std::cout << "Wrong arguments encountered, using default file"
					<< "\"config_files/default.conf\"\n";
			std::cout << "\033[1mUsage\33[0m: ./webserv [config file]\n";
		}

		return (char *)"config_files/default.conf";
	}
	else
		return av[1];
}

int	main(int ac, char **av)
{
	(void)av;

	char *file = handle_args(ac, av);

	signal(SIGINT, sigint_handler);

	WebServer	webserv;
	int			num_events;

	if (webserv.parse_config(file) < 0 ||
		webserv.prepare_listeners() < 0 ||
		webserv.epoll_start_instance() < 0)
		return 1;

	init_MIMEtypes();
	init_responses();

	while (g_signal)
	{
		num_events = epoll_wait(webserv.epoll_instance,
								webserv.epoll_events,
								MAX_CONNECTIONS, -1);
		if (num_events < 0)
		{
			if (!g_signal)
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
