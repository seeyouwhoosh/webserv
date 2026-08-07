#include <arpa/inet.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <cstring>
#include <sys/socket.h>

#define PORT 9999
#define BUFFER_SIZE 1024

void sendHtml(int *sock, const char *route) {
  FILE *html = fopen(route, "r");
  if (!html) {
    perror("Error opening html file");
    return;
  }

  char buf[BUFFER_SIZE] = {0};
  size_t read = 0;

  char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
  send(*sock, header, strlen(header), 0);

  while ((read = fread(buf, sizeof(buf[0]), BUFFER_SIZE, html)) > 0) {
    send(*sock, buf, read, 0);
  }
  fclose(html);
}

/* outputs error message if msg is not NULL (else, picks from strerror) and return -err_code */
int	error_msg(int err_code, const char *msg)
{
	std::cerr << "Error: "
	if (msg)
		std::cerr << msg << "\n";
	else
		std::cerr << strerror(err_code) << "\n";
	return -err_code;
}

/* prepares listening socket, if it fails, return value < 0, else returns socket */
int	listen_socket(struct addrinfo *addr, int listen_num)
{
	if ((serversock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) < 0)
		return error_msg(errno, NULL);
	if (bind(serversock, addr->ai_addr, res->ai_addrlen) < 0)
		return error_msg(errno, NULL);
	if (listen(serversock, listen_num) < 0)
		return error_msg(errno, NULL);
	return serversock;
}

int	main(int ac, char **av)
{
	struct addrinfo hints;
	struct addrinfo	*servinfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	std::stringstream	port;
	port << PORT;
	if (getaddrinfo(NULL, port.str(), &servinfo) != 0)
	{
		std::cout << "Error: GAI\n";
	}

	int	sockfd = listen_socket((struct sockaddr *)servinfo->ai_addr, 5);
	std::cout << "Listening for connections on port " << PORT << "\n";

	struct sockaddr_in	clientaddr;
	socklen_t	clientlen = sizeof(clientaddr);
	int	*clientsocket = (int *)malloc(sizeof(int));
	while (1)
	{

		if ((*clientsocket = accept(sockfd, (struct sockaddr *)&clientaddr, (socklen_t *)&clientlen)) < 0)
		{
			std::cout << "Error: failed to grab connection " << errno << std::endl;
			continue;
		}
		std::cout << "Connection accepted\n";

		std::cout << "Here is to do whatever a webserver does\n";
		close(*clientsocket);
		std::cout << "Client disconnected\n";
		free(clientsocket);
	}
	close(sockfd);

}