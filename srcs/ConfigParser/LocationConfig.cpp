#include "ConfigParser.hpp"
#include "utils.hpp"

/* ORTHODOX CANONICAL FORM */

LocationConfig::LocationConfig() {}

LocationConfig::~LocationConfig() {}

LocationConfig::LocationConfig(const std::string name): name(name) {}

LocationConfig::LocationConfig(const LocationConfig &other): HTTPContext(other)
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
	str_max_body_size = other.str_max_body_size;
	max_body_size = other.max_body_size;
	allowed_methods = other.allowed_methods;
	upload_store = other.upload_store;
	return_status = other.return_status;
	return *this;
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

void	LocationConfig::clear()
{
	error_par.clear();
	flags = NONE;
	cgi_file.clear();
	root.clear();
	indexes.clear();
	autoindex = AUTOINDEX_UNSET;
	str_max_body_size.clear();
	max_body_size = 0;
	error_pages.clear();
	cgi_handlers.clear();

	name.clear();
	allowed_methods.clear();
	upload_store.clear();
	return_status.first = 0;
	return_status.second = "";
}

int	LocationConfig::fillLocation(std::vector<std::string>::iterator &start)
{
	LocationConfig		temp_location;
	int					status;
	std::vector<int>	error_keys;
	std::string			directive;

	if (start == tokens->end() || *start == "{" || *start == ";")
	{
		error_par = "location";
		return EMPTY_VALUE;
	}

	name = *start;
	start++;

	if (start == tokens->end() || *start != "{")
	{
		error_par = "location";
		return MULTIPLE_VALUES;
	}

	start++;

	for (; start != tokens->end() && *start != "}"; start++)
	{
		error_par = *start;
		if (*start == ";")
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
			if ((start + 1) == tokens->end() || *(start + 1) == ";")
				return EMPTY_VALUE;

			if (matchDirective(*start) & flags)
				return DUPLICATED_DIRECTIVE;
			directive = *start;
		}
		else
		{
			if (directive == "root" && (status = parseRoot(*start)))
				return status;
			else if (directive == "autoindex" && (status = parseAutoindex(*start)))
				return status;
			else if (directive == "client_max_body_size" && (status = parseAutoindex(*start)))
				return status;
			else if (directive == "error_page" && (status = parseErrorPage(start, error_keys)))
				return status;
			else if (directive == "cgi" && (status = parseCGIHandler(*start)))
				return status;
			else if ((directive == "limit_except" || directive == "methods") && (status = parseAllowedMethods(*start)))
				return status;
			else if ((directive == "upload" || directive == "upload_store") && (status = parseUpload(*start)))
				return status;
			else if ((directive == "return" || directive == "upload_store") && (status = parseUpload(*start)))
				return status;
			else if (directive == "index")
				indexes.push_back(*start);
			else
			{
				error_par = directive;
				return UNKNOWN_DIRECTIVE;
			}
		}
	}
	return 0;
}