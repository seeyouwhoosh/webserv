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

#include "GlobalConfig.hpp"
#include <iostream>
#include <fstream>
#include <signal.h>


// Para que el ctrl C no rompa el programa
void	sigint_handler()
{
	signal(SIGINT, SIG_IGN);
	free_stuff();
	exit(0);
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

	if (parse_file(av[1]) == -1 || prepare_sockets() == -1)
	{
		free_stuff();
		return 1;
	}

	while (true)
	{
		poll_and_answer_requests();
	}

	return 1;
}