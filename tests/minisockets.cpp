#include <iostream>
#include <sstream>
#include <cstring>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <poll.h>

#define PORT 9999
#define BACKLOG 5
#define BUFFER_SIZE 1024

/* outputs error message if msg is not NULL (else, picks from strerror) and return -err_code */
int	error_msg(int err_code, const char *msg)
{
	std::cerr << "\033[1;31mError: \033[0;31m";
	if (msg)
		std::cerr << msg << "\n";
	else
		std::cerr << strerror(err_code) << "\n\033[0m";
	return -err_code;
}

/* prepares listening socket, if it fails, return value < 0, else returns socket */
int	listen_socket(struct addrinfo *addr, int listen_num)
{
	int	serversock;
	int	status;
	int	yes = 1;

	if (!addr)
		return error_msg(-1, "not a valid struct");

	for (struct addrinfo *p = addr; p != NULL; p = p->ai_next)
	{
		if ((serversock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
		{
			status = errno;
			continue;
		}
		setsockopt(serversock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
		if (bind(serversock, p->ai_addr, p->ai_addrlen) < 0)
		{
			status = errno;
			close(serversock);
			continue;
		}
		if (listen(serversock, listen_num) < 0)
		{
			close(serversock);
			return error_msg(errno, NULL);
		}
		return serversock;
	}
	return error_msg(status, NULL);
}

int	main(int ac, char **av)
{
	struct addrinfo	hints;
	struct addrinfo	*server_info;
	int				status;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	std::stringstream	port;
	port << PORT;
	if ((status = getaddrinfo(NULL, port.str().c_str(), &hints, &server_info)) != 0)
		return (errno, NULL);

	int	sockfd;
	if ((sockfd = listen_socket(server_info, BACKLOG)) < 0)
	{
		freeaddrinfo(server_info);
		return error_msg(sockfd, NULL);
	}
	std::cout << "Listening for connections on port " << PORT << "\n";

	struct sockaddr_storage	clientaddr;
	socklen_t				clientlen = sizeof(clientaddr);
	int						*clientsocket = (int *)malloc(sizeof(int));

	if ((*clientsocket = accept(sockfd, (struct sockaddr *)&clientaddr, (socklen_t *)&clientlen)) < 0)
		return error_msg(errno, NULL);
	if (fcntl(*clientsocket, F_SETFL, O_NONBLOCK) < 0)
		return error_msg(errno, NULL);;
	
	std::cout << "Connection accepted\n";
	std::cout << "Here is to do whatever a webserver does\n";
	std::cout << "The client is saying: \n";

	char	buff[BUFFER_SIZE];
	int		read_size;
	const char	*msg = "Here is where the header is supposed to go.\n READING FOR SOMETHING\n";
	int	len = std::strlen(msg);
	
	struct pollfd pfd[1];
	pfd[0].fd = *clientsocket;
	pfd[0].events = POLLIN | POLLOUT;

	if (send(*clientsocket, msg, len, 0) < 0)
		return error_msg(errno, NULL);
	pfd[0].events &= ~POLLOUT;

	while (1)
	{
		int num_events = poll(pfd, 1, 5000);
		if (num_events < 0)
		{
			if (errno == EINTR)
				continue;
			return error_msg(errno, NULL);
		}
		if (num_events == 0)
		{
			std::cout << "Poll timed out!\n";
			break;
		}
		if (pfd[0].revents & POLLHUP)
		{
			std::cout << "Client closed connection\n";
			break;
		}

		if (pfd[0].revents & POLLOUT)
		{
			;
		}
		if (pfd[0].revents & POLLIN)
		{
			char buff[BUFFER_SIZE];

			int read_size = recv(*clientsocket, buff, BUFFER_SIZE - 1, 0);
			if (read_size > 0)
			{
				buff[read_size] = '\0';
				std::cout << buff;
			}
			else if (read_size == 0)
			{
				std::cout << "Client closed connection\n";
				break;
			}
			else
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK)
					continue;
				return error_msg(errno, NULL);
			}
		}
	}
	


	close(*clientsocket);
	std::cout << "Client disconnected\n";
	free(clientsocket);

	close(sockfd);
	freeaddrinfo(server_info);
}