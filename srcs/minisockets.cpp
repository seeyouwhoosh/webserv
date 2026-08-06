#include <arpa/inet.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <cstring>

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

int	main(int ac, char **av)
{
	/* Create a socket (IPv4, TCP). El tercer argumento es 0 porque suele haber un solo protocolo para para cada tipo (SOCK_STREAM, por ejemplo) */
	int sockfd;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		std::cout << "Error: socket failed\n";
		return -1;
	}

	/*
	Para el tipo de socket que tenemos, en nuestro caso un AF_INET, pasamos una estructura u otra, en este caso, usamos sockaddr_in 
	
	struct sockaddr_in {
	sa_family_t     sin_family;     // AF_INET. Siempre es este.
	in_port_t       sin_port;       //Port number
	struct in_addr  sin_addr;       // IPv4 address
	};

	struct in_addr {
	in_addr_t	s_addr;
	}:

	Para el puerto y la IP, siempre se guardan en el Network Byte Order, así que hay que usar la función htons() "host to network (short)" para convertir un unsigned short a network

	Special addresses
	INADDR_LOOPBACK (127.0.0.1)
	INADDR_ANY (0.0.0.0)
	INADDR_BROADCAST (255.255.255.255)

	*/

	struct sockaddr_in	sockaddr;
	sockaddr.sin_family = AF_INET; // en IPv4 always
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(PORT); //convert host number to network byte

	if (bind(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0)
	{
		std::cout << "Error: bind to port " << PORT << " failed\n";
		return -1;
	}


	/* Start listening. Hold at most number connections in the queue
	backlog is the maximum number of connections that will be queued before connections start being refused.
	*/

	if (listen(sockfd, 5) < 0)
	{
		std::cout << "Error: listen on socket failed " << errno << "\n";
		return -1;
	}

	std::cout << "Listening for connections on port " << PORT << "\n";

	while (1)
	{
		struct sockaddr_in	clientaddr;
		socklen_t	clientlen = sizeof(clientaddr);
		int	*clientsocket = (int *)malloc(sizeof(int));
	
		/* Grab a connection from the queue
		/* addrlen is now a value-result argument. It expects a pointer to an int that will be the size of addr. After the function is executed, the int refered by addrlen will be set to the size of the peer address.
		*/
		
		if ((*clientsocket = accept(sockfd, (struct sockaddr *)&clientaddr, (socklen_t *)&clientlen)) < 0)
		{
			std::cout << "Error: failed to grab connection " << errno << std::endl;
			continue;
		}
		std::cout << "Connection accepted\n";

		sendHtml(clientsocket, "../tests/404.html");
		close(*clientsocket);
		std::cout << "Client disconnected\n";
		free(clientsocket);

	}
	close(sockfd);
}