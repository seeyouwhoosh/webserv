/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xwu <xwu@student.42urduliz.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/29 22:20:28 by xwu               #+#    #+#             */
/*   Updated: 2026/07/30 12:59:35 by xwu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "ConfigCommon.hpp"
#include "LocationConfig.hpp"

class LocationConfig;

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
