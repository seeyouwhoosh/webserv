#pragma once
#include <netdb.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include <cstring>
#include <cerrno>
#include <ctime>

#include <vector>
#include <sstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>

#include "Config.hpp"
#include "HTTPmessage.hpp"
#include "Parser.hpp"

#define MAX_CONNECTIONS	512

typedef	struct t_client
{
	int			fd;
	time_t		last_activity;
	int			keep_alive;
	int			max_requests;
	int			n_requests;
}	t_client;

class HTTPmessage;

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
	std::map<int, t_client>		clients;
	std::map<int, HTTPmessage>	clientBuffers;
	size_t						num_connections;
	struct epoll_event			*epoll_events;

	int			prepare_listeners();
	int			epoll_add_listens();
	int			process_connections(int num_events);
	bool		is_listener(int fd);
	int			epoll_add_fd(int fd);
	int			handle_new_connection(int listener);
	void		create_new_client(int clientsock, int listener);
	int			keep_alive_client(int fd);
	int			handle_client_data(int client);
	void		free_all();
	void		free_all_addrinfos();
	int			close_connection(int fd);
	
	std::string	getPort(int listener);
};

int			listen_socket(struct addrinfo *addr, int listen_num);
int			error_msg(int err_code, const char *func, const char *msg);
int			start_epoll_instance(WebServer &webserv);
bool		is_empty(std::fstream &file);
std::string	getDate();

