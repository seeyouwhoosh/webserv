#pragma once

#include <netdb.h>

#include <cctype>

#include <vector>
#include <map>
#include <string>
#include <fstream>

typedef	std::map<std::string, std::string>	cgi_handler_type;
typedef	std::pair<int, std::string>			tuple;

enum e_autoindex
{
	AUTOINDEX_UNSET = -1,
	AUTOINDEX_OFF,
	AUTOINDEX_ON
};

enum ParseError
{
	GENERIC_ERROR = 1,
	UNCLOSED_QUOTE,
	UNMATCHED_BRACKET,
	MISSING_SEMICOLON,
	NOT_IMPLEMENTED,
	EMPTY_DIRECTIVE,
	INVALID_DIRECTIVE,
	MISSING_DIRECTIVE,
	DUPLICATED_DIRECTIVE,
	MULTIPLE_VALUES,
	INVALID_VALUE,
	EMPTY_VALUE
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
	LocationConfig(const std::string name);
	~LocationConfig();
	LocationConfig(const LocationConfig &other);
	LocationConfig	&operator=(const LocationConfig &other);

	std::string					name;
	std::string					root;
	std::vector<std::string>	indexes;
	bool						autoindex;
	std::vector<std::string>	allowed_methods;
	std::string					str_max_body_size;
	size_t						max_body_size;
	std::map<int, std::string>	error_pages;
	cgi_handler_type			cgi_handlers;
	std::string					upload_store;
	std::pair<int, std::string>	return_status;
};

class ServerConfig
{
public:
	ServerConfig();
	~ServerConfig();
	ServerConfig(const ServerConfig &other);
	ServerConfig	&operator=(const ServerConfig &other);

	std::vector<LocationConfig>		locations;
	std::vector<t_listen>			listens;
	std::string						server_name; // está permitido varios nombres
	std::string						root;
	std::vector<std::string>		indexes;
	bool							autoindex;
	std::string						str_max_body_size;
	size_t							max_body_size;
	std::map<int, std::string>		error_pages;
	cgi_handler_type				cgi_handlers;
};

class ConfigParser
{
private:
	std::vector<std::string>	tokens;
	std::string					raw;
	std::string					error_par;
	std::vector<ServerConfig>	servers;

	int		getTokens();
	int		handleQuote(std::stringstream &ss, std::string starter, char quote_char);
	int		sendParseError(int error, const std::string &par);
	void	push_separately(std::string &s, size_t pos);
	int		fillDirectives(std::vector<ServerConfig> *servers);
	int		toMaxBodySize(std::string s);
	int		fillServer(std::vector<std::string>::iterator &start, std::vector<ServerConfig> *servers);
	int		fillLocation(std::vector<std::string>::iterator &start);
	void	fillDefaults(std::vector<ServerConfig> *servers);

	int	parseRoot(std::string value);
	int	parseAutoindex(std::string value);
	int	parseMaxbody(std::string value);
	int	parseErrorPage(std::vector<std::string>::iterator it, std::vector<int> &error_keys);
public:
	ConfigParser();
	~ConfigParser();
	ConfigParser(const ConfigParser &other);
	ConfigParser	&operator=(const ConfigParser &other);

	std::string					root;
	std::vector<std::string>	indexes;
	short						autoindex;
	std::string					str_max_body_size;
	size_t						max_body_size;
	std::map<int, std::string>	error_pages;

	int	parseConfig(char *file, std::vector<ServerConfig> *servers);
};