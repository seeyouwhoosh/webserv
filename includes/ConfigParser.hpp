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

enum e_parserror
{
	GENERIC_ERROR = 1,
	UNCLOSED_QUOTE,
	UNMATCHED_BRACKET,
	MISSING_SEMICOLON,
	EMPTY_DIRECTIVE,
	UNKNOWN_DIRECTIVE,
	DUPLICATED_DIRECTIVE,
	MULTIPLE_VALUES,
	INVALID_VALUE,
	EMPTY_VALUE
};

enum solo_directives
{
	NONE = 0,
	ROOT = 1 << 0,
	AUTOINDEX = 1 << 1,
	BODY_SIZE = 1 << 2,
	ALLOWED_METHODS = 1 << 3,
	UPLOAD = 1 << 4,
	RETURN = 1 << 5
};

typedef struct s_listen
{
	std::string				host;
	std::string				port;
	struct addrinfo			*serversock_info;
	int						sockfd;
}	t_listen;

class ConfigParser
{
private:
	std::vector<std::string>	tokens;
	std::string					raw;
	std::string					error_par;
	std::vector<ServerConfig>	servers;
	int							flags;

	int		getTokens();
	int		handleQuote(std::stringstream &ss, std::string starter, char quote_char);
	int		sendParseError(int error, const std::string &par);
	void	push_separately(std::string &s, size_t pos);
	int		fillDirectives(std::vector<ServerConfig> *servers);
	int		toMaxBodySize(std::string s);
	int		fillServer(std::vector<std::string>::iterator &start, std::vector<ServerConfig> *servers);
	int		fillLocation(std::vector<std::string>::iterator &start);
	void	fillDefaults(std::vector<ServerConfig> *servers);

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

	int		parseRoot(std::string &value);
	int		parseAutoindex(std::string &value);
	int		parseMaxbody(std::string &value);
	int		parseErrorPage(std::vector<std::string>::iterator &it, std::vector<int> &error_keys);

	int	parseConfig(char *file, std::vector<ServerConfig> *servers);
};

class ServerConfig: public ConfigParser
{
public:
	ServerConfig();
	~ServerConfig();
	ServerConfig(const ServerConfig &other);
	ServerConfig	&operator=(const ServerConfig &other);

	std::vector<LocationConfig>		locations;
	std::vector<t_listen>			listens;
	std::string						server_name;
	cgi_handler_type				cgi_handlers;

	int	parseListens(std::string &value);
	int	parseServerName(std::string &value);
	int	parseCGIHandler(std::string &value);
};

class LocationConfig: public ServerConfig
{
public:
	LocationConfig();
	LocationConfig(const std::string name);
	~LocationConfig();
	LocationConfig(const LocationConfig &other);
	LocationConfig	&operator=(const LocationConfig &other);

	std::string					name;
	std::vector<std::string>	allowed_methods;
	std::string					upload_store;
	std::pair<int, std::string>	return_status;

	int	parseLocationName(std::string &value);
	int	parseAllowedMethods(std::string &value);
	int	parseUpload(std::string &value);
	int	parseReturnstatus(std::string &value);
};

