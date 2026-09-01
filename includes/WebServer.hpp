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
	WebServer();
	WebServer(const WebServer &other);
	WebServer &operator=(const WebServer &other);
	~WebServer();

	GlobalConfig				global;
	std::vector<ServerConfig>	servers;

	int							epoll_instance;
	std::vector<int>			listeners;
	std::map<int, t_client>		clients;
	std::map<int, HTTPmessage>	clientRequests;
	std::map<int, Parser>		clientParsers;
	size_t						num_connections;
	struct epoll_event			*epoll_events;

	int			prepare_listeners();
	int			process_connections(int num_events);
	bool		is_listener(int fd);
	int			handle_new_connection(int listener);
	void		create_new_client(int clientsock, int listener);
	int			keep_alive_client(int fd);
	int			handle_client_data(int client);
	void		free_all();
	void		free_all_addrinfos();
	int			close_connection(int fd);

	int			epoll_start_instance();
	int			epoll_add_fd(int fd);
	int			epoll_del_fd(int fd);
	int			epoll_addflag_fd(int fd, int flag);
	int			epoll_delflag_fd(int fd, int flag);
	int			epoll_add_listens();
	
	std::string	getPort(int listener);
};

int			listen_socket(struct addrinfo *addr, int listen_num);
int			error_msg(int err_code, const char *func, const char *msg);
bool		is_empty(std::fstream &file);
std::string	getDate();

