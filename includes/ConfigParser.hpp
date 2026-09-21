#pragma once

#include <netdb.h>
#include <vector>
#include <map>
#include <string>
#include <fstream>

typedef std::map<std::string, std::string> cgi_handler_type;

enum e_autoindex
{
	AUTOINDEX_UNSET = -1,
	AUTOINDEX_OFF,
	AUTOINDEX_ON
};

typedef struct s_listen
{
	std::string				host;
	std::string				port;
	struct addrinfo			*serversock_info;
	int						sockfd;
}	t_listen;

class LocationConfig
{
public:
	LocationConfig();
	~LocationConfig();
	LocationConfig(const LocationConfig &other);
	LocationConfig	&operator=(const LocationConfig &other);

	std::string					name;
	std::string					root;
	std::vector<std::string>	indexes;
	bool						autoindex;
	std::vector<std::string>	allowed_methods;
	size_t						client_max_body_size;
	cgi_handler_type			cgi_handlers;
	int							limit_except;
	std::string					upload_store;
	std::pair<int, std::string>	return_status;
	std::map<int, std::string>	error_pages;
};

class ServerConfig
{
public:
	ServerConfig();
	~ServerConfig();
	ServerConfig(const ServerConfig &other);
	ServerConfig	&operator=(const ServerConfig &other);

	std::vector<LocationConfig>	locations;
	std::vector<t_listen>			listens;
	std::string						server_name;
	std::string						root;
	std::vector<std::string>		indexes;
	bool							autoindex;
	std::vector<std::string>		allowed_methods;
	size_t							client_max_body_size;
	std::string						upload_store;
	cgi_handler_type				cgi_handlers;
	std::map<int, std::string>		error_pages;
};

class ConfigParser
{
public:
	ConfigParser();
	~ConfigParser();
	ConfigParser(const ConfigParser &other);
	ConfigParser	&operator=(const ConfigParser &other);

	std::string					root;
	std::vector<std::string>	indexes;
	bool						autoindex;
	size_t						client_max_body_size;
	std::map<int, std::string>	error_pages;

	int	parse_config(char *file, std::vector<ServerConfig> *servers);
};