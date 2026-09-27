#include "ConfigParser.hpp"
#include "utils.hpp"

/* ORTHODOX CANONICAL FORM */

ServerConfig::ServerConfig() {}

ServerConfig::~ServerConfig() {}

ServerConfig::ServerConfig(const ServerConfig &other)
{
	*this = other;
}
ServerConfig	&ServerConfig::operator=(const ServerConfig &other)
{
	locations = other.locations;
	listens = other.listens;
	server_name = other.server_name;
	root = other.root;
	indexes = other.indexes;
	autoindex = other.autoindex;
	str_max_body_size = other.str_max_body_size;
	max_body_size = other.max_body_size;
	cgi_handlers = other.cgi_handlers;
	error_pages = other.error_pages;

	return *this;
}

int	ServerConfig::parseListens(std::string &value)
{
	t_listen	temp_listen = {
		.host = "0.0.0.0",
		.port = "8000"
	};
	size_t		ipv6_end = value.find(']');
	size_t		sep;

	if (ipv6_end != std::string::npos)
		sep = value.find(':', ipv6_end);
	else
		sep = value.find(':');

	if (sep == std::string::npos)
	{
		if (all_digits(value))
			temp_listen.port = value;
		else
		{
			if (value == "localhost")
				temp_listen.host = "127.0.0.1";
			else if (value == "*")
				temp_listen.host = "0.0.0.0";
			else
				temp_listen.host = value;
		}
	}
	else
	{
		temp_listen.host = value.substr(0, sep);
		temp_listen.port = value.substr(sep + 1);

		if (temp_listen.host == "localhost")
			temp_listen.host = "127.0.0.1";
		else if (temp_listen.host == "*")
			temp_listen.host = "0.0.0.0";
	}

	for (std::vector<t_listen>::iterator it = listens.begin(); it != listens.end(); it++)
	{
		if (it->port == temp_listen.port)
		{
			
			if (it->host == temp_listen.host)
				return DUPLICATED_VALUE;
			if (it->host == "0.0.0.0" || temp_listen.host == "0.0.0.0" || it->host == "*" || temp_listen.host == "*")
				return DUPLICATED_VALUE;
		}
	}

	listens.push_back(temp_listen);
	return 0;
}

// NO HE DADO SOPORTE PARA WILDCARDS
int	ServerConfig::parseServerName(std::string &value)
{
	for (std::vector<std::string>::iterator it = server_name.begin(); it != server_name.end(); it++)
	{
		if (value == *it)
			return DUPLICATED_VALUE;
	}
	server_name.push_back(value);
	return 0;
}

int	ServerConfig::fillServer(std::vector<std::string>::iterator &start)
{
	LocationConfig		location;
	int					status;
	std::vector<int>	error_keys;
	std::string			directive;

	location.setTokens(*tokens);
	if (!tokens)
		return GENERIC_ERROR;
	if (start != tokens->end() && *start != "{")
	{
		error_par = *start;
		if (*start == ";")
		{
			error_par = "server";
			return EMPTY_VALUE;
		}
		return INVALID_VALUE;
	}
	start++;

	for (; start != tokens->end() && *start != "}"; start++)
	{
		error_par = *start;
		if (*start == ";")
		{
			if (!directive.empty())
			{
				error_par = directive;
				return EMPTY_DIRECTIVE;
			}
			directive.clear();
		}
		else if (directive.empty())
		{
			if ((start + 1) == tokens->end() || *(start + 1) == ";")
				return EMPTY_VALUE;

			if (matchDirective(*start) & flags)
				return DUPLICATED_DIRECTIVE;
			directive = *start;
		}
		else
		{
			if (*start == "location")
			{
				if ((status = location.fillLocation(start)))
				{
					error_par = location.error_par;
					return status;
				}
				locations.push_back(location);
				location.clear();
				directive.clear();
			}
			else if (directive == "root" && (status = parseRoot(*start)))
				return status;
			else if (directive == "autoindex" && (status = parseAutoindex(*start)))
				return status;
			else if (directive == "client_max_body_size" && (status = parseAutoindex(*start)))
				return status;
			else if (directive == "error_page" && (status = parseErrorPage(start, error_keys)))
				return status;
			else if (directive == "listen")
			{
				error_par = "listen";
				if (*(start + 1) != ";")
					return MULTIPLE_VALUES;
				if ((status = parseListens(*start)))
					return status;
			}
			else if (directive == "server_name" && (status = parseServerName(*start)))
				return status;
			else if (directive == "cgi" && (status = parseCGIHandler(*start)))
				return status;
			else if (directive == "index")
				indexes.push_back(*start);
			else
			{
				error_par = directive;
				return UNKNOWN_DIRECTIVE;
			}
			directive.clear();
		}
	}
	return 0;
}


void	ServerConfig::clear()
{
	error_par.clear();
	flags = NONE;
	cgi_file.clear();
	root.clear();
	indexes.clear();
	autoindex = AUTOINDEX_UNSET;
	str_max_body_size.clear();
	max_body_size = 0;
	error_pages.clear();
	cgi_handlers.clear();

	server_name.clear();
	listens.clear();
	locations.clear();
}