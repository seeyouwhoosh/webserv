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

LocationConfig::LocationConfig() {}

LocationConfig::~LocationConfig() {}

LocationConfig::LocationConfig(const std::string name): name(name) {}

LocationConfig::LocationConfig(const LocationConfig &other)
{
	*this = other;
}

LocationConfig	&LocationConfig::operator=(const LocationConfig &other)
{
	name = other.name;
	root = other.root;
	indexes = other.indexes;
	cgi_handlers = other.cgi_handlers;
	allowed_methods = other.allowed_methods;
	error_pages = other.error_pages;
	autoindex = other.autoindex;
	str_max_body_size = other.str_max_body_size;
	max_body_size = other.max_body_size;
	allowed_methods = other.allowed_methods;
	upload_store = other.upload_store;
	return_status = other.return_status;
	return *this;
}

ConfigParser::ConfigParser() {}

ConfigParser::~ConfigParser() {}

ConfigParser::ConfigParser(const ConfigParser &other) {}

ConfigParser	&ConfigParser::operator=(const ConfigParser &other) {}

static void	eraseComments(std::string &s)
{
	size_t pos = s.find('#');

	while (pos != std::string::npos)
	{
		size_t end_pos = s.find('\n', pos);

		if (end_pos != std::string::npos)
			s.erase(pos, end_pos - pos);
		else
		{
			s.erase(pos);
			break;
		}

		pos = s.find('#', pos);
	}
}

int	ConfigParser::parseConfig(char *file, std::vector<ServerConfig> *servers)
{
	std::fstream	f(file, std::ios::in | std::ios::binary);
	int				status;

	if (!f.is_open())
		return error_msg(1, "open", "can't open the file");

	raw = dumptoString(f);
	f.close();

	eraseComments(raw);

	if (unmatchedBracket(raw))
		return sendParseError(UNMATCHED_BRACKET, "");

	if (incompleteSemicolons(raw))
		return sendParseError(MISSING_SEMICOLON, "");

	if ((status = getTokens()))
		return sendParseError(status, "");

	if ((status = fillDirectives(servers)))
		return sendParseError(status, error_par);
	return 0;
}
