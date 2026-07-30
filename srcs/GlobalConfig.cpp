/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GlobalConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xwu <xwu@student.42urduliz.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/29 22:20:31 by xwu               #+#    #+#             */
/*   Updated: 2026/07/29 22:20:32 by xwu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "GlobalConfig.hpp"

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
