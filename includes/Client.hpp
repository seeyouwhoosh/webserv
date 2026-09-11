#pragma once
#include <ctime>

#include "HTTPmessage.hpp"
#include "Parser.hpp"

class Client
{
private:
	time_t	last_activity;
	int		keep_alive;
	int		max_requests;
	int		n_requests;

	HTTPmessage	&message;
	Parser		&parser;

public:
	Client();
	Client(Client &other);
	Client	&operator=(Client &other);
	~Client();

	int	fd;
	
	/* Getters and setters */
	time_t		getLastActivity();
	int			getKeepAlive();
	int			getMaxRequest();
	int			getNumberRequests();
	HTTPmessage	&getHTTPmessage();
	Parser		&getParser();
	void		setLastActivity(time_t last);
	void		setKeepAlive(int keepalive);
	void		setMaxRequest(int max_request);
	void		setNumberRequests(int n);
	void		setHTTPmessage(HTTPmessage &message);
	void		setParser(Parser &parser);
};
