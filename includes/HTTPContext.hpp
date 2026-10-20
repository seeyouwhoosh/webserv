#pragma once

#include <vector>
#include <map>
#include <string>
#include <algorithm>

typedef	std::map<std::string, std::string>	cgi_handler_type;

enum e_autoindex
{
	AUTOINDEX_UNSET = -1,
	AUTOINDEX_OFF,
	AUTOINDEX_ON
};

enum solo_directives
{
	NONE = 0,
	ROOT = 1 << 0,
	ALIAS = 1 << 1,
	AUTOINDEX = 1 << 2,
	BODY_SIZE = 1 << 3,
	ALLOWED_METHODS = 1 << 4,
	UPLOAD = 1 << 5,
	RETURN = 1 << 6,
	TIMEOUT = 1 << 7,
	CGI_TIMEOUT = 1 << 8
};

class ServerConfig;

class HTTPContext
{
protected:
	int							flags;
	std::string					cgi_file;
	std::vector<std::string>	*tokens;

public:
	HTTPContext();
	virtual ~HTTPContext();
	HTTPContext(const HTTPContext &other);
	HTTPContext	&operator=(const HTTPContext &other);

	std::string					error_par;

	std::string					root;
	std::vector<std::string>	indexes;
	short						autoindex;
	std::string					str_max_body_size;
	size_t						max_body_size;
	std::string					str_timeout;
	size_t						timeout;
	std::string					str_cgi_timeout;
	size_t						cgi_timeout;
	std::map<int, std::string>	error_pages;
	cgi_handler_type			cgi_handlers;

	int		parseRoot(std::string &value);
	int		parseAutoindex(std::string &value);
	int		toMaxBodySize(std::string s);
	int		parseMaxbody(std::string &value);
	int		parseTimeOut(std::string &value);
	int		toTimeOut(std::string s);
	int		parseCGITimeOut(std::string &value);
	int		toCGITimeOut(std::string s);
	int		parseErrorPage(std::vector<std::string>::iterator &it, std::vector<int> &error_keys);
	int		parseCGIHandler(std::string &value);

	int		fillDirectives(std::vector<ServerConfig> *servers);
	void	fillDefaults(std::vector<ServerConfig> *servers);

	void	setTokens(std::vector<std::string> &tokens);
	void	clear();
};