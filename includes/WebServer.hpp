#pragma once
#include <vector>
#include <sstream>
#include <iostream>
#include <netdb.h>
#include <string>
#include <cstring>
#include <sys/epoll.h>

#include "Config.hpp"

#define MAX_CONNECTIONS	512

class WebServer
{
private:
	
public:
	WebServer();
	WebServer(const WebServer &other);
	WebServer &operator=(const WebServer &other);
	~WebServer();

	GlobalConfig				global;
	std::vector<ServerConfig>	servers;

	int							epoll_instance;
	std::vector<int>			listeners;
	std::vector<int>			clients;
	size_t						num_connections;
	struct epoll_event			*epoll_events;

	int		prepare_listeners();
	int		epoll_add_listens();
	int		process_connections(int num_events);
	bool	is_listener(int fd);
	int		epoll_add_fd(int fd);
	int		handle_new_connection(int listener);
	int		handle_client_data(int client);
	void	free_all();
	void	free_all_addrinfos();
	int		close_connection(int fd);
};

int	listen_socket(struct addrinfo *addr, int listen_num);


/* GENERAL UTILS */

int	error_msg(int err_code, const char *func, const char *msg);
