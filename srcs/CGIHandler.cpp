#include "CGIHandler.hpp"
#include "HTTPmessage.hpp"
#include "ServerConfig.hpp"
#include "utils.hpp"

/* Orthodox Canonical form */

CGIHandler::CGIHandler()
{

}

CGIHandler::CGIHandler(CGIHandler &other)
{
	*this = other;
}

CGIHandler	&CGIHandler::operator=(CGIHandler &other)
{
	
	return *this;
}

CGIHandler::~CGIHandler()
{

}

std::string	CGIHandler::matchExtension(const std::string &ext, const LocationConfig *location) const
{
	if (location)
	{
		cgi_handler_type::const_iterator	it = location->cgi_handlers.find(ext);
		if (it != location->cgi_handlers.end())
			return it->second;
	}
	else
		return "";
}

/**
 * @brief Parse for cgi executable based on their extension
 * 
 * If it's a file that matches with one of the cgi handlers or the extension is
 * ".cgi", then sets path_info accordingly and updates full_path to path_info
 * (else full_path is the same)
 * 
 * If cgi-bin is detected, send it to the parseCGIbinFile function
 */
void	CGIHandler::parseFiletoCGI(std::string &full_path, LocationConfig *location)
{
	size_t		cgibin_pos = full_path.find("/cgi-bin/");

	if (cgibin_pos != std::string::npos)
	{
		parseCGIbinFile(cgibin_pos, full_path);
		return;
	}
	
	std::stringstream	ss(full_path);
	std::string			segment;
	std::string			current_path;
	bool				found = false;

	while (std::getline(ss, segment, '/'))
	{
		if (segment.empty())
			break;

		if (found)
			path_info += "/" + segment;
		else
		{
			current_path += segment;

			struct stat	st;
			if (stat(current_path.c_str(), &st) == 0)
			{
				size_t		pos_ext = segment.find_last_of('.');
				std::string	ext_str = segment.substr(pos_ext);
				if ((st.st_mode & S_IFMT)== S_IFREG && (!(cgi_path = matchExtension(ext_str, location)).empty() || ext_str == ".cgi"))
				{
					iscgi = true;
					found = true;
					full_path = current_path;
					cgi_path = current_path;
					filename = segment;
				}
				else if ((st.st_mode & S_IFMT)== S_IFDIR)
					current_path += "/";
			}
			else
				break;
		}
	}

}

bool	CGIHandler::isCGI()
{
	return iscgi;
}

/**
 * @brief Having detected "cgi-bin", searches for the executable path
 * 
 * After the "/cgi-bin/", looks for the next element, if it's a directory, keep searching; else, attach it to the path (for the cgi thingy), the rest of that is attached to path_info
 * 
 * @example The full path is "/html/cgi-bin/bin/binary/some/other/words". Then, first we cut to "bin/binary/some/other/words". We get until "binary", because there may be a pseudopath for the cgi to use (see PATH_INFO)
 * @note Updates full_path
 * @note Updates iscgi
 */
void	CGIHandler::parseCGIbinFile(size_t cgibin_pos, std::string &full_path)
{
	size_t				start_pos = cgibin_pos + std::strlen("/cgi-bin/");
	std::stringstream	ss(full_path.substr(start_pos));
	std::string			segment;
	std::string			current_path = full_path.substr(0, start_pos);
	bool				found = false;

	while (std::getline(ss, segment, '/'))
	{
		if (segment.empty())
			break;

		if (found)
			path_info += "/" + segment;
		else
		{
			current_path += segment;

			struct stat	st;
			if (stat(current_path.c_str(), &st) == 0)
			{
				if ((st.st_mode & S_IFMT)== S_IFREG)
				{
					iscgi = true;
					found = true;
					full_path = current_path;
					cgi_path = current_path;
					filename = segment;
				}
				else if ((st.st_mode & S_IFMT)== S_IFDIR)
					current_path += "/";
			}
			else
				break;
		}
	}
}

static void	dumpOutput(int pipe, std::string &output)
{
	char		buffer[BUFFER_SIZE];
	ssize_t		bytes_read;

	while ((bytes_read = read(pipe, buffer, sizeof(buffer))) > 0)
		output.append(buffer, bytes_read);
}

int	CGIHandler::execute(const HTTPmessage& request, const LocationConfig *location)
{
	int		pipe_in[2];
	int		pipe_out[2];
	int		status;

	if ((status = checkFileStatus(F_OK | X_OK, cgi_path)) != OK)
		return status;
	if (pipePipes(pipe_in, pipe_out) < 0)
		return INTERNAL_SERVER_ERROR;

	fill_env(request);

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
			if ((status = gotoDirectory()) != 0)
				exit(status);
			fill_argv();
			fill_envp();
		
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

			if (!request.rq_body.empty())
				write(pipe_in[1], request.rq_body.c_str(), request.rq_body.size());
			close(pipe_in[1]);
			
			dumpOutput(pipe_out[0], output);
			close(pipe_out[0]);

			waitpid(pid, &status, 0);
			if (!WIFEXITED(status))
			{
				if ((status = WEXITSTATUS(status)) > 0)
					return status;
				return INTERNAL_SERVER_ERROR;
			}

			parseOutput(request);

			std::string	content_len;
			if (!(content_len = request.getHeader("Content-Length")).empty())
			{
				int	content_length = strtoInt(content_len);
				request.setBody(output.substr(0, content_length));
			}

			return OK;
		}
		}
}

void	CGIHandler::parseOutput(HTTPmessage &request)
{
	size_t	end_of_header = output.find("\r\n\r\n");
	if (end_of_header == output.npos)
		return;

	std::string			header_part = output.substr(0, end_of_header + 4);
	output.erase(0, end_of_header + 4);

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
		request.setHeader(key, value);
	}
}


void	CGIHandler::fill_env(const HTTPmessage &request)
{
	cgi_env.reserve(request.rq_headers.size() + 17); // 17 variables from below

	cgi_env.push_back("SERVER_SOFTWARE=Custom/1.0");
	cgi_env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	
	cgi_env.push_back("SERVER_PROTOCOL=" + request.rq_startline.version);
	cgi_env.push_back("SERVER_PORT=" + server_port);
	cgi_env.push_back("SERVER_NAME=" + request.getServerConf()->server_name[0]);
	cgi_env.push_back("REQUEST_METHOD=" + request.rq_startline.method);
	cgi_env.push_back("QUERY_STRING=" + query);

	cgi_env.push_back("CONTENT_LENGTH=" + toString(request.rq_content_length));

	stringmap::const_iterator it = request.rq_headers.find("content-type");
	if (it != request.rq_headers.end())
		cgi_env.push_back("CONTENT_TYPE=" + it->second);
	
	cgi_env.push_back("PATH_INFO=" + path_info);
	cgi_env.push_back("PATH_TRANSLATED=" + request.getServerConf()->root + path_info);

	cgi_env.push_back("AUTH_TYPE=");
	cgi_env.push_back("REMOTE_ADDR=" + client_addr);
	cgi_env.push_back("REMOTE_HOST=" + client_addr);
	cgi_env.push_back("REMOTE_IDENT=");
	cgi_env.push_back("REMOTE_USER=");

	cgi_env.push_back("SCRIPT_NAME=" + filename);

	for (stringmap::const_iterator it = request.rq_headers.begin(); it != request.rq_headers.end(); it++)
	{
		if (it->first == "content-type" || it->first == "content-length")
			continue;
		std::string	var = it->first;
		toupperString(var);
		std::replace(var.begin(), var.end(), '-', '_');
		cgi_env.push_back("HTTP_" + var + "=" + it->second);
	}
}

void	CGIHandler::fill_argv()
{
	if (!cgi_path.empty())
	{
		argv.push_back(const_cast<char *>(filename.c_str()));
		argv.push_back(const_cast<char *>(cgi_path.c_str()));
	}
	else 
		argv.push_back(const_cast<char *>(cgi_path.c_str()));
	argv.push_back(NULL);
}

void	CGIHandler::fill_envp()
{
	for (size_t i = 0; i < cgi_env.size(); ++i)
		envp.push_back(const_cast<char *>(cgi_env[i].c_str()));
	envp.push_back(NULL);
}

int	CGIHandler::gotoDirectory()
{
	std::string	dir_path = cgi_path.substr(0, cgi_path.find_last_of('/'));
	struct stat	st;
	if (stat(dir_path.c_str(), &st) != 0)
		return FORBIDDEN;
	else if ((st.st_mode & S_IFMT) != S_IFDIR)
		return UNPROCESSABLE_CONTENT;
	else
		return chdir(dir_path.c_str());
}
