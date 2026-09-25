#include "ConfigParser.hpp"
#include "utils.hpp"

int	ConfigParser::parseRoot(std::string value)
{
	if (root.empty())
		root = value;
	else
	{
		error_par = "root";
		return MULTIPLE_VALUES;
	}
	return 0;
}

int	ConfigParser::parseAutoindex(std::string value)
{
	if (autoindex == AUTOINDEX_UNSET)
	{
		if (value == "on")
			autoindex = AUTOINDEX_ON;
		else if (value == "off")
			autoindex = AUTOINDEX_OFF;
		else
		{
			error_par = value;
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

int	ConfigParser::parseMaxbody(std::string value)
{
	if (str_max_body_size.empty())
	{
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

int	ConfigParser::parseErrorPage(std::vector<std::string>::iterator it, std::vector<int> &error_keys)
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
		{
			error_par = *it;
			return INVALID_VALUE;
		}
		error_keys.push_back(strtoInt(*it));
	}
	return 0;
}
