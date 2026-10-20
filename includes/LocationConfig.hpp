#pragma once
#include <vector>
#include <map>
#include <string>

#include "HTTPContext.hpp"

class LocationConfig: public HTTPContext
{
private:
	std::string		page_return;

public:
	LocationConfig();
	LocationConfig(const std::string name);
	~LocationConfig();
	LocationConfig(const LocationConfig &other);
	LocationConfig	&operator=(const LocationConfig &other);

	std::string					name;
	std::string					alias;
	std::vector<std::string>	allowed_methods;
	std::string					upload_store;
	std::pair<int, std::string>	return_status;

	int		parseAlias(std::string &value);
	int		parseAllowedMethods(std::string &value);
	int		parseUpload(std::string &value);
	int		parseReturnstatus(std::string &value);

	int		fillLocation(std::vector<std::string>::iterator &start);

	void	clear();
};
