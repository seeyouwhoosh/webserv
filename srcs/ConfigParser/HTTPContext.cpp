/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xwu <xwu@student.42urduliz.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/30 13:35:00 by xwu               #+#    #+#             */
/*   Updated: 2026/07/30 13:35:00 by xwu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPContext.hpp"
#include "utils.hpp"
#include "ConfigParser.hpp"

/* ORTHODOX CANONICAL FORM */

HTTPContext::HTTPContext(): flags(NONE), autoindex(AUTOINDEX_UNSET) {}

HTTPContext::~HTTPContext() {}

HTTPContext::HTTPContext(const HTTPContext &other)
{
	*this = other;
}

HTTPContext &HTTPContext::operator=(const HTTPContext &other)
{
	if (this == &other)
		return *this;

	flags = other.flags;
	error_par = other.error_par;
	root = other.root;
	indexes = other.indexes;
	autoindex = other.autoindex;
	str_max_body_size = other.str_max_body_size;
	max_body_size = other.max_body_size;
	str_timeout = other.str_timeout;
	timeout = other.timeout;
	cgi_timeout = other.cgi_timeout;
	error_pages = other.error_pages;
	cgi_file = other.cgi_file;
	cgi_handlers = other.cgi_handlers;
	tokens = other.tokens;

	return *this;
}

/* SETTERS */

void	HTTPContext::setTokens(std::vector<std::string> &tokens)
{
	this->tokens = &tokens;
}

int	HTTPContext::parseRoot(std::string &value)
{
	if (flags & ALIAS)
		return ALIAS_ROOT;
	if (flags & ROOT)
	{
		error_par = "root";
		return MULTIPLE_VALUES;
	}

	flags = flags | ROOT;
	root = value;
	return 0;
}

int	HTTPContext::parseAutoindex(std::string &value)
{
	if (autoindex == AUTOINDEX_UNSET)
	{
		flags = flags | AUTOINDEX;
		if (value == "on")
			autoindex = AUTOINDEX_ON;
		else if (value == "off")
			autoindex = AUTOINDEX_OFF;
		else
		{
			error_par = "autoindex";
			return INVALID_VALUE;
		}
	}
	else
	{
		error_par = "autoindex";
		return MULTIPLE_VALUES;
	}
	return 0;
}

int	HTTPContext::toMaxBodySize(std::string s)
{
	if (s.empty())
		return 1;

	toupperString(s);

	size_t	last = s.size() - 1;
	size_t	multiplier = 1;
	size_t	i = 0;

	for (; i < last; i++)
	{
		if (!std::isdigit(s[i]))
			return 1;
	}
	if (!std::isdigit(s[i]))
	{
		if (s[last] == 'K')
			multiplier = 1024;
		else if (s[last] == 'M')
			multiplier = 1024 * 1024;
		else if (s[last] == 'G') 
			multiplier = 1024 * 1024 * 1024;
		else
			return 1;
		s.erase(last);
	}
	
	if (s.empty())
		return 1;

	long			number = strtoInt(s);
	unsigned long	total = number * multiplier;

	if (total > SIZE_MAX)
		return 1;
	max_body_size = total;

	return 0;
}

int	HTTPContext::parseMaxbody(std::string &value)
{
	if (str_max_body_size.empty())
	{
		flags = flags | BODY_SIZE;
		str_max_body_size = value;
		if (toMaxBodySize(value))
		{
			error_par = "client_max_body_size";
			return INVALID_VALUE;
		}
	}
	else
	{
		error_par = "max_body_size";
		return MULTIPLE_VALUES;
	}
	return 0;
}

int	HTTPContext::toTimeOut(std::string s)
{
	if (s.empty())
		return 1;
	toupperString(s);

	size_t	last = s.size() - 1;
	size_t	i = 0;
	for (; i < last; i++)
	{
		if (!std::isdigit(s[i]))
			return 1;
	}

	if (!(s[i] == 'S' || s[i] == 'M' || std::isdigit(s[i])))
		return 1;
	int	offset = s[last] == 'M' ? 60 : 1;

	if (s.empty())
		return 1;
	long	val = strtoInt(s);
	if (val <= 0 || val > 86400)
		return 1;
	timeout = strtoInt(s) * offset;
	return 0;
}

int	HTTPContext::parseTimeOut(std::string &value)
{
	if (str_timeout.empty())
	{
		flags = flags | TIMEOUT;
		str_timeout = value;
		if (toTimeOut(value))
		{
			error_par = "timeout";
			return INVALID_VALUE;
		}
	}
	else
	{
		error_par = "timeout";
		return MULTIPLE_VALUES;
	}
	return 0;
}

int	HTTPContext::toCGITimeOut(std::string s)
{
	if (s.empty())
		return 1;
	toupperString(s);

	size_t	last = s.size() - 1;
	size_t	i = 0;
	for (; i < last; i++)
	{
		if (!std::isdigit(s[i]))
			return 1;
	}

	if (!(s[i] == 'S' || s[i] == 'M' || std::isdigit(s[i])))
		return 1;
	int	offset = s[last] == 'M' ? 60 : 1;

	if (s.empty())
		return 1;
	long	val = strtoInt(s);
	if (val <= 0 || val > 86400)
		return 1;
	cgi_timeout = strtoInt(s) * offset;
	return 0;
}

int	HTTPContext::parseCGITimeOut(std::string &value)
{
	if (str_timeout.empty())
	{
		flags = flags | CGI_TIMEOUT;
		str_timeout = value;
		if (toCGITimeOut(value))
		{
			error_par = "cgi_read_timeout";
			return INVALID_VALUE;
		}
	}
	else
	{
		error_par = "cgi_read_timeout";
		return MULTIPLE_VALUES;
	}
	return 0;
}

int	HTTPContext::parseErrorPage(std::vector<std::string>::iterator &it, std::vector<int> &error_keys)
{
	if (!allDigits(*it) && (it + 1) != tokens->end() && *(it + 1) == ";")
	{
		std::string	error_page = *it;
		for (size_t i = 0; i < error_keys.size(); i++)
			error_pages[error_keys[i]] = error_page;
		error_keys.clear();
	}
	else
	{
		if (!allDigits(*it))
		{
			error_par = "error_page";
			return INVALID_VALUE;
		}
		error_keys.push_back(strtoInt(*it));
	}
	return 0;
}

int HTTPContext::parseCGIHandler(std::string &value)
{
	if (cgi_file.empty())
	{
		cgi_file = value;
		return 0;
	}

	if (cgi_handlers.find(cgi_file) != cgi_handlers.end())
	{
		cgi_file.clear();
		error_par = "cgi";
		return DUPLICATED_VALUE;
	}

	cgi_handlers.insert(std::make_pair(cgi_file, value));
	cgi_file.clear();

	return 0;
}

int	HTTPContext::fillDirectives(std::vector<ServerConfig> *servers)
{
	ServerConfig		server;
	int					status;
	std::vector<int>	error_keys;
	std::string			directive;

	if (!servers || !tokens)
		return GENERIC_ERROR;

	server.setTokens(*tokens);
	for (std::vector<std::string>::iterator it = tokens->begin(); it != tokens->end(); it++)
	{
		error_par = *it;
		if (*it == ";")
		{
			if (!directive.empty())
			{
				error_par = directive;
				return EMPTY_DIRECTIVE;
			}
			directive.clear();
			continue;
		}
		if (directive.empty())
		{
			if (*it == "}" || *it == "{")
				continue;

			if ((it + 1) == tokens->end() || *(it + 1) == ";")
				return EMPTY_VALUE;

			if (matchDirective(*it) & flags)
				return DUPLICATED_DIRECTIVE;
			directive = *it;
		}
		else
		{
			if (directive == "server")
			{
				if ((status = server.fillServer(it)))
				{
					if (status != MOVE_SERVER_UPFRONT)
					{
						error_par = server.error_par;
						return status;
					}
				}
				if (status == MOVE_SERVER_UPFRONT)
					servers->insert(servers->begin(), server);
				else
					servers->push_back(server);
				server.clear();
				directive.clear();
			}
			else if (directive == "root" && (status = parseRoot(*it)))
				return status;
			else if (directive == "autoindex" && (status = parseAutoindex(*it)))
				return status;
			else if (directive == "client_max_body_size" && (status = parseMaxbody(*it)))
				return status;
			else if (directive == "timeout" && (status = parseTimeOut(*it)))
				return status;
			else if (directive == "cgi_read_timeout" && (status = parseCGITimeOut(*it)))
				return status;
			else if (directive == "error_page" && (status = parseErrorPage(it, error_keys)))
				return status;
			else if (directive == "index")
				indexes.push_back(*it);
			else
			{
				if (status != 0)
				{
					error_par = directive;
					return UNKNOWN_DIRECTIVE;
				}
			}
			directive.clear();
		}
	}
	return 0;
}

void	HTTPContext::fillDefaults(std::vector<ServerConfig> *servers)
{
	if (root.empty())
		root = "html";
	if (indexes.empty())
		indexes.push_back("index.html");
	if (autoindex == AUTOINDEX_UNSET)
		autoindex = AUTOINDEX_OFF;
	if (str_max_body_size.empty())
	{
		str_max_body_size = "1m";
		max_body_size = 1000000;
	}
	if (str_timeout.empty())
	{
		str_timeout = "15s";
		timeout = 15;
	}
	if (str_cgi_timeout.empty())
	{
		str_cgi_timeout = "5s";
		cgi_timeout = 5;
	}
	for (std::vector<ServerConfig>::iterator it = servers->begin(); it != servers->end(); it++)
	{
		/* Server config */
		if (it->locations.empty())
		{
			LocationConfig	default_location("/");
			it->locations.push_back(default_location);
		}
		if (it->listens.empty())
		{
			t_listen	default_listen;
			default_listen.host = "0.0.0.0";
			default_listen.port = "8000";
			it->listens.push_back(default_listen);
		}
		if (it->server_name.empty())
			it->server_name.push_back("");
		if (it->root.empty())
			it->root = root;
		if (it->indexes.empty())
			it->indexes.push_back("index.html");
		if ((it->autoindex == AUTOINDEX_UNSET))
			it->autoindex = autoindex;
		if (it->str_max_body_size.empty())
		{
			it->str_max_body_size = str_max_body_size;
			it->max_body_size = max_body_size;
		}
		if (it->str_timeout.empty())
		{
			it->str_timeout = str_timeout;
			it->timeout = timeout;
		}
		if (it->str_cgi_timeout.empty())
		{
			it->str_cgi_timeout = str_cgi_timeout;
			it->cgi_timeout = cgi_timeout;
		}
		/* Location config */
		for (std::vector<LocationConfig>::iterator jt = it->locations.begin(); jt != it->locations.end() ; jt++)
		{
			if (jt->root.empty() && !(jt->flags & ALIAS))
				jt->root = it->root;
			if ((jt->autoindex == AUTOINDEX_UNSET))
				jt->autoindex = it->autoindex;
			if (jt->allowed_methods.empty())
			{
				std::string	default_methods[] = {"GET", "POST"};
				jt->allowed_methods.insert(jt->allowed_methods.begin(), default_methods, default_methods + 2);
			}
			if (jt->str_max_body_size.empty())
			{
				jt->str_max_body_size = it->str_max_body_size;
				jt->max_body_size = it->max_body_size;
			}
			if (jt->error_pages.empty() && !it->error_pages.empty())
				jt->error_pages = it->error_pages;
			if (jt->indexes.empty())
				jt->indexes = it->indexes;
		}
	}
}

void	HTTPContext::clear()
{
	flags = NONE;
	cgi_file.clear();
	error_par.clear();
	root.clear();
	indexes.clear();
	autoindex = AUTOINDEX_UNSET;
	str_max_body_size.clear();
	max_body_size = 0;
	str_timeout.clear();
	timeout = 0;
	str_cgi_timeout.clear();
	cgi_timeout = 0;
	error_pages.clear();
	cgi_handlers.clear();
}