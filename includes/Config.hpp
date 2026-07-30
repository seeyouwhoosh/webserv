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
	std::string		host;
	unsigned int	port;
}					t_listen;

class GlobalConfig
{
private:
	std::vector<ServerConfig>	servers;
	std::string					root;
	std::vector<std::string>	indexes;
	bool						autoindex;
	std::string					client_max_body_size;
	std::map<int, std::string>	error_pages;
public:
	GlobalConfig();
	~GlobalConfig();
	GlobalConfig(GlobalConfig &other);
	GlobalConfig	&operator=(GlobalConfig &other);
};

class ServerConfig
{
private:
	std::vector<LocationConfig> locations;
	std::vector<t_listen>		listens;
	std::string					server_name;
	std::string					root;
	std::vector<std::string>	indexes;
	bool						autoindex;
	size_t						client_max_body_size;
	cgi_handler_type			cgi_handler;
	std::map<int, std::string>	error_pages;
public:
	ServerConfig();
	~ServerConfig();
	ServerConfig(ServerConfig &other);
	ServerConfig	&operator=(ServerConfig &other);
};

class LocationConfig
{
private:
	std::string					root;
	std::vector<std::string>	indexes;
	bool						autoindex;
	size_t						client_max_body_size;
	cgi_handler_type			cgi_handlers;
	int							limit_except;
	std::string					upload_store;
	int							return_status;
	std::map<int, std::string>	error_pages;

public:
	LocationConfig();
	~LocationConfig();
	LocationConfig(LocationConfig &other);
	LocationConfig	&operator=(LocationConfig &other);
};
