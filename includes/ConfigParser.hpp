#pragma once

#include <netdb.h>

#include <cctype>

#include <vector>
#include <map>
#include <string>
#include <fstream>

#include "ServerConfig.hpp"

enum e_parserror
{
	GENERIC_ERROR = 1,
	UNCLOSED_QUOTE,
	UNMATCHED_BRACKET,
	MISSING_SEMICOLON,
	EMPTY_DIRECTIVE,
	UNKNOWN_DIRECTIVE,
	DUPLICATED_DIRECTIVE,
	DUPLICATED_VALUE,
	MULTIPLE_VALUES,
	INVALID_VALUE,
	EMPTY_VALUE,
	ALIAS_ROOT,
	MOVE_SERVER_UPFRONT
};

class ConfigParser
{
private:
	std::vector<std::string>	tokens;
	std::string					raw;
	std::vector<ServerConfig>	servers;

	int		getTokens();
	int		handleQuote(std::stringstream &ss, std::string &starter, char quote_char);
	void	push_separately(std::string &s, size_t pos);

public:
	ConfigParser();
	~ConfigParser();
	ConfigParser(const ConfigParser &other);
	ConfigParser	&operator=(const ConfigParser &other);

	std::string	error_par;
	HTTPContext	globalconf;

	int		sendParseError(int error);
	int		parseConfig(char *file, std::vector<ServerConfig> *servers);
};

