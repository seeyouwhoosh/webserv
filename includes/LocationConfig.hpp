/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xwu <xwu@student.42urduliz.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/29 22:20:25 by xwu               #+#    #+#             */
/*   Updated: 2026/07/30 12:54:39 by xwu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "ConfigCommon.hpp"

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

