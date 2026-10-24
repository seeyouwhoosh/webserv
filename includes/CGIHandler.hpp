#pragma once

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include <vector>
#include <string>

class HTTPmessage;
class LocationConfig;

class CGIHandler
{
private:
	/* Client constants */
	std::string					server_port;
	std::string					client_addr;

	/* Variables */
	std::vector<std::string>	cgi_env;
	std::vector<char *>			argv;
	std::vector<char *>			envp;
	std::string					cgi_path;
	std::string 				script_path;
	std::string					filename;
	std::string					path_info;
	std::string					query;
	pid_t						pid;
	int							pipe_in[2]; // Writing to CGI
	int							pipe_out[2]; // Reading from CGI
	bool						isrunning;
	bool						iscgi;

	/* Fillers */
	void			fill_env(const HTTPmessage &request);
	void			fill_argv();
	void			fill_envp();

	void			parseCGIbinFile(size_t cgibin_pos, std::string &full_path,
									LocationConfig *location);
	std::string		matchExtension(const std::string &ext,
									const LocationConfig *location) const;
	int				gotoDirectory();
	size_t			findHeaderEnd(const std::string &output);

	public:
	CGIHandler();
	CGIHandler(CGIHandler &other);
	CGIHandler	&operator=(CGIHandler &other);
	~CGIHandler();
	
	std::string	input;
	std::string	output;
	time_t		last_activity;

	void				parseFiletoCGI(std::string &full_path, LocationConfig *location);
	void				separateQuery(std::string &full_path);
	int					execute(HTTPmessage& request);
	void				parseOutput(HTTPmessage &request);
	void				killScript();
	void				clear();

	/* Getters, setters and booleans */
	bool				isCGI();
	bool				isRunning();
	void				setRunning(bool b);
	void				setClientAddress(std::string client_address);
	void				setServerPort(std::string ServerPort);
	const std::string	&getQuery() const;
	int					getPipeOut() const;
	void				setPipeIn(int fd);
	int					getPipeIn() const;
	pid_t				getPID() const;
};
