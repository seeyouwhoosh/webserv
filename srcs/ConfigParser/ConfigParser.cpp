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
	HTTPContext		globalconf;

	if (!f.is_open())
		return error_msg(1, "open", "can't open the file");

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
	return 0;
}
