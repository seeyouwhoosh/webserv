#include "HTTPmessage.hpp"
#include "ConfigParser.hpp"
#include "utils.hpp"
#include <cstring>

/**
 * @brief Having detected "cgi-bin", searches for the executable path
 * 
 * After the "cgi-bin/", looks for the next element, if it's a directory, keep searching; else, attach it to the path (for the cgi thingy), the rest of that is attached to path_info
 * 
 * @example The full path is "/html/cgi-bin/bin/binary/some/other/words". Then, first we cut to "bin/binary/some/other/words". We get until "binary", because there may be a pseudopath for the cgi to use (see PATH_INFO)
 */
void	HTTPmessage::filefromCGIbin(size_t cgibin_pos)
{
	std::stringstream	ss(full_path.substr(cgibin_pos + std::strlen("/cgi-bin/")));
	std::string			segment;
	std::string			current_path = full_path.substr(0, cgibin_pos + std::strlen("/cgi-bin/"));
	bool				found = false;

	while (std::getline(ss, segment, '/'))
	{
		if (segment.empty())
			break;

		if (found)
			rq_startline.path_info += "/" + segment;
		else
		{
			current_path += segment;

			struct stat	st;
			if (stat(current_path.c_str(), &st) == 0)
			{
				if ((st.st_mode & S_IFMT)== S_IFREG)
				{
					rq_startline.iscgi = true;
					full_path = current_path;
					found = true;
				}
				else if ((st.st_mode & S_IFMT)== S_IFDIR)
					current_path += "/";
			}
			else
				break;
		}
	}
}

void	HTTPmessage::indepCGIexecutable(size_t app_pos)
{
	size_t		app_end = full_path.find('/', app_pos);
	std::string	app_ext = (app_end == std::string::npos) ? full_path.substr(app_pos) : full_path.substr(app_pos, app_end - app_pos);

	struct stat	st;
	if (stat(full_path.c_str(), &st) < 0)
		return;

	if (((st.st_mode & S_IFMT) == S_IFREG) && (!(cgi_path = matchCGI(app_ext)).empty() || app_ext == ".cgi"))
	{
		rq_startline.iscgi = true;
		if (app_end != std::string::npos)
		{
			rq_startline.path_info = full_path.substr(0, app_end);
			full_path = full_path.substr(app_end);
		}
		else
			rq_startline.path_info = full_path;
	}
	else
		rq_startline.iscgi = false;
}

void	HTTPmessage::CGIparse()
{
	size_t		app_pos = full_path.find('.');
	size_t		cgibin_pos = full_path.find("/cgi-bin/");

	if (cgibin_pos != std::string::npos)
		filefromCGIbin(cgibin_pos);
	else if (app_pos != std::string::npos)
		indepCGIexecutable(app_pos);
}

void	HTTPmessage::parseCGIoutput(std::string &cgi_output)
{
	size_t				end_of_header = cgi_output.find("\r\n\r\n");
	if (end_of_header == cgi_output.npos)
		return;

	std::string			header_part = cgi_output.substr(0, end_of_header + 4);
	cgi_output.erase(0, end_of_header + 4);

	std::stringstream	ss(header_part);
	std::string			line;

	while (std::getline(ss, line))
	{
		if (!line.empty() && line[line.length() - 1] == '\r')
			line.erase(line.length() - 1, 1);

		if (line.empty())
			continue;

		size_t colon_pos = line.find(':');
		if (colon_pos == std::string::npos)
			continue;

		std::string key = line.substr(0, colon_pos);
		std::string value = line.substr(colon_pos + 1);

		size_t first_non_space = value.find_first_not_of(" \t");
		if (first_non_space != std::string::npos)
			value = value.substr(first_non_space);
		else
			value.clear();
		rs_headers[key] = value;
	}
}

static void	dumpOutput(int pipe, std::string &cgi_output)
{
	char		buffer[BUFFER_SIZE];
	ssize_t		bytes_read;

	while ((bytes_read = read(pipe, buffer, sizeof(buffer))) > 0)
		cgi_output.append(buffer, bytes_read);
}

int	HTTPmessage::handleCGI()
{
	int		pipe_in[2];
	int		pipe_out[2];
	int		status;

	if ((status = checkFileStatus(F_OK | X_OK, full_path)) != OK)
		return status;
	if (pipePipes(pipe_in, pipe_out) < 0)
		return INTERNAL_SERVER_ERROR;

	fillCGIenv();

	pid_t	pid = fork();
	switch (pid)
	{
	case -1:
	{
		closePipes(pipe_in, pipe_out);
		return INTERNAL_SERVER_ERROR;
	}
	case 0:
	{
		std::vector<char *> argv;
		std::vector<char *> envp;

		chdir(full_path.substr(0, full_path.find_last_of('/')).c_str());
		fillArgvEnvp(argv, envp);
	
		dup2(pipe_in[0], STDIN_FILENO);
		dup2(pipe_out[1], STDOUT_FILENO);
		closePipes(pipe_in, pipe_out);
	
		execve(argv[0], argv.data(), envp.data());
		exit(1);
	}
	default:
	{
		close(pipe_in[0]);
		close(pipe_out[1]);

		if (!rq_body.empty())
			write(pipe_in[1], rq_body.c_str(), rq_body.size());
		close(pipe_in[1]);
		
		std::string	cgi_output;
		dumpOutput(pipe_out[0], cgi_output);
		close(pipe_out[0]);

		waitpid(pid, &status, 0);
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
			return INTERNAL_SERVER_ERROR;

		parseCGIoutput(cgi_output);

		std::string	content_len;
		if (!(content_len = rs_headers["Content-Length"]).empty())
		{
			rs_content_length = strtoInt(content_len);
			if (rs_content_length == cgi_output.size())
				this->rs_body = cgi_output; 
			else
				this->rs_body = cgi_output.substr(0, rs_content_length);
		}

		return OK;
	}
	}
}
