#include "utils.hpp"

void	toupperString(std::string &s)
{
	for (size_t i = 0; i < s.length(); i++)
		s[i] = std::toupper(s[i]);
}

int	hexToAscii(std::string s)
{
	int result = 0;
	int base = 1;

	for (std::string::reverse_iterator it = s.rbegin(); it != s.rend(); ++it)
	{
		char c = std::toupper(static_cast<unsigned char>(*it));
		int val = 0;

		if (c >= 'A' && c <= 'F')
			val = c - 'A' + 10;
		else if (c >= '0' && c <= '9')
			val = c - '0';
		else
			return -1;
		result += val * base;
		base *= 16;
	}
	return result;
}

/* convert IPv4 and IPv6 addresses from binary to text form */
std::string	inet_ntop(int af, const void *src)
{
	std::stringstream ss;
	std::string s;

	if (!src)
		return "";

	if (af == AF_INET)
	{
		const struct in_addr	*addr4 = static_cast<const struct in_addr *>(src);
		uint32_t				addr = ntohl(addr4->s_addr);

		ss << ((addr >> 24) & 255 ) << "." << ((addr >> 16) & 255 ) << "." << ((addr >> 8) & 255 ) << "." << (addr & 255 );
	}
	else if (af == AF_INET6)
	{
		const struct in6_addr *addr6 = static_cast<const struct in6_addr *>(src);
	
		for (size_t i = 0; i < 16; i+=2)
		{
			uint16_t group = (addr6->s6_addr[i] << 8) | addr6->s6_addr[i + 1];
			ss << std::hex << group;
			if (i != 14)
				ss << ":";
		}
	}
	else
		return "";
	s = ss.str();

	return s;
}

long	strtoInt(std::string s)
{
	std::stringstream	ss(s.c_str());
	long				n;

	ss >> n;
	return n;
}
