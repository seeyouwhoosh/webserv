#pragma once
#include <ctime>
#include <sstream>

#include "HTTPmessage.hpp"
#include "Parser.hpp"

class Client
{
private:
	int		fd;
	time_t	last_activity;
	int		keep_alive;
	int		max_requests;
	int		n_requests;

	HTTPmessage	message;
	Parser		parser;
public:
	Client();
	Client(const Client &other);
	Client(int fd);
	Client	&operator=(const Client &other);
	~Client();

	/* Parser */
	int	parse();

	/* Keep Alive */
	bool	askkeepAlive();
	void	updateActivity();

	/* HTTPmessage functions */
	void	initHTTPmessage(std::string &port, std::string address);
	int		setStatusCode(int code);
	int		generateResponse();
	int		sendResponse();
	int		responseComplete();
	void	clear();
};
