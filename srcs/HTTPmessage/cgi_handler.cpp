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

std::string	HTTPmessage::matchCGI(std::string extension)
{
	cgi_handler_type::iterator	it = serverconf->cgi_handlers.find(extension);

	if (it == serverconf->cgi_handlers.end())
		return "";
	
	return it->second;
}

int	HTTPmessage::handle_cgi()
{
	struct stat	*st;

	if (stat(full_path.c_str(), st) < 0)
		return NOT_FOUND;

	if (access(full_path.c_str(), F_OK | X_OK) < 0)
		return FORBIDDEN;
	
	

	int		fd[2];
	pid_t	pid;

	pid = fork();
	switch (pid)
	{
	case -1:
		return INTERNAL_SERVER_ERROR;
	case 0:
		execve(,, cgi_env.data())
		break;
	default:
		break;
	}
	/* Mirar que el archivo está bien, existe y es ejecutable, si no, poner el código de error */
	/* Contruir el pipe, fork y el env para ejecutarlo y redirigirlo a algún archivo o algo así*/
}