#include "ConfigParser.hpp"
#include "utils.hpp"

/* ORTHODOX CANONICAL FORM */

HTTPContext::HTTPContext(): flags(NONE), autoindex(AUTOINDEX_UNSET) {}

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
	str_timeout = other.str_timeout;
	timeout = other.timeout;
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

int	HTTPContext::parseAlias(std::string &value)
{
	if (flags & ALIAS)
		return ALIAS_ROOT;
	if (flags & ROOT)
	{
		error_par = "alias";
		return MULTIPLE_VALUES;
	}

	flags = flags | ALIAS;
	alias = value;
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
	toupperString(s);

	size_t	last = s.size() - 1;
	size_t	i = 0;
	for (; i < s.size() - 1; i++)
	{
		if (!std::isdigit(s[i]))
			return 1;
	}

	if  (s[i] != 'K' && s[i] != 'M' && !std::isdigit(s[i]))
		return 1;
	int	offset = s[last] == 'K' ? 1000 : s[last] == 'M' ? 1000000 : 1;

	max_body_size = strtoInt(s) * offset;

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
		cgi_handlers.insert(std::make_pair(cgi_file, value));
		cgi_file.clear();
	}
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
