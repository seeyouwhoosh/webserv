#include "Client.hpp"
#include "utils.hpp"

Client::Client()
{
	last_activity = std::time(NULL);
	keep_alive = -1;
	max_requests = -1;
	n_requests = 0;
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
	last_activity = other.last_activity;
	keep_alive = other.keep_alive;
	max_requests = other.max_requests;
	n_requests = other.n_requests;

	message = other.message;
	parser = other.parser;
	return *this;
}

Client::~Client() {}

time_t	Client::getLastActivity()
{
	return last_activity;
}

void	Client::setLastActivity(time_t time)
{
	last_activity = time;
}

bool	Client::askkeepAlive(size_t timeout)
{
	if (keep_alive < 0)
		return 1;
	if (!keep_alive)
		return 0;

	time_t		now = time(NULL);
	if ((size_t)(now - last_activity) > timeout)
		return 0;
	if ((now - last_activity < keep_alive) || (max_requests > 0 && n_requests <= max_requests))
		return 1;
	return 0;
}

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
				std::string key = s.substr(0, eq_pos);
				std::string val = s.substr(eq_pos + 1);

			if (s.find("timeout") != s.npos)
				keep_alive = strtoInt(s.substr(s.find("=") + 1));
			if (s.find("max") != s.npos)
				max_requests = strtoInt(s.substr(s.find("=") + 1));
			}
		}
	}

	last_activity = std::time(NULL);
	n_requests++;
}

void Client::initHTTPmessage(std::string &port, std::string address, ServerConfig &server)
{
	message.setServerPort(port);
	message.setClientAddress(address);
	message.setServerConf(server);
}

//bonus session
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

//bonus session
void Client::setSessionCookie(const std::string &sessionId)
{
	message.setHeader("Set-Cookie", "session_id=" + sessionId);
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

void	Client::setResponse()
{
	http_responses.push(message.setResponse());
	message.clear();
}


int	Client::responseComplete()
{
	return !http_responses.empty();
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

int	Client::sendResponse()
{
	ssize_t		sent = 0;

	if (http_responses.empty())
		return 0;
	
	std::string &currentResponse = http_responses.front();
	sent = send(fd, currentResponse.data(), currentResponse.size(), MSG_NOSIGNAL);
	if (sent < 0)
		return error_msg(errno, "send", NULL);
	if (!sent)
		return 0;

	last_activity = std::time(NULL);
	currentResponse.erase(0, sent);
	if (currentResponse.empty())
		http_responses.pop();

	return sent;
}