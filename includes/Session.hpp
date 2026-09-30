#pragma once

#include <string>
#include <ctime>

struct Session
{
	std::string id;
	std::time_t last_activity;
};