#pragma once
#include <vector>
#include <string>

class HTTPmessage;
class LocationConfig;

class CGIHandler
{
private:
	/* Client constants */
	std::string					server_port;
	std::string					client_addr;

	/* Variables */
	std::vector<std::string>	cgi_env;
	std::vector<char *>			argv;
	std::vector<char *>			envp;
	std::string					cgi_path;
	std::string					filename;
	std::string					path_info;
	std::string					query;
	bool						iscgi;
	std::string					output;

	void			fill_env(const HTTPmessage &request);
	void			fill_argv();
	void			fill_envp();
	void			parseCGIbinFile(size_t cgibin_pos, std::string &full_path);
	std::string		matchExtension(const std::string &ext, const LocationConfig *location) const;
	int				gotoDirectory();
	void			parseOutput(HTTPmessage &request);

public:
	CGIHandler();
	CGIHandler(CGIHandler &other);
	CGIHandler	&operator=(CGIHandler &other);
	~CGIHandler();

	void	parseFiletoCGI(std::string &full_path, LocationConfig *location);

	int		execute(const HTTPmessage& request, const LocationConfig *location);
	void	clear();

	bool	isCGI();
};

/**
 * @note Probablemente sería buena idea hacer que el contenido se envíe directamente desde esta clase (solo podemos suponer que está bien tal y como está después de ejecutar) -> No es posible porque hay un queue de respuestas -> mejor enviarlo directamente al cliente 
 * 
 * El programa cgi tampoco puede superar el timeout, empieza contar desde la última actividad del cliente
 */