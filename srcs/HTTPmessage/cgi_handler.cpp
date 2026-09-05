#include "HTTPmessage.hpp"

void	HTTPmessage::fillCGIenv()
{
	/* ALL REQUESTS */
	cgi_env.push_back(std::string("SERVER_SOFTWARE=") + "Custom/1.0");
	cgi_env.push_back(std::string("GATEWAY_INTERFACE=") + "cgi/1.1");
	
	cgi_env.push_back("SERVER_PROTOCOL=" + rq_startline.version);
	cgi_env.push_back("SERVER_PORT=" + server_port);
	cgi_env.push_back("SERVER_NAME=" + serverconf->server_name);
	cgi_env.push_back("REQUEST_METHOD=" + rq_startline.method);
	cgi_env.push_back("QUERY_STRING=" + rq_startline.query);

	cgi_env.push_back("CONTENT_LENGTH=" + rq_content_length);
	cgi_env.push_back("CONTENT_TYPE=" + rq_headers["Content-Type"]);
	cgi_env.push_back("PATH_INFO=" + rq_startline.path_info);
	cgi_env.push_back("PATH_TRANSLATED=" + servercong->root + rq_startline.path_info);

	cgi_env.push_back("AUTH_TYPE=");
	cgi_env.push_back("REMOTE_ADDR=" + client_addr);
	cgi_env.push_back("REMOTE_HOST=" + client_addr);
	cgi_env.push_back("REMOTE_IDENT=");
	cgi_env.push_back("REMOTE_USER=");

	cgi_env.push_back("SCRIPT_NAME=" + rq_startline.file);

	for (stringmap::iterator it = rq_headers.begin(); it != rq_headers.end(); it++)
	{
		std::string	var = it->first;
		std::transform(var.begin(), var.end(), var.begin(), std::toupper);
		std::replace(var.begin(), var.end(), '-', '_');
		cgi_env.push_back("HTTP_" + var + "=" + it->second)
	}
	
	cgi_env.push_back(NULL);
}

std::string	HTTPmessage::matchCGI(std::string extension, LocationConfig &locationconf)
{
	cgi_handler_type::iterator	it = locationconf.cgi_handlers.find(extension);

	if (it == locationconf.cgi_handlers.end())
		return "";
	
	return it->second;
}

std::string	HTTPmessage::matchCGI(std::string extension, ServerConfig &serverconf)
{
	cgi_handler_type::iterator	it = serverconf.cgi_handlers.find(extension);

	if (it == serverconf.cgi_handlers.end())
		return "";
	
	return it->second;
}

void	HTTPmessage::CGIparse(ServerConfig &serverconf)
{
	size_t		app_pos = full_path.find('.');
	size_t		cgibin_pos = full_path.find("/cgi-bin/");
	struct stat	st;

	if (cgibin_pos != std::string::npos)
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

				struct stat st;
				if (stat(current_path.c_str(), &st) == 0)
				{
					if ((st.st_mode & S_IFMT)== S_ISREG)
					{
						rq_startline.iscgi = true;
						full_path = current_path;
						found = true;
					}
					else if ((st.st_mode & S_IFMT)== S_ISDIR)
						current_path += "/";
				}
				else
					break; 
		}
	}
	else if (app_pos != std::string::npos)
	{
		size_t		app_end = full_path.find('/', app_pos);
		std::string	app_ext = (app_end == std::string::npos) ? full_path.substr(app_pos + 1) : full_path.substr(app_pos + 1, app_end - app_pos - 1);

		stat(full_path.c_str(), &st);
		if (((index_st.st_mode & S_IFMT) == S_IFREG) && (cgi_path = matchCGI(app_ext, serverconf)) != "")
		{
			rq_startline.iscgi = true;
			rq_startline.path_info = full_path.substr(app_end);
			full_path = full_path.substr(0, app_end);
		}
		else
			rq_startline.iscgi = false;
	}
}

void	HTTPmessage::CGIparse(LocationConfig &locationconf)
{
	size_t		app_pos = full_path.find('.');
	size_t		cgibin_pos = full_path.find("/cgi-bin/");
	struct stat	st;

	if (cgibin_pos != std::string::npos)
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

				struct stat st;
				if (stat(current_path.c_str(), &st) == 0)
				{
					if ((st.st_mode & S_IFMT)== S_ISREG)
					{
						rq_startline.iscgi = true;
						full_path = current_path;
						found = true;
					}
					else if ((st.st_mode & S_IFMT)== S_ISDIR)
						current_path += "/";
				}
				else
					break; 
		}
	}
	else if (app_pos != std::string::npos)
	{
		size_t		app_end = full_path.find('/', app_pos);
		std::string	app_ext = (app_end == std::string::npos) ? full_path.substr(app_pos + 1) : full_path.substr(app_pos + 1, app_end - app_pos - 1);

		stat(full_path.c_str(), &st);
		if (((index_st.st_mode & S_IFMT) == S_IFREG) && (cgi_path = matchCGI(app_ext, locationconf)) != "")
		{
			rq_startline.iscgi = true;
			rq_startline.path_info = full_path.substr(app_end);
			full_path = full_path.substr(0, app_end);
		}
		else
			rq_startline.iscgi = false;
	}
}

int	HTTPmessage::handle_cgi()
{
	struct stat	st;

	if (stat(full_path.c_str(), &st) < 0)
		return NOT_FOUND;

	if (access(full_path.c_str(), F_OK | X_OK) < 0)
		return FORBIDDEN;
	
	int pipe_in[2];
	int pipe_out[2];
	pid_t	pid;
	if (pipe(pipe_in) < 0 || pipe(pipe_out) < 0)
		return INTERNAL_SERVER_ERROR;
	pid = fork();
	switch (pid)
	{
	case -1:
		close(pipe_in[0]);
		close(pipe_in[1]);
		close(pipe_out[0]);
		close(pipe_out[1]);
		return INTERNAL_SERVER_ERROR;
	case 0:
		dup2(pipe_in[0], STDIN_FILENO);
		dup2(pipe_out[1], STDOUT_FILENO);
		close(pipe_in[0]);
		close(pipe_in[1]);
		close(pipe_out[0]);
		close(pipe_out[1]);

		std::vector<char*> argv;

		if (!cgi_path.empty()) 
		{
			argv.push_back(interpreter.c_str());
			argv.push_back(full_path.c_str());
		} 
		else 
			argv.push_back(const_cast<char*>(full_path.c_str()));
		argv.push_back(NULL);

		execve(argv[0], argv.data(), cgi_env.data());
		exit(1);
	default:
		close(pipe_in[0]);
		close(pipe_out[1]);

		if (!body.empty())
			write(pipe_in[1], body.c_str(), body.size());
		close(pipe_in[1]);

		char buffer[1024];
		ssize_t bytes_read;
		std::string cgi_output;

		while ((bytes_read = read(pipe_out[0], buffer, sizeof(buffer))) > 0)
			cgi_output.append(buffer, bytes_read);
		close(pipe_out[0]);
		int status;
		waitpid(pid, &status, 0);

		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
			return INTERNAL_SERVER_ERROR;
		}

		this->response_body = cgi_output; 

		return OK;
	}
	/* Mirar que el archivo está bien, existe y es ejecutable, si no, poner el código de error */
	/* Contruir el pipe, fork y el env para ejecutarlo y redirigirlo a algún archivo o algo así*/
}