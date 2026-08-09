#include <iostream>
#include <cstring>

/* outputs error message if msg is not NULL (else, picks from strerror) and return -err_code */
int	error_msg(int err_code, const char *msg)
{
	std::cerr << "\033[1;31mError: \033[0;31m";
	if (msg)
		std::cerr << msg << "\n";
	else
		std::cerr << strerror(err_code) << "\n\033[0m";
	return -err_code;
}
