/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xwu <xwu@student.42urduliz.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/30 13:30:47 by xwu               #+#    #+#             */
/*   Updated: 2026/07/30 13:30:47 by xwu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <vector>
#include <map>
#include <string>

typedef std::map<std::string, std::string> cgi_handler_type;

enum e_autoindex
{
	AUTOINDEX_UNSET = -1,
	AUTOINDEX_OFF,
	AUTOINDEX_ON
};

typedef	struct s_listen
{
	std::string				host;
	std::string				port;
	struct addrinfo			*serversock_info;
	int						sockfd;
}							t_listen;

class GlobalConfig
{
private:

public:
	GlobalConfig();
	~GlobalConfig();
	GlobalConfig(const GlobalConfig &other);
	GlobalConfig	&operator=(const GlobalConfig &other);

	std::string					root;
	std::vector<std::string>	indexes;
	bool						autoindex;
	size_t						client_max_body_size;
	std::map<int, std::string>	error_pages;
};


class LocationConfig
{
private:

public:
	LocationConfig();
	~LocationConfig();
	LocationConfig(const LocationConfig &other);
	LocationConfig	&operator=(const LocationConfig &other);

	std::string					root;
	std::vector<std::string>	indexes;
	bool						autoindex;
	size_t						client_max_body_size;
	cgi_handler_type			cgi_handlers;
	int							limit_except;
	std::string					upload_store;
	int							return_status;
	std::map<int, std::string>	error_pages;
};

class ServerConfig
{
private:

public:
	ServerConfig();
	~ServerConfig();
	ServerConfig(const ServerConfig &other);
	ServerConfig	&operator=(const ServerConfig &other);

	std::vector<LocationConfig> locations;
	std::vector<t_listen>		listens;
	std::string					server_name;
	std::string					root;
	std::vector<std::string>	indexes;
	bool						autoindex;
	size_t						client_max_body_size;
	cgi_handler_type			cgi_handler;
	std::map<int, std::string>	error_pages;
};

