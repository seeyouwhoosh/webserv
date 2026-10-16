#pragma once
#include <vector>
#include <map>
#include <string>
#include <netdb.h> /* struct addrinfo */

#include "HTTPContext.hpp"
#include "LocationConfig.hpp"

typedef struct s_listen
{
	std::string				host;
	std::string				port;
	struct addrinfo			*serversock_info;
	int						sockfd;
}	t_listen;

class ServerConfig: public HTTPContext
{
public:
	ServerConfig();
	~ServerConfig();
	ServerConfig(const ServerConfig &other);
	ServerConfig	&operator=(const ServerConfig &other);

	std::vector<std::string>		server_name;
	std::vector<t_listen>			listens;
	std::vector<LocationConfig>		locations;

	int	parseListens(std::string &value);
	int	parseServerName(std::string &value);

	int	fillServer(std::vector<std::string>::iterator &start);

	void	clear();
};
