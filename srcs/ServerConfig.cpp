#include "ServerConfig.hpp"

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
