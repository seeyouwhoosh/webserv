/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xwu <xwu@student.42urduliz.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/29 22:20:15 by xwu               #+#    #+#             */
/*   Updated: 2026/10/17 18:03:31 by xwu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "utils.hpp"

/* ----------------------- Orthodox Canonical Form ----------------------- */

Client::Client()
{
	last_activity = std::time(NULL);
	keep_alive = -1;
	max_requests = -1;
	n_requests = 0;

	message.setCGIHandler(&cgi);
}

Client::Client(const Client &other)
{
	*this = other;
}

Client::Client(int fd): fd(fd)
{
	last_activity = std::time(NULL);
	keep_alive = -1;
	max_requests = -1;
	n_requests = 0;
}

Client	&Client::operator=(const Client &other)
{
	fd = other.fd;
	server = other.server;
	last_activity = other.last_activity;
	keep_alive = other.keep_alive;
	max_requests = other.max_requests;
	n_requests = other.n_requests;

	message = other.message;
	parser = other.parser;
	return *this;
}

Client::~Client() {}

/* ----------------------------- Keep Alive ----------------------------- */

time_t	Client::getLastActivity()
{
	return last_activity;
}

ServerConfig	*Client::getServer()
{
	return server;
}

void	Client::setLastActivity(time_t time)
{
	last_activity = time;
}

/**
 * @brief Checks if the client should close the connection
 * If keep_alive is 0, close; if keep_alive < 0, don't close; 
 */
bool	Client::askkeepAlive()
{
	if (keep_alive < 0)
		return true;
	if (!keep_alive)
		return false;

	time_t		now = time(NULL);
	if (std::difftime(now, last_activity) > keep_alive)
		return true;
	if (max_requests > 0 && n_requests <= max_requests)
		return true;
	return false;
}

/**
 * @brief Gets the Connection and Keep-Alive header, parses it and updates last_activity
 */
void	Client::updateActivity()
{
	stringmap::iterator conn_it = message.rq_headers.find("connection");
	if (conn_it != message.rq_headers.end() && conn_it->second == "close")
		keep_alive = 0;
	else
		keep_alive = -1;

	stringmap::iterator	it = message.rq_headers.find("keep-alive");
	if ((keep_alive < 0) && (it != message.rq_headers.end()))
	{
		std::stringstream	ss(it->second.c_str());
		std::string			s;

		while (std::getline(ss, s, ','))
		{
			if (s.empty())
				continue;

			size_t eq_pos = s.find('=');
			if (eq_pos != std::string::npos)
			{
				std::string val = s.substr(eq_pos + 1);

				if (s.compare(0, eq_pos, "timeout") == 0)
					keep_alive = strtoInt(val);
				else if (s.compare(0, eq_pos, "max") == 0)
					max_requests = strtoInt(val);
			}
		}
	}

	last_activity = std::time(NULL);
	n_requests++;
}

/* -------------------------------- HTTP -------------------------------- */

void Client::setSocket(int sock)
{
	this->fd = sock;
}

/**
 * @brief Prepares the structures to receive messages
 * 
 * Sets server on client, sets sever, port, address and fd in messages
*/
void Client::initHTTPmessage(ServerConfig *server)
{
	this->server = server;
	message.setCGIHandler(&cgi);
	message.setClientfd(fd);
	message.setServerConf(server);
}

int	Client::parse()
{
	return parser.parse(message);
}

int	Client::setStatusCode(int code)
{
	return message.setStatusCode(code);
}

int	Client::generateResponse()
{
	return message.generateResponse();
}

int	Client::generateErrorResponse(int error)
{
	return message.generateErrorResponse(error);
}


/**
 * @brief Adds the response to the response queue and clears message to use
 */
int	Client::setResponse()
{
	std::string	msg = message.setResponse();
	http_responses.push(msg);

	updateActivity();
	parser.clear();
	message.clear();
	cgi.clear();
	return 0;
}

/**
 * @return returns whether the queue is empty, i.e., no messages to send
 */
int	Client::responseComplete()
{
	return http_responses.empty();
}

void	Client::clearParser()
{
	parser.clear();
}

void	Client::clearResponse()
{
	message.clear();
}

void Client::appendData(const std::string &data)
{
	parser.appendData(data);
}

/**
 * @brief Send responses from the queue of messages
 * @return 0 if success; else <0
 */
int	Client::sendResponse()
{
	ssize_t		sent = 0;

	if (http_responses.empty())
		return 0;
	
	std::string &currentResponse = http_responses.front();
	sent = send(fd, currentResponse.data(),
				currentResponse.size(), MSG_NOSIGNAL);
	if (sent < 0)
		return -1;
	if (!sent)
		return 0;

	last_activity = std::time(NULL);
	currentResponse.erase(0, sent);
	if (currentResponse.empty())
		http_responses.pop();

	return sent;
}

/* -------------------------------- Bonus -------------------------------- */

std::string Client::getSessionId() const
{
	stringmap::const_iterator it = message.rq_headers.find("cookie");

	if (it == message.rq_headers.end())
		return "";

	std::string cookie = it->second;

	size_t pos = cookie.find("session_id=");

	if (pos == std::string::npos)
		return "";

	pos += 11;

	size_t end = cookie.find(';', pos);

	if (end == std::string::npos)
		return cookie.substr(pos);

	return cookie.substr(pos, end - pos);
}

void Client::setSessionCookie(const std::string &sessionId)
{
	message.setHeader("Set-Cookie", "session_id=" + sessionId);
}