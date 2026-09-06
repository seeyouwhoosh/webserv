#pragma once
#include <netdb.h> /* getaddrinfo, freeaddrinfo */
#include <sys/epoll.h> /* epoll_create, epoll_ctl, epoll_wait */
#include <arpa/inet.h> /* htons, ntohs */
#include <fcntl.h> /* fcntl */

#include <cstring>
#include <cerrno>
#include <ctime>

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

#include "Config.hpp"
#include "HTTPmessage.hpp"
#include "Parser.hpp"

#define MAX_CONNECTIONS	512

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 1024
#endif

typedef	struct t_client
{
	int			fd;
	time_t		last_activity;
	int			keep_alive;
	int			max_requests;
	int			n_requests;
}	t_client;

class HTTPmessage;
class Parser;

class WebServer
{
private:
	
public:
	/* Orthodox Canonical Form */
	WebServer();
	WebServer(const WebServer &other);
	WebServer &operator=(const WebServer &other);
	~WebServer();

	/* Configuration classes */
	GlobalConfig				global;
	std::vector<ServerConfig>	servers;

	/* Connection variables */
	int							epoll_instance;
	std::vector<int>			listeners;
	std::map<int, t_client>		clients;
	std::map<int, HTTPmessage>	clientRequests;
	std::map<int, Parser>		clientParsers;
	size_t						num_connections;
	struct epoll_event			*epoll_events;

	/* Sockets and clients handlers */
	int			prepare_listeners();
	int			handle_new_connection(int listener);
	bool		is_listener(int fd);
	int			process_connections(int num_events);

	void		create_new_client(int clientsock, int listener, void *clientaddr);
	int			keep_alive_client(int fd);
	int			handle_client_data(int client);
	int			close_connection(int fd);

	void		free_all();
	void		free_all_addrinfos();

	/* Epoll handlers */
	int			epoll_start_instance();
	int			epoll_add_fd(int fd);
	int			epoll_del_fd(int fd);
	int			epoll_addflag_fd(int fd, int flag);
	int			epoll_delflag_fd(int fd, int flag);
	int			epoll_add_listens();
	
	/* Getters and setters*/
	std::string	getPort(int listener);
};

/* Miscelaneous helper functions */

template <typename T>
std::string toString(const T val)
{
	std::stringstream stream;
	stream << val;
	return stream.str();
}

int			listen_socket(struct addrinfo *addr, int listen_num);

int			error_msg(int err_code, const char *func, const char *msg);

bool		file_is_empty(std::fstream &file);

std::string	getDate();

std::string	inet_ntop(int af, const void *src);
