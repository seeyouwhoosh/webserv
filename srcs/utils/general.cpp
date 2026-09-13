#include "utils.hpp"

/* outputs error message if msg is not NULL (else, picks from strerror) and return -err_code */
int	error_msg(int err_code, const char *func, const char *msg)
{
	std::cerr << "\033[1;31m[Error] \033[0;31m";
	if (func)
		std::cerr << func << ": ";
	if (msg)
		std::cerr << msg << "\n";
	else
		std::cerr << strerror(err_code) << "\n\033[0m";
	return -err_code;
}

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

void	closePipes(int pipe_in[2], int pipe_out[2])
{
	close(pipe_in[0]);
	close(pipe_in[1]);
	close(pipe_out[0]);
	close(pipe_out[1]);
}
