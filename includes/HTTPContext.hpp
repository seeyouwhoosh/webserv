#pragma once
#include <vector>
#include <map>
#include <string>

#include "ServerConfig.hpp"

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
	AUTOINDEX = 1 << 1,
	BODY_SIZE = 1 << 2,
	ALLOWED_METHODS = 1 << 3,
	UPLOAD = 1 << 4,
	RETURN = 1 << 5
};

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
	std::map<int, std::string>	error_pages;
	cgi_handler_type			cgi_handlers;

	int		parseRoot(std::string &value);
	int		parseAutoindex(std::string &value);
	int		toMaxBodySize(std::string s);
	int		parseMaxbody(std::string &value);
	int		parseErrorPage(std::vector<std::string>::iterator &it, std::vector<int> &error_keys);
	int		parseCGIHandler(std::string &value);

	int		fillDirectives(std::vector<ServerConfig> *servers);


	void	setTokens(std::vector<std::string> &tokens);
};