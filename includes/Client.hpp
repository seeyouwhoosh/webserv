#pragma once
#include <ctime>
#include <sstream>
#include <queue>

#include "HTTPmessage.hpp"
#include "RequestParser.hpp"

class Client
{
private:
	int						fd;
	ServerConfig			*server;
	RequestParser			parser;
	HTTPmessage				message;
	std::queue<std::string>	http_responses;

	/* Keep Alive */
	time_t					last_activity;
	int						keep_alive;
	int						max_requests;
	int						n_requests;

public:
	Client();
	Client(const Client &other);
	Client(int fd);
	Client	&operator=(const Client &other);
	~Client();

	/* Parser */
	int				parse();
	void			appendData(const std::string &data);
	void			clearParser();

	/* Keep Alive */
	bool			askkeepAlive();
	void			updateActivity();
	time_t			getLastActivity();
	void			setLastActivity(time_t time);

	/* HTTPmessage functions */
	ServerConfig	*getServer();
	void			setSocket(int sock);
	void			initHTTPmessage(std::string &port, std::string address, ServerConfig *server);
	int				setStatusCode(int code);
	int				generateResponse();
	void			setResponse();
	int				responseComplete();
	void			clearResponse();
	int				sendResponse();
	
	/* Bonus */
	std::string		getSessionId() const;
	void			setSessionCookie(const std::string &sessionId);
};
