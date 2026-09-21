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
- [x] Gestionar comillas (Quote opened but never closed)
- [ ] Expansión de comodín
- [x] Integrar comentarios
- [x] Comprobar que el punto y coma está allí
- [ ] Comprobar que la directiva no está sola solo nombre (Block or simple directive with no name)
*/

int	ConfigParser::sendParseError(int error, const std::string &par)
{
	std::string	msg;

	switch (error)
	{
	case GENERIC_ERROR:
		msg = "an error has ocurred while parsing the config file";
		break;
	case UNCLOSED_QUOTE:
		msg = "quote opened but never closed";
		break;
	case UNMATCHED_BRACKET:
		msg = "bracket unmatched";
		break;
	case MISSING_SEMICOLON:
		msg = "missing semicolon";
		break;
	case EMPTY_DIRECTIVE:
		msg = "empty directive";
		break;
	case INVALID_DIRECTIVE:
		msg = "invalid directive";
		break;
	case NOT_IMPLEMENTED:
		msg = "directive not implemented";
		break;
	case MISSING_DIRECTIVE:
		msg = "missing directive";
		break;
	case INVALID_VALUE:
		msg = "invalid value in directive \"" + par + "\"";
		break;
	case EMPTY_VALUE:
		msg = "empty value in directive \"" + par + "\"";
		break;
	default:
		break;
	}
	return error_msg(1, "parse", msg.c_str());
}

int ConfigParser::handleQuote(std::stringstream &ss, std::string starter, char quote_char)
{
	size_t		remainder_index = starter.find(quote_char);
	std::string	remainder = starter.substr(0, remainder_index);
	v.push_back(remainder);

	size_t		end_quote = starter.find(quote_char, remainder_index + 1);
	std::string quote;

	if (end_quote != std::string::npos)
	{
		quote = starter.substr(remainder_index + 1, end_quote - remainder_index - 1);
		tokens.push_back(quote);
		
		remainder = starter.substr(end_quote + 1);
		tokens.push_back(remainder);
		return 0;
	}

	quote = starter.substr(remainder_index + 1);
	std::string	str;
	if (std::getline(ss, str, quote_char))
	{
		quote += str;
		tokens.push_back(quote);
		return 0;
	}
	else
	{
		if (ss.eof())
			return UNCLOSED_QUOTE;
		return GENERIC_ERROR;
	}
}

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

int	ConfigParser::getTokens()
{
	std::stringstream			ss(raw);
	std::string					temp;
	char						c;
	int							status;

	while (ss >> temp)
	{
		if (temp.find('{'))
			;
		if (temp.find('}'))
			;
		size_t	quote_pos = temp.find('\'') == std::string::npos ? temp.find('\'') : temp.find('\"');
		if (quote_pos != std::string::npos && (status = handleQuote(ss, temp, temp[quote_pos])))
			return status;
	}
	return 0;
}

void	eraseComments(std::string &s)
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

bool	incompleteSemicolons(std::string &s)
{
	std::stringstream	ss(s);
	std::string			buff;

	while (std::getline(ss, buff))
	{
		if (buff.find_first_not_of(" \t\r\n") == std::string::npos)
			continue;

		if (buff.find('{') != std::string::npos || buff.find('}') != std::string::npos)
			continue;

		size_t last_char = buff.find_last_not_of(" \t\r\n");
		if (last_char == std::string::npos || buff[last_char] != ';')
			return true;
	}
	return false;
}

tuple	ConfigParser::fillDirectives()
{
	tuple	p(0, "");

	return p;
}

int	ConfigParser::parse_config(char *file, std::vector<ServerConfig> *servers)
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

	if ((status = getTokens(s)))
		return sendParseError(status, "");

	tuple	p;
	if ((p = fillDirectives()))
		return sendParseError(p.first, p.second);
	return 0;
}