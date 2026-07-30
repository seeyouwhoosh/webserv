#include <sys/socket.h>
#include <iostream>

int	main(int ac, char **av)
{
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		std::cout << "Error: socket failed\n";
		return 1;
	}
}