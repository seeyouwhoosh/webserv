#include "ConfigParser.hpp"
#include "utils.hpp"

int	ConfigParser::parseRoot(std::string &value)
{
	if (!root.empty())
		return MULTIPLE_VALUES;

	flags = flags | ROOT;
	root = value;
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
			return INVALID_VALUE;
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
			return INVALID_VALUE;
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
				return DUPLICATED_VALUE;
			if (it->host == "0.0.0.0" || temp_listen.host == "0.0.0.0" || it->host == "*" || temp_listen.host == "*")
				return DUPLICATED_VALUE;
		}
	}

	listens.push_back(temp_listen);
	return 0;
}

// NO HE DADO SOPORTE PARA WILDCARDS
int	ServerConfig::parseServerName(std::string &value)
{
	for (std::vector<std::string>::iterator it = server_name.begin(); it != server_name.end(); it++)
	{
		if (value == *it)
			return DUPLICATED_VALUE;
	}
	server_name.push_back(value);
	return 0;
}

// mirar si suele pasar con el punto o no, porque yo lo suelo parsear sin punto, creo
int	ServerConfig::parseCGIHandler(std::string &value)
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

int	LocationConfig::parseAllowedMethods(std::string &value)
{
	for (std::vector<std::string>::iterator it = allowed_methods.begin(); it != allowed_methods.end(); it++)
	{
		if (*it == value)
			return DUPLICATED_VALUE;
	}
	if (value != "GET" || value != "POST" || value != "UPLOAD")
		return INVALID_VALUE;
	allowed_methods.push_back(value);
	return 0;
}

int	LocationConfig::parseUpload(std::string &value)
{
	if (!upload_store.empty())
		return MULTIPLE_VALUES;

	flags = flags | ROOT;
	upload_store = value;
	return 0;
}

int	LocationConfig::parseReturnstatus(std::string &value)
{
	if (page_return.empty())
	{
		if (!all_digits(value) || value.length() > 3)
			return INVALID_VALUE;
		page_return = value;
	}
	else
	{
		return_status.first = strtoInt(page_return);
		return_status.second = value;
		flags = flags | RETURN;
	}
	return 0;
}
