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

#include "ConfigParser.hpp"
#include "Client.hpp"
#include "utils.hpp"
#include "Session.hpp"

#define MAX_CONNECTIONS	1024

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 1024
#endif

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
	ConfigParser				globalconf;
	std::vector<ServerConfig>	servers;

	/* Connection variables */
	int							epoll_instance;
	std::vector<int>			listeners;
	std::map<int, Client>		clients;
	std::map<std::string, Session> sessions; //bonus session
	size_t						num_connections;
	struct epoll_event			*epoll_events;

	/* Parser */
	int			parse_config(char *file);

	/* Sockets and clients handlers */
	int			prepare_listeners();
	int			handle_new_connection(int listener);
	bool		is_listener(int fd);
	int			process_connections(int num_events);

	void		create_new_client(int clientsock, int listener, void *clientaddr);
	int			handle_client_data(int client);
	int			send_data_client(int client);
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
	std::string createSession();//bonus session
	bool hasSession(const std::string &sessionId) const;//bonus session
};
