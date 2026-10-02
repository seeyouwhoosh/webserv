#include "WebServer.hpp"

WebServer::WebServer()
{
	epoll_events = new struct epoll_event[MAX_CONNECTIONS];
	num_connections = 0;
}

WebServer::~WebServer() {}

WebServer::WebServer(const WebServer &other)
{
	*this = other;
}

WebServer &WebServer::operator=(const WebServer &other)
{
	(void)other;
	return *this;
}

void	WebServer::free_all()
{
	for (size_t i = 0; i < servers.size() ; i++)
	{
		for (size_t j = 0; j < servers[i].listens.size(); j++)
		{
			if (servers[i].listens[j].serversock_info)
			{
				freeaddrinfo(servers[i].listens[j].serversock_info);
				servers[i].listens[j].serversock_info = NULL;
			}
			close(servers[i].listens[j].sockfd);
		}
	}
	if (epoll_events)
	{
		delete[] epoll_events;
		epoll_events = NULL;
	}
}

int	WebServer::close_connection(int fd)
{
	if (epoll_del_fd(fd) < 0)
	{
		free_all();
		return -1;
	}
	close(fd);
	num_connections--;
	return 0;
}

void	WebServer::free_all_addrinfos()
{
	for (size_t i = 0; i < servers.size() ; i++)
	{
		for (size_t j = 0; j < servers[i].listens.size(); j++)
		{
			if (servers[i].listens[j].serversock_info)
				freeaddrinfo(servers[i].listens[j].serversock_info);
		}
	}
}

std::string	WebServer::getPort(int listener)
{
	for (size_t i = 0; i < servers.size() ; i++)
	{
		for (size_t j = 0; j < servers[i].listens.size(); j++)
		{
			if (listener == servers[i].listens[j].sockfd)
				return servers[i].listens[j].port;
		}
	}
	return "";
}

// HANDLE CLIENT REQUESTS

int	WebServer::process_connections(int num_events)
{
	for (int i = 0; i < num_events ; i++)
	{
		int fd = epoll_events[i].data.fd;
		if (epoll_events[i].events & EPOLLERR)
		{
			if (close_connection(fd) < 0)
				return -1;
			std::cout << "Client" << fd << "error\n";
			continue;
		}
		if (epoll_events[i].events & EPOLLHUP)
		{
			if (close_connection(fd) < 0)
				return -1;
			std::cout << "Client" << fd << "closed connection\n";
			continue;
		}
		if (epoll_events[i].events & EPOLLOUT)
		{
			if (send_data_client(fd) < 0)
				return -1;
		}
		if (epoll_events[i].events & EPOLLIN)
		{
			if (is_listener(epoll_events[i].data.fd))
			{
				if (handle_new_connection(fd) < 0)
					return -1;
			}
			else if (handle_client_data(fd) < 0)
				return -1;
		}
	}
	return 0;
}

int WebServer::parse_config(char *file)
{
	return globalconf.parseConfig(file, &servers);
}

//bonus session
std::string WebServer::createSession()
{
	std::stringstream ss;
	std::time_t now = std::time(NULL);

	ss << now << sessions.size();

	std::string id = ss.str();

	Session session;
	session.id = id;
	session.last_activity = now;
	session.visits = 1;

	sessions[id] = session;

	return id;
}

//bonus session
bool WebServer::hasSession(const std::string &sessionId) const
{
	if (sessions.find(sessionId) == sessions.end())
		return false;

	return true;
}