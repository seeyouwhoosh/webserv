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
