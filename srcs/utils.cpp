#include "idk.hpp"

in_addr_t	inet_addr(std::stringstream &addr)
{
	int bytes[4];
	std::string s;

	for (size_t i = 0; i < 4; i++)
	{
		if (!std::getline(addr, s, '.'))
			return 0;
		
		bytes[i] = std::stoi(s);
		if (bytes[i] < 0 || bytes[i] > 255)
			return 0;
	}

	return htonl((bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3]);
}

in_addr_t	inet_addr(std::string &addr)
{
	int bytes[4];
	std::string s;
	std::stringstream ss(addr);

	for (size_t i = 0; i < 4; i++)
	{
		if (!std::getline(ss, s, '.'))
			return 0;
		
		bytes[i] = std::stoi(s);
		if (bytes[i] < 0 || bytes[i] > 255)
			return 0;
	}

	return htonl((bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3]);
}
