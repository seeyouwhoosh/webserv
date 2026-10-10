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
	std::string					cgi_path;
	std::string					path_info;
	std::string					query;
	bool						iscgi;

	void	fill_env(HTTPmessage &request);
	void	fill_argv(std::string &script, std::string &path);
	void	fill_envp();
	void	parseCGIbinFile();

public:
	CGIHandler();
	CGIHandler(CGIHandler &other);
	CGIHandler	&operator=(CGIHandler &other);
	~CGIHandler();

	void	parseFiletoCGI(const HTTPmessage& request, const LocationConfig& config);

	int		execute(const HTTPmessage& request, const LocationConfig& config);
	void	clear();

	bool	isCGI();
};

/**
 * @note Probablemente sería buena idea hacer que el contenido se envíe directamente desde esta clase (solo podemos suponer que está bien tal y como está después de ejecutar) -> No es posible porque hay un queue de respuestas -> mejor enviarlo directamente al cliente 
 * 
 * El programa cgi tampoco puede superar el timeout, empieza contar desde la última actividad del cliente
 */