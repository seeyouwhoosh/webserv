/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xwu <xwu@student.42urduliz.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/30 13:35:00 by xwu               #+#    #+#             */
/*   Updated: 2026/10/17 01:52:18 by xwu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"
#include "utils.hpp"
#include "ConfigParser.hpp"

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
	HTTPContext::operator=(other);
	name = other.name;
	alias = other.alias;
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
	if (value != "GET" && value != "POST" && value != "DELETE")
	{
		error_par = "limit_except";
		return INVALID_VALUE;
	}
	allowed_methods.push_back(value);
	return 0;
}

int	LocationConfig::parseUpload(std::string &value)
{
	if (!upload_store.empty())
	{
		error_par = "upload_store";
		return MULTIPLE_VALUES;
	}

	flags = flags | ROOT;
	upload_store = value;
	return 0;
}

int	LocationConfig::parseReturnstatus(std::string &value)
{
	if (page_return.empty())
	{
		if (!allDigits(value) || value.length() > 3)
		{
			error_par = "return";
			return INVALID_VALUE;
		}
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

int	LocationConfig::parseAlias(std::string &value)
{
	if (flags & ROOT)
		return ALIAS_ROOT;
	if (flags & ALIAS)
	{
		error_par = "alias";
		return MULTIPLE_VALUES;
	}

	flags = flags | ALIAS;
	alias = value;
	return 0;
}

void	LocationConfig::clear()
{
	HTTPContext::clear();

	name.clear();
	alias.clear();
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
		status = UNKNOWN_DIRECTIVE;
		error_par = *start;
		if (*start == ";")
		{
			if (directive.empty())
				return EMPTY_DIRECTIVE;
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
			else if (directive == "alias" && (status = parseAlias(*start)))
				return status;
			else if (directive == "autoindex" && (status = parseAutoindex(*start)))
				return status;
			else if (directive == "client_max_body_size" && (status = parseMaxbody(*start)))
				return status;
			else if (directive == "error_page" && (status = parseErrorPage(start, error_keys)))
				return status;
			else if (directive == "cgi" && (status = parseCGIHandler(*start)))
				return status;
			else if ((directive == "limit_except" || directive == "methods") && (status = parseAllowedMethods(*start)))
				return status;
			else if ((directive == "upload" || directive == "upload_store") && (status = parseUpload(*start)))
				return status;
			else if ((directive == "return") && (status = parseReturnstatus(*start)))
				return status;
			else if (directive == "index")
				indexes.push_back(*start);
			else
			{
				if (status != 0)
				{
					error_par = directive;
					return status;
				}
			}
		}
	}
	return 0;
}