#include "socket.hpp"
#include <cctype>
#include <cstring>

static int	valid_ip(int af, const char *src)
{
	size_t	char_count = 0;

	if (af == AF_INET)
	{
		for (size_t i = 0; src[i] != '\0'; i++)
		{
			if (!std::isdigit(src[i] - '0') && src[i] != '.')
				return 0;
			if (src[i] == '.')
				char_count++;
		}
		if (char_count != 4)
			return 0;
		return 1;
	}
	else if (af == AF_INET6)
	{
		for (size_t i = 0; src[i] != '\0'; i++)
		{
			if (!std::isxdigit(src[i] - '0') && src[i] != ':')
				return 0;
			if (src[i] == ':')
				char_count++;
		}
		if (char_count != 6)
			return 0;
		return 1;
	}
	else
		return 0;
}

/* convert IPv4 and IPv6 addresses from text to binary form */
int inet_pton(int af, const char *src, void *dst)
{
	int	ip_type;
	std::string s;
	std::stringstream ss(src);

	if (valid_ip(af, src))
		return -1;
	if (af == AF_INET)
	{
		ip_type = 4;
		int bytes[ip_type];

		for (size_t i = 0; i < ip_type; i++)
		{
			if (!std::getline(ss, s, '.'))
				return -1;
			bytes[i] = std::stoi(s);
			if (bytes[i] < 0 || bytes[i] > 255)
				return 0;
		}
		(struct in_addr *)dst->s_addr = htons((bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3]);
		return 1;
	}
	else if (af == AF_INET6)
	{
		;
	}
	else
		return -1;
}

/* convert IPv4 and IPv6 addresses from binary to text form */
const char	*inet_ntop(int af, const void *src, char dst[size], socklen_t size)
{
	std::stringstream ss;
	std::string s;

	if (af == AF_INET)
	{
		const struct in_addr	*addr4 = static_cast<const struct in_addr *>(src);
		uint32_t	addr = ntohs(addr->s_addr);

		ss << ((addr >> 24) & 255 ) << "." << ((addr >> 16) & 255 ) << "." << ((addr >> 8) & 255 ) << "." << (addr & 255 );
	}
	else if (af == AF_INET6)
	{
		const struct in6_addr *addr6 = static_cast<const struct in6_addr *>(src);
	
		for (size_t i = 0; i < 16; i+=2)
		{
			uint16_t group = (addr6->s6_addr[i] << 8) | addr6->s6_addr[i + 1];
			if (group)
				ss << std::hex << group;
			if (i != 6)
				ss << ":";
		}
		dst = ss.str();
	}
	else
		return NULL;

	s = ss.str();
	if (s.size() + 1 > size)
		return NULL;
	std::strcpy(dst, s);
	return dst;
}

