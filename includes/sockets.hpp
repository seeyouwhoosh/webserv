#pragma once
#include <sstream>
#include <iostream>
#include <netdb.h>
#include <string>

/* Network byte host to string and viceversa */

in_addr_t	inet_addr(std::stringstream &addr);
in_addr_t	inet_addr(std::string &addr);
std::string	inet_ntoa(in_addr_t addr);
