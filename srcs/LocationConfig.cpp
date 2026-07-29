#include "LocationConfig.hpp"

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