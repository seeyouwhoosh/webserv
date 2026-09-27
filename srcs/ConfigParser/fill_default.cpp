#include "HTTPContext.hpp"
#include "utils.hpp"
#include "ServerConfig.hpp"

void	HTTPContext::fillDefaults(std::vector<ServerConfig> *servers)
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
			t_listen	default_listen;
			default_listen.host = "0.0.0.0";
			default_listen.port = "8000";
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
