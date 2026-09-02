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
	cgi_env.push_back("PATH_INFO=");
	cgi_env.push_back("PATH_TRANSLATED=");

	cgi_env.push_back("AUTH_TYPE=");
	cgi_env.push_back("REMOTE_ADDR=");
	cgi_env.push_back("REMOTE_HOST=");
	cgi_env.push_back("REMOTE_IDENT=");
	cgi_env.push_back("REMOTE_USER=");

	cgi_env.push_back("SCRIPT_NAME=" + rq_startline.file);
	cgi_env.push_back(NULL);
}

std::string	HTTPmessage::matchCGI(std::string extension)
{
	cgi_handler_type::iterator	it = serverconf->cgi_handlers.find(extension);

	if (it == serverconf->cgi_handlers.end())
		return "";
	
	return it->second;
}

