#pragma once
#include <sstream>
#include <iostream>
#include <netdb.h>
#include <string>

/* Network byte host to string and viceversa */

int inet_pton(int af, const char *src, void *dst);
const char	*inet_ntop(int af, const void *src, char dst[size], socklen_t size);
