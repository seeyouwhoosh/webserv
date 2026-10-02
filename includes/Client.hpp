#pragma once
#include <ctime>
#include <sstream>

#include "HTTPmessage.hpp"
#include "RequestParser.hpp"

class Client
{
private:
	int		fd;
	time_t	last_activity;
	int		keep_alive;
	int		max_requests;
	int		n_requests;

	HTTPmessage		message;
	RequestParser	parser;
public:
	Client();
	Client(const Client &other);
	Client(int fd);
	Client	&operator=(const Client &other);
	~Client();

	/* Parser */
	int		parse();
	void	appendData(const std::string &data);

	/* Keep Alive */
	bool	askkeepAlive();
	void	updateActivity();

	/* HTTPmessage functions */
	void	initHTTPmessage(std::string &port, std::string address, ServerConfig &server);
	std::string getSessionId() const;//bonus session
	void	setSessionCookie(const std::string &sessionId);//bonus session
	int		setStatusCode(int code);
	int		generateResponse();
	int		sendResponse();
	int		responseComplete();
	void	clearResponse();
	void	clearParser();
};
