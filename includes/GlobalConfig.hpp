/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GlobalConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xwu <xwu@student.42urduliz.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/29 22:20:22 by xwu               #+#    #+#             */
/*   Updated: 2026/07/30 12:48:53 by xwu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "ConfigCommon.hpp"
#include "ServerConfig.hpp"

class ServerConfig;

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

