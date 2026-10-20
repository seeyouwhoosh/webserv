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

#include "ServerConfig.hpp"
#include "utils.hpp"
#include "ConfigParser.hpp"

/* ORTHODOX CANONICAL FORM */

ServerConfig::ServerConfig() {}

ServerConfig::~ServerConfig() {}

ServerConfig::ServerConfig(const ServerConfig &other): HTTPContext(other)
{
	*this = other;
}
ServerConfig	&ServerConfig::operator=(const ServerConfig &other)
{
	HTTPContext::operator=(other);
	locations = other.locations;
	listens = other.listens;
	server_name = other.server_name;

	return *this;
}

int	ServerConfig::parseListens(std::string &value)
{
	t_listen	temp_listen;
	temp_listen.host = "0.0.0.0";
	temp_listen.port = "8000";
	temp_listen.serversock_info = NULL;
	temp_listen.sockfd = -1;

	size_t		ipv6_end = value.find(']');
	size_t		sep;

	if (ipv6_end != std::string::npos)
		sep = value.find(':', ipv6_end);
	else
		sep = value.find(':');

	if (sep == std::string::npos)
	{
		if (allDigits(value))
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
			error_par = "listen";
			if (it->host == temp_listen.host)
				return DUPLICATED_VALUE;
			if (it->host == "0.0.0.0" || temp_listen.host == "0.0.0.0" || it->host == "*" || temp_listen.host == "*")
				return DUPLICATED_VALUE;
		}
	}

	listens.push_back(temp_listen);
	return 0;
}

int	ServerConfig::parseServerName(std::string &value)
{
	if (value.find('*') != std::string::npos)
	{
		error_par = "server_name";
		return INVALID_VALUE;
	}
	if (std::find(server_name.begin(), server_name.end(), value) != server_name.end())
	{
		error_par = "server_name";
		return DUPLICATED_VALUE;
	}

	server_name.push_back(value);

	if (value == "_")
		return MOVE_SERVER_UPFRONT;
	return 0;
}

int	ServerConfig::fillServer(std::vector<std::string>::iterator &start)
{
	LocationConfig		location;
	int					status;
	std::vector<int>	error_keys;
	std::string			directive;
	int					open_bracket = 0;

	location.setTokens(*tokens);
	if (!tokens)
		return GENERIC_ERROR;
	if (start != tokens->end() && *start != "{")
	{
		error_par = "server";
		if (*start == ";")
			return EMPTY_VALUE;
		return INVALID_VALUE;
	}
	start++;

	for (; start != tokens->end(); start++)
	{
		if (*start == "}")
		{
			if (!open_bracket)
				break;
			else
				open_bracket--;
			continue;
		}
		status = UNKNOWN_DIRECTIVE;
		error_par = *start;
		if (*start == ";")
		{
			if (directive.empty())
				return EMPTY_DIRECTIVE;
			directive.clear();
			continue;
		}
		if (directive.empty())
		{
			if (*start == "{")
			{
				open_bracket++;
				continue;
			}
			if ((start + 1) == tokens->end() || *(start + 1) == ";")
				return EMPTY_VALUE;

			if (matchDirective(*start) & flags)
				return DUPLICATED_DIRECTIVE;
			directive = *start;
		}
		else
		{
			if (directive == "location")
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
			else if (directive == "client_max_body_size" && (status = parseMaxbody(*start)))
				return status;
			else if (directive == "timeout" && (status = parseTimeOut(*start)))
				return status;
			else if (directive == "cgi_read_timeout" && (status = parseCGITimeOut(*start)))
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
				if (status != 0)
				{
					error_par = directive;
					return status;
				}
			}
		}
	}
	return 0;
}


void	ServerConfig::clear()
{
	HTTPContext::clear();

	server_name.clear();
	listens.clear();
	locations.clear();
}