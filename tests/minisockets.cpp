#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <sys/epoll.h>

int	main(int ac, char **av)
{
	/* Create a socket (IPv4, TCP). El tercer argumento es 0 porque suele haber un solo protocolo para para cada tipo (SOCK_STREAM, por ejemplo) */
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		std::cout << "Error: socket failed\n";
		return 1;
	}

	
	int opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // Para que no haya un delay al conectarse por segunda vez
	
	/* Listen to port 9999 on any address
	
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

	sockaddr_in	sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(9999); //convert host number to network byte

	if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == -1)
	{
		std::cout << "Error: bind to port 9999 failed\n";
		return 1;
	}

	size_t addrlen = sizeof(sockaddr);

	// Start listening. Hold at most 10 connections in the queue
	// backlog is the maximum number of connections that will be queued before connections start being refused.

	if (listen(sockfd, 10) == -1)
	{
		std::cout << "Error: listen on socker failed " << errno << "\n";
		return 1;
	}

	// Grab a connection from the queue
	// addrlen is now a value-result argument. It expects a pointer to an int that will be the size of addr. After the function is executed, the int refered by addrlen will be set to the size of the peer address.

	int connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
	if (connection == -1) {
	std::cout << "Error: failed to grab connection " << errno << std::endl;
	exit(EXIT_FAILURE);
	}

	// Read from the connection
	char buffer[100];
	size_t bytesRead;

	while ((bytesRead = read(connection, buffer, 100)))
	{
		buffer[bytesRead] = '\0';
		std::cout << "The message was: " << buffer;
	}
	
	// Send a message to the connection
	std::string response = "Good talking to you\n";
	send(connection, response.c_str(), response.size(), 0);

	// Close the connections
	close(connection);
	close(sockfd);
}