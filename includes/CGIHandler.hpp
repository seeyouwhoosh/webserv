#pragma once
#include <vector>
#include <string>

class HTTPmessage;
class LocationConfig;

class CGIHandler
{
private:
	std::vector<std::string>	cgi_env;
	std::string					path_info;
	

	void	fill_env(HTTPmessage &request);
	void	fill_argv(std::string &script, std::string &path);
	void	fill_envp();

public:
	CGIHandler();
	CGIHandler(CGIHandler &other);
	CGIHandler	&operator=(CGIHandler &other);
	~CGIHandler();

	int		execute(const HTTPmessage& request, const LocationConfig& config);
	void	setPathInfo(std::string path);
};

