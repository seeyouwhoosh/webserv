/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xwu <xwu@student.42urduliz.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/30 13:35:00 by xwu               #+#    #+#             */
/*   Updated: 2026/07/30 13:35:00 by xwu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

/* ORTHODOX CANONICAL FORM */

GlobalConfig::GlobalConfig() {}

GlobalConfig::~GlobalConfig() {}

GlobalConfig::GlobalConfig(GlobalConfig &other)
{
	*this = other;
}
GlobalConfig	&GlobalConfig::operator=(GlobalConfig &other)
{
	(void)other;
	
	return *this;
}

ServerConfig::ServerConfig() {}

ServerConfig::~ServerConfig() {}

ServerConfig::ServerConfig(ServerConfig &other)
{
	*this = other;
}
ServerConfig	&ServerConfig::operator=(ServerConfig &other)
{
	(void)other;
	
	return *this;
}

LocationConfig::LocationConfig() {}

LocationConfig::~LocationConfig() {}

LocationConfig::LocationConfig(LocationConfig &other)
{
	*this = other;
}
LocationConfig	&LocationConfig::operator=(LocationConfig &other)
{
	(void)other;
	
	return *this;
}
