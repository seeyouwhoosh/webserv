#include "ConfigParser.hpp"
#include "utils.hpp"

/* ORTHODOX CANONICAL FORM */

HTTPContext::HTTPContext(): autoindex(AUTOINDEX_UNSET), flags(NONE) {}

HTTPContext::~HTTPContext() {}

HTTPContext::HTTPContext(const HTTPContext &other)
{
	*this = other;
}

HTTPContext	&HTTPContext::operator=(const HTTPContext &other)
{
	root = other.root;
	indexes = other.indexes;
	autoindex = other.autoindex;
	str_max_body_size = other.str_max_body_size;
	max_body_size = other.max_body_size;
	error_pages = other.error_pages;

	return *this;
}

/* SETTERS */

void	HTTPContext::setTokens(std::vector<std::string> &tokens)
{
	this->tokens = &tokens;
}

int	HTTPContext::parseRoot(std::string &value)
{
	if (!root.empty())
		return MULTIPLE_VALUES;

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
			return INVALID_VALUE;
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
	toupperString(s);

	size_t	last = s.size() - 1;
	for (size_t i = 0; i < s.size(); i++)
	{
		if (i == (last) && (s[i] != 'K' || s[i] != 'M' || !std::isdigit(s[i])))
			return 1;
		else if (!std::isdigit(s[i]))
		return 1;
	}
	int	offset = s[last] == 'K' ? 1000 : s[last] == 'M' ? 1000000 : 1;

	max_body_size = strtoInt(s) * offset;
}

int	HTTPContext::parseMaxbody(std::string &value)
{
	if (str_max_body_size.empty())
	{
		flags = flags | BODY_SIZE;
		str_max_body_size = value;
		if (toMaxBodySize(value))
		{
			error_par = value;
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

int	HTTPContext::parseErrorPage(std::vector<std::string>::iterator &it, std::vector<int> &error_keys)
{
	if (!all_digits(*it) && (it + 1) != tokens->end() && *(it + 1) == ";")
	{
		std::string	error_page = *it;
		for (size_t i = 0; i < error_keys.size(); i++)
			error_pages[error_keys[i]] = error_page;
		error_keys.clear();
	}
	else
	{
		if (!all_digits(*it))
			return INVALID_VALUE;
		error_keys.push_back(strtoInt(*it));
	}
	return 0;
}

// mirar si suele pasar con el punto o no, porque yo lo suelo parsear sin punto, creo
int	HTTPContext::parseCGIHandler(std::string &value)
{
	if (cgi_file.empty())
		cgi_file = value;
	else
	{
		if (cgi_handlers.find(value) != cgi_handlers.end())
			return DUPLICATED_VALUE;
		cgi_handlers.insert(cgi_file, value);
		cgi_file.clear();
	}
	return 0;
}

int	HTTPContext::fillDirectives(std::vector<ServerConfig> *servers)
{
	ServerConfig	server;
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
					error_par = server.error_par;
					return status;
				}
				servers->push_back(server);
				server.clear();
				directive.clear();
			}
			else if (directive == "root" && (status = parseRoot(*it)))
				return status;
			else if (directive == "autoindex" && (status = parseAutoindex(*it)))
				return status;
			else if (directive == "client_max_body_size" && (status = parseAutoindex(*it)))
				return status;
			else if (directive == "error_page" && (status = parseErrorPage(it, error_keys)))
				return status;
			else if (directive == "index")
				indexes.push_back(*it);
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