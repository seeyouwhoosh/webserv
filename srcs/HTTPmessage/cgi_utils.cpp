#include "HTTPmessage.hpp"

void	HTTPmessage::fillCGIenv()
{
	cgi_env.reserve(rq_headers.size() + 17);

	cgi_env.push_back(std::string("SERVER_SOFTWARE=") + "Custom/1.0");
	cgi_env.push_back(std::string("GATEWAY_INTERFACE=") + "cgi/1.1");
	
	cgi_env.push_back("SERVER_PROTOCOL=" + rq_startline.version);
	cgi_env.push_back("SERVER_PORT=" + server_port);
	cgi_env.push_back("SERVER_NAME=" + serverconf->server_name);
	cgi_env.push_back("REQUEST_METHOD=" + rq_startline.method);
	cgi_env.push_back("QUERY_STRING=" + rq_startline.query);

	cgi_env.push_back("CONTENT_LENGTH=" + toString(rq_content_length));

	stringmap::iterator it = rq_headers.find("Content-Type");
	if (it != rq_headers.end())
		cgi_env.push_back("CONTENT_TYPE=" + it->second);
	
	cgi_env.push_back("PATH_INFO=" + rq_startline.path_info);
	cgi_env.push_back("PATH_TRANSLATED=" + serverconf->root + rq_startline.path_info);

	cgi_env.push_back("AUTH_TYPE=");
	cgi_env.push_back("REMOTE_ADDR=" + client_addr);
	cgi_env.push_back("REMOTE_HOST=" + client_addr);
	cgi_env.push_back("REMOTE_IDENT=");
	cgi_env.push_back("REMOTE_USER=");

	cgi_env.push_back("SCRIPT_NAME=" + rq_startline.file);

	for (stringmap::iterator it = rq_headers.begin(); it != rq_headers.end(); it++)
	{
		std::string	var = it->first;
		toupperString(var);
		std::replace(var.begin(), var.end(), '-', '_');
		cgi_env.push_back("HTTP_" + var + "=" + it->second);
	}
}

std::string	HTTPmessage::matchCGI(std::string extension, ServerConfig &serverconf, LocationConfig *locationconf)
{
	cgi_handler_type::iterator	it = locationconf ? locationconf->cgi_handlers.find(extension) : serverconf.cgi_handlers.find(extension);

	if (it == serverconf.cgi_handlers.end())
		return "";
	
	return it->second;
}

void	HTTPmessage::fillArgvEnvp(std::vector<char *> &argv, std::vector<char *> &envp)
{
	if (!cgi_path.empty())
	{
		argv.push_back(const_cast<char *>(cgi_path.c_str()));
		argv.push_back(const_cast<char *>(full_path.c_str()));
	}
	else 
		argv.push_back(const_cast<char *>(full_path.c_str()));
	argv.push_back(NULL);

	for (size_t i = 0; i < cgi_env.size(); ++i)
		envp.push_back(const_cast<char*>(cgi_env[i].c_str()));
	envp.push_back(NULL);
}
