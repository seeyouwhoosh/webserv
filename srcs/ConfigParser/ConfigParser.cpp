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
	tokens = other.tokens;
	raw = other.raw;
	servers = other.servers;
	error_par = other.error_par;

	return *this;
}

/* MEMBER FUNCTIONS */

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
		return sendError(1, "open", "can't open the file");

	if (isFileEmpty(f))
		return sendError(1, "parseConfig", "file is empty");
	raw = dumptoString(f);
	f.close();

	eraseComments(raw);

	if (unmatchedBracket(raw))
		return sendParseError(UNMATCHED_BRACKET);

	if (incompleteSemicolons(raw))
		return sendParseError(MISSING_SEMICOLON);

	if ((status = getTokens()))
		return sendParseError(status);
	
	globalconf.setTokens(tokens);
	if ((status = globalconf.fillDirectives(servers)))
	{
		error_par = globalconf.error_par;
		return sendParseError(status);
	}
	globalconf.fillDefaults(servers);
	return 0;
}

int	ConfigParser::sendParseError(int error)
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
	case UNKNOWN_DIRECTIVE:
		msg = "unknown directive \"" + error_par + "\"";
		break;
	case DUPLICATED_DIRECTIVE:
		msg = "directive \"" + error_par + "\" doesn't allow duplicates";
		break;
	case DUPLICATED_VALUE:
		msg = "directive \"" + error_par + "\" doesn't allow duplicated values";
		break;
	case INVALID_VALUE:
		msg = "invalid value in directive \"" + error_par + "\"";
		break;
	case MULTIPLE_VALUES:
		msg = "directive \"" + error_par + "\" doesn't allow multiple values";
		break;
	case EMPTY_VALUE:
		msg = "empty value in directive \"" + error_par + "\"";
		break;
	case ALIAS_ROOT:
		msg = "cannot have both directives \"alias\" and \"root\"";
		break;
	default:
		break;
	}
	return sendError(1, "parse", msg.c_str());
}

int ConfigParser::handleQuote(std::stringstream &ss, std::string &starter, char quote_char)
{
	size_t		remainder_index = starter.find(quote_char);
	std::string	remainder = starter.substr(0, remainder_index);
	if (!remainder.empty())
		tokens.push_back(remainder);

	size_t		end_quote = starter.find(quote_char, remainder_index + 1);
	std::string quote;

	/* Quote ends in the same token*/
	if (end_quote != std::string::npos)
	{
		quote = starter.substr(remainder_index + 1, end_quote - remainder_index - 1);
		tokens.push_back(quote);
		
		starter.erase(0, end_quote + 1);
		return 0;
	}

	/* Quote doesn't end in the same token */
	quote = starter.substr(remainder_index + 1);
	std::string	str;
	if (std::getline(ss, str, quote_char))
	{
		quote += str;
		tokens.push_back(quote);
		starter.clear();
		return 0;
	}
	else
	{
		if (ss.eof())
			return UNCLOSED_QUOTE;
		return GENERIC_ERROR;
	}
}

void	ConfigParser::push_separately(std::string &s, size_t index)
{
	std::string	remainder = s.substr(0, index);
	if (!remainder.empty())
		tokens.push_back(remainder);
	s.erase(0, index);
}

int	ConfigParser::getTokens()
{
	std::stringstream	ss(raw);
	std::string			temp;
	int					status;

	while (ss >> temp)
	{
		size_t	index;
		size_t	quote_pos = temp.find('\'') != std::string::npos ? temp.find('\'') : temp.find('\"');

		if (quote_pos != std::string::npos && (status = handleQuote(ss, temp, temp[quote_pos])))
			return status;
		if ((index = temp.find(';')) != std::string::npos)
			push_separately(temp, index);
		if ((index = temp.find('{'))!= std::string::npos)
			push_separately(temp, index);
		if ((index = temp.find('}'))!= std::string::npos)
			push_separately(temp, index);

		if (!temp.empty())
			tokens.push_back(temp);
	}

	return 0;
}
