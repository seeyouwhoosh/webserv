#include "ConfigParser.hpp"
#include "utils.hpp"

int	ConfigParser::parseRoot(std::string &value)
{
	if (root.empty())
	{
		flags = flags | ROOT;
		root = value;
	}
	else
	{
		error_par = "root";
		return MULTIPLE_VALUES;
	}
	return 0;
}

int	ConfigParser::parseAutoindex(std::string &value)
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

int	ConfigParser::parseMaxbody(std::string &value)
{
	flags = flags | BODY_SIZE;
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

int	ConfigParser::parseErrorPage(std::vector<std::string>::iterator &it, std::vector<int> &error_keys)
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

int	ServerConfig::parseListens(std::string &value)
{
	t_listen	temp_listen = {
		.host = "0.0.0.0",
		.port = "8000"
	};
	size_t		ipv6_end = value.find(']');
	size_t		sep;

	if (ipv6_end != std::string::npos)
		sep = value.find(':', ipv6_end);
	else
		sep = value.find(':');

	if (sep == std::string::npos)
	{
		if (all_digits(value))
			temp_listen.port = value;
		else
		{
			if (value == "localhost")
				temp_listen.host = "127.0.0.1";
			else if (value == "*")
				temp_listen.host = "0.0.0.0";
			else
				temp_listen.host = value;
		}
	}
	else
	{
		temp_listen.host = value.substr(0, sep);
		temp_listen.port = value.substr(sep + 1);

		if (temp_listen.host == "localhost")
			temp_listen.host = "127.0.0.1";
		else if (temp_listen.host == "*")
			temp_listen.host = "0.0.0.0";
	}

	for (std::vector<t_listen>::iterator it = listens.begin(); it != listens.end(); it++)
	{
		if (it->port == temp_listen.port)
		{
			if (it->host == temp_listen.host)
				return MULTIPLE_VALUES;
			if (it->host == "0.0.0.0" || temp_listen.host == "0.0.0.0" || it->host == "*" || temp_listen.host == "*")
				return MULTIPLE_VALUES;
		}
	}

	listens.push_back(temp_listen);
	return 0;
}

int	ServerConfig::parseServerName(std::string &value)
{

}

int	ServerConfig::parseCGIHandler(std::string &value)
{

}

int	LocationConfig::parseLocationName(std::string &value)
{

}

int	LocationConfig::parseAllowedMethods(std::string &value)
{

}

int	LocationConfig::parseUpload(std::string &value)
{

}

int	LocationConfig::parseReturnstatus(std::string &value)
{

}
