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

/* ORTHODOX CANONICAL FORM */

ConfigParser::ConfigParser() {}

ConfigParser::~ConfigParser() {}

ConfigParser::ConfigParser(const ConfigParser &other)
{
	*this = other;
}

ConfigParser	&ConfigParser::operator=(const ConfigParser &other)
{
	root = other.root;
	indexes = other.indexes;
	autoindex = other.autoindex;
	client_max_body_size = other.client_max_body_size;
	error_pages = other.error_pages;

	return *this;
}

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
	allowed_methods = other.allowed_methods;
	root = other.root;
	indexes = other.indexes;
	autoindex = other.autoindex;
	client_max_body_size = other.client_max_body_size;
	cgi_handlers = other.cgi_handlers;
	error_pages = other.error_pages;

	return *this;
}

LocationConfig::LocationConfig() {}

LocationConfig::~LocationConfig() {}

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
	client_max_body_size = other.client_max_body_size;
	limit_except = other.limit_except;
	upload_store = other.upload_store;
	return_status = other.return_status;
	return *this;
}

/*
- [ ] Gestionar comillas (Quote opened but never closed)
- [ ] Expansión de comodín
- [ ] Integrar comentarios
- [ ] Comprobar que el punto y coma está allí
- [ ] Comprobar que la directiva no está sola solo nombre (Block or simple directive with no name)

*/

std::string	&dumptoString(std::fstream f)
{
	if (isFileEmpty(f))
		return "";

	std::stringstream ss;
	ss << f.rdbuf();
	
	std::string	s = ss.str();
	
	return s;
}

bool	unmatchedBracket(std::string s)
{
	int		par = 0;
	bool	in_comment = false;

	for (size_t i = 0; i < s.size(); i++)
	{
		if (c == '#')
		{
			in_comment = true;
			continue;
		}
		if (in_comment)
		{
			if (c == '\n')
				in_comment = false;
			continue;
		}
		if (s[i] == '{')
			par++;
		else if (s[i] == '}')
		{
			par--;
			if (par < 0)
				return false;
		}
	}
	if (par)
		return false;
	return true;
}

std::vector<std::string>	&getTokens(std::string s)
{
	std::vector<std::string>	&tokens;
	std::stringstream	ss(s);
	std::string			temp;

	while (ss >> temp)
	{
		if (temp.find('{'))
			;
		if (temp.find('}'))
			;
		if (temp.find('*'))
			;
		if (temp.find('\"'))
			;
	}


}

void eraseComments(std::string &s)
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

int	ConfigParser::parse_config(char *file, std::vector<ServerConfig> *servers);

{
	std::fstream	f(file, std::ios::in | std::ios::binary);

	if (!f.is_open())
		return error_msg(1, "open", "can't open the file");

	std::string	s = dumptoString(f);

	if (unmatchedBracket(s))
		return error_msg(1, "parse", "unmatched bracket ");

	eraseComments(s);

	std::vector<std::string>	tokens = getTokens(s);
	// ir dumpeando linea a linea el string y parsear
	f.close();
	return 0;
}