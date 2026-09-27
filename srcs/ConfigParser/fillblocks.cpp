#include "ConfigParser.hpp"
#include "utils.hpp"

// por cambiar
int	ConfigParser::fillLocation(std::vector<std::string>::iterator &start)
{
	LocationConfig		temp_location;
	int					status;
	std::vector<int>	error_keys;
	std::string			directive;

	while (start != tokens.end() && (*start == "{" || *start == ";"))
	{
		error_par = "location";
		if ((status = temp_location.parseLocationName(*start)))
			return status;
	}
	
	if (start != tokens.end() && (*start == "{" || *start == ";"))
	{
		error_par = "location";
		return EMPTY_VALUE;
	}

	temp_location.name = *start;

	if (*(++start) != "{")
		return MULTIPLE_VALUES;

	start++;

	for (; start != tokens.end() && *start != "}"; start++)
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
		}
		else if (directive.empty())
		{
			if ((start + 1) == tokens.end() || *(start + 1) == ";")
				return EMPTY_VALUE;

			if (matchDirective(*start) & flags)
				return DUPLICATED_DIRECTIVE;
			directive = *start;
		}
		else
		{
			if (*start == "location")
			{
				if ((status = fillLocation(start)))
				{
					error_par = (temp_location.locations.end() - 1)->error_par;
					return status;
				}
				directive.clear();
			}
			else if (directive == "root" && (status = temp_location.parseRoot(*start)))
				return status;
			else if (directive == "autoindex" && (status = temp_location.parseAutoindex(*start)))
				return status;
			else if (directive == "client_max_body_size" && (status = temp_location.parseAutoindex(*start)))
				return status;
			else if (directive == "error_page" && (status = temp_location.parseErrorPage(start, error_keys)))
				return status;
			else if (directive == "cgi" && (status = temp_location.parseCGIHandler(*start)))
				return status;
			else if ((directive == "limit_except" || directive == "methods") && (status = temp_location.parseAllowedMethods(*start)))
				return status;
			else if ((directive == "upload" || directive == "upload_store") && (status = temp_location.parseUpload(*start)))
				return status;
			else if ((directive == "return" || directive == "upload_store") && (status = temp_location.parseUpload(*start)))
				return status;
			else if (directive == "index")
				temp_location.indexes.push_back(*start);
			else
			{
				error_par = directive;
				return UNKNOWN_DIRECTIVE;
			}
		}
	}
	(servers.end() - 1)->locations.push_back(temp_location);
	return 0;
}

int	ConfigParser::fillServer(std::vector<std::string>::iterator &start, std::vector<ServerConfig> *servers)
{
	ServerConfig		temp_server;
	int					status;
	std::vector<int>	error_keys;
	std::string			directive;

	if (start != tokens.end() && *start != "{")
	{
		error_par = *start;
		if (*start == ";")
		{
			error_par = "server";
			return EMPTY_VALUE;
		}
		return INVALID_VALUE;
	}
	start++;

	for (; start != tokens.end() && *start != "}"; start++)
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
		}
		else if (directive.empty())
		{
			if ((start + 1) == tokens.end() || *(start + 1) == ";")
				return EMPTY_VALUE;

			if (matchDirective(*start) & flags)
				return DUPLICATED_DIRECTIVE;
			directive = *start;
		}
		else
		{
			if (*start == "location")
			{
				if ((status = fillLocation(start)))
				{
					error_par = (temp_server.locations.end() - 1)->error_par;
					return status;
				}
				directive.clear();
			}
			else if (directive == "root" && (status = temp_server.parseRoot(*start)))
				return status;
			else if (directive == "autoindex" && (status = temp_server.parseAutoindex(*start)))
				return status;
			else if (directive == "client_max_body_size" && (status = temp_server.parseAutoindex(*start)))
				return status;
			else if (directive == "error_page" && (status = temp_server.parseErrorPage(start, error_keys)))
				return status;
			else if (directive == "listen")
			{
				error_par = "listen";
				if (*(start + 1) != ";")
					return MULTIPLE_VALUES;
				if ((status = temp_server.parseListens(*start)))
					return status;
			}
			else if (directive == "server_name" && (status = temp_server.parseServerName(*start)))
				return status;
			else if (directive == "cgi" && (status = temp_server.parseCGIHandler(*start)))
				return status;
			else if (directive == "index")
				temp_server.indexes.push_back(*start);
			else
			{
				error_par = directive;
				return UNKNOWN_DIRECTIVE;
			}
			directive.clear();
		}
	}
	servers->push_back(temp_server);
	return 0;
}

int	ConfigParser::fillDirectives(std::vector<ServerConfig> *servers)
{
	int					status;
	std::vector<int>	error_keys;
	std::string			directive;

	if (!servers)
		return GENERIC_ERROR;

	for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); it++)
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
		}
		else if (directive.empty())
		{
			if (*it == "}" || *it == "{")
				continue;

			if ((it + 1) == tokens.end() || *(it + 1) == ";")
				return EMPTY_VALUE;

			if (matchDirective(*it) & flags)
				return DUPLICATED_DIRECTIVE;
			directive = *it;
		}
		else
		{
			if (directive == "server")
			{
				if ((status = fillServer(it, servers)))
				{
					error_par = (servers->end() - 1)->error_par;
					return status;
				}
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

void	ConfigParser::fillDefaults(std::vector<ServerConfig> *servers)
{
	if (root.empty())
		root = "html";
	if (indexes.empty())
		indexes.push_back("index.html");
	if (autoindex == AUTOINDEX_UNSET)
		autoindex = AUTOINDEX_OFF;
	if (str_max_body_size.empty())
	{
		str_max_body_size = "1m";
		max_body_size = 1000000;
	}

	for (std::vector<ServerConfig>::iterator it = servers->begin(); it != servers->end(); it++)
	{
		/* Server config */
		if (it->locations.empty())
		{
			LocationConfig	default_location("/");
			it->locations.push_back(default_location);
		}
		if (it->listens.empty())
		{
			t_listen	default_listen = {
				.host = "0.0.0.0",
				.port = "8000"
			};
			it->listens.push_back(default_listen);
		}
		if (it->server_name.empty())
			it->server_name.push_back("");
		if (it->root.empty())
			it->root = root;
		if (it->indexes.empty())
			it->indexes.push_back("index.html");
		if ((it->autoindex = AUTOINDEX_UNSET))
			it->autoindex = autoindex;
		if (it->str_max_body_size.empty())
		{
			it->str_max_body_size = str_max_body_size;
			it->max_body_size = max_body_size;
		}

		/* Location config */
		for (std::vector<LocationConfig>::iterator jt = it->locations.begin(); jt != it->locations.end() ; jt++)
		{
			if (jt->root.empty())
				jt->root = it->root;
			if ((jt->autoindex = AUTOINDEX_UNSET))
				jt->autoindex = it->autoindex;
			if (jt->allowed_methods.empty())
			{
				std::string	default_methods[] = {"GET", "POST"};
				jt->allowed_methods.insert(jt->allowed_methods.begin(), default_methods, default_methods + 2);
			}
			if (jt->str_max_body_size.empty())
			{
				jt->str_max_body_size = it->str_max_body_size;
				jt->max_body_size = it->max_body_size;
			}
			if (jt->error_pages.empty() && !it->error_pages.empty())
				jt->error_pages = it->error_pages;
			if (jt->cgi_handlers.empty())
				jt->cgi_handlers = it->cgi_handlers;
		}
	}
}
