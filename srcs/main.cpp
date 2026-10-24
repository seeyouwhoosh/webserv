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

void	sigquit_handler(int sig)
{
	(void)sig;
	signal(SIGQUIT, SIG_IGN);
	g_signal = 0;
}

void	sigtstp_handler(int sig)
{
	(void)sig;
	signal(SIGTSTP, SIG_IGN);
	g_signal = 0;
}

static char	*handle_args(int ac, char **av)
{
	if (ac < 2)
	{
		if (SHOW_LOGS)
		{
			std::cout << "Wrong arguments encountered, using default file "
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
	signal(SIGQUIT, sigquit_handler);
	signal(SIGTSTP, sigtstp_handler);

	WebServer	webserv;
	int			num_events;

	if (webserv.parseConfig(file) < 0 ||
		webserv.prepareListeners() < 0 ||
		webserv.epoll_start_instance() < 0)
		return 1;

	init_MIMEtypes();
	init_responses();

	while (g_signal)
	{
		num_events = webserv.epoll_wait(MAX_CONNECTIONS, EPOLL_TIMEOUT);
		if (num_events < 0)
		{
			int saved_errno = errno;

			if (!g_signal)
				break;
			if (saved_errno == EINTR)
				continue;

			webserv.clear();
			return sendError(errno, "epoll_wait", NULL);
		}
		if (webserv.processConnections(num_events) < 0)
			break;
		webserv.checkTimeouts();
	}

	webserv.clear();
	return 0;
}
