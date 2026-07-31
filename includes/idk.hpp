#pragma once
#include <sstream>
#include <iostream>
#include <netdb.h>
#include <string>

in_addr_t	inet_addr(std::stringstream &addr);
in_addr_t	inet_addr(std::string &addr);
