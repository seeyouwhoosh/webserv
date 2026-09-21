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


/* Imaginemos que el string grande está en config parser y se llama strbuff */
void	ConfigParser::handleDoubleQuote(std::vector<std::string> &v, std::stringstream &ss, std::string starter)
{
	size_t	remainder_index = starter.find('\"');
	std::string	remainder = starter.substr(0, remainder_index);

	size_t	end_quote = starter.find('\"')
	std::string	quote = starter.substr(remainder_index + 1, );

	std::string	str;
	while (ss >> str)
	{
		if ()
	}
	
}

void	ConfigParser::handleSingleQuote(std::vector<std::string> &v, std::stringstream &ss, std::string starter)
{
	size_t	remainder_index = starter.find('\'');
	std::string	remainder = starter.substr(0, remainder_index);
	v.push_back(remainder);

	size_t		end_quote = starter.find('\'');
	std::string	quote;
	if (end_quote != std::string::npos)
	{
		quote = starter.substr(remainder_index + 1, end_quote);
		v.push_back(quote);
		remainder = starter.substr(end_quote + 1);
		v.push_back(remainder);
		return;
	}

	std::string	str;
	while (ss >> str)
	std::getline(ss, str, '\'');
	
}

int	ConfigParser::parse_config(char *file, std::vector<ServerConfig> *servers);

{
	std::fstream	f(file, std::ios::in);

	if (!f.is_open())
		return error_msg(1, "open", "can't open the file");

	// dumpear todo en un string o así

	// Mirar que los brackets están emparejados
	if (unmatchedBrackets())
		return error_msg(1, "parse", "unmatched brackets");
	// ir dumpeando linea a linea el string y parsear
	f.close();
	return 0;
}