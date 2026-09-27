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
	if (!all_digits(*it) && (it + 1) != tokens.end() && *(it + 1) == ";")
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