#include "utils.hpp"

/**
 * @brief outputs error message if msg is not NULL (else, picks from strerror) and return -err_code
 * @note Takes into account the SHoW_LOGS macro
 w*/
int	error_msg(int err_code, const char *func, const char *msg)
{
	if (!SHOW_LOGS)
		return -err_code;
	std::cerr << "\033[1;31m[Error] \033[0;31m";
	if (func)
		std::cerr << func << ": ";
	if (msg)
		std::cerr << msg << "\n\033[0m";
	else
		std::cerr << strerror(err_code) << "\n\033[0m";
	return -err_code;
}
/**
 * @brief Gets the date in the format "day-of-week, day month year, hour:minutes:second GMT"
 */
std::string	getDate()
{
	time_t		rawtime;
	struct tm	*tm;

	time(&rawtime);
	tm = gmtime(&rawtime);

	if (!tm)
		return "";

	char buffer[30];
	if (strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", tm) == 0)
		return "";

	return std::string(buffer);
}

int	pipePipes(int pipe_in[2], int pipe_out[2])
{
	if (pipe(pipe_out) < 0)
		return -1;

	if (pipe(pipe_in) < 0)
	{
		close(pipe_out[0]);
		close(pipe_out[1]);
		return -1;
	}
	return 0;
}
/**
 * @brief Sets a fd to non blocking
 * @return When success, returns 0, else, closes fd and returns -1
 */
int	setNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
	{
		close(fd);
		return 1;
	}
	return 0;
}

void	closePipes(int pipe_in[2], int pipe_out[2])
{
	close(pipe_in[0]);
	close(pipe_in[1]);
	close(pipe_out[0]);
	close(pipe_out[1]);
}

/**
 * @brief Prepares listening socket
 * 
 * Created a socket, sets it to non blocking, bind it to addr and starts listening
 * 
 * @return If it fails, return value < 0, else returns socket
 * @note Prints out error messages */
int	listen_socket(struct addrinfo *addr, int listen_num)
{
	int	serversock;
	int	status = 1;
	int	yes = 1;

	if (!addr)
		return error_msg(-1, "listen_socket", "not a valid struct");

	for (struct addrinfo *p = addr; p != NULL; p = p->ai_next)
	{
		if ((serversock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
		{
			status = errno;
			continue;
		}
		if (setNonBlocking(serversock) < 0)
			return error_msg(errno, "fcntl", NULL);
		setsockopt(serversock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
		if (bind(serversock, p->ai_addr, p->ai_addrlen) < 0)
		{
			status = errno;
			close(serversock);
			serversock = -1;
			continue;
		}
		if (listen(serversock, listen_num) < 0)
		{
			close(serversock);
			return error_msg(errno, "listen", NULL);
		}
		return serversock;
	}
	return error_msg(status, "listen_socket", NULL);
}

bool	all_digits(std::string s)
{
	for (size_t i = 0; i < s.size(); i++)
	{
		if (!std::isdigit(s[i]))
			return false;
	}
	return true;
}

int matchDirective(std::string &s)
{
	const char	*d[] = {"root", "alias", "autoindex", "max_body_size", "limit_except", "upload_store", "return", "timeout"};

	for (size_t i = 0; i < 8; i++)
	{
		if (s == d[i])
			return 1 << i;
	}
	return 0;
}
