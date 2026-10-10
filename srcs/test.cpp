#include <iostream>
#include <string>

// Incluye aquí los headers reales de tus clases
#include "CGIHandler.hpp"
#include "HTTPmessage.hpp"
#include "LocationConfig.hpp"
#include "ServerConfig.hpp"

int main() {
    // 1. Instanciar objetos de prueba (ajusta los constructores según tu código)
    CGIHandler cgiHandler;
    HTTPmessage request;
    LocationConfig location;


    // Simular una ruta de archivo inicial (ej. script en un directorio cgi-bin o con extensión .py/.php)
    std::string full_path = "./var/www/cgi-bin/cgi.py/algo/mas?query=2344";

	location.cgi_handlers[".cgi"] = "/usr/bin/python3";
	request.rq_startline.version = "HTTP/1.1";
	request.rq_startline.raw_path = full_path;
	request.rq_startline.method = "GET";
	request.rq_body = "hola\n";
	request.rq_content_length = 5;
	ServerConfig	server;
	server.server_name.push_back("Prueba");
	request.setServerConf(&server);
    request.setServerPort("9999");
    request.rq_headers["content-type"] = "plain/text";


    std::cout << "=== PRUEBA DE CGI ===" << std::endl;
    std::cout << "Ruta inicial: " << full_path << std::endl;

	cgiHandler.separateQuery(full_path);
	std::cout << "Ruta después de separateQuery: " << full_path << std::endl;
    // 2. Probar parseFiletoCGI
    cgiHandler.parseFiletoCGI(full_path, &location);
    std::cout << "Ruta después de parseFiletoCGI: " << full_path << std::endl;

    // 3. Probar isCGI
    if (cgiHandler.isCGI()) {
        std::cout << "[OK] El archivo fue identificado correctamente como CGI." << std::endl;
    } else {
        std::cout << "[AVISO] El archivo NO fue identificado como CGI." << std::endl;
    }
	
    // 4. Probar execute
    int status_code = cgiHandler.execute(request);
    std::cout << "Codigo de estado devuelto por execute: " << status_code << std::endl;
	cgiHandler.printParameters();
	
    // 5. Probar clear
    cgiHandler.clear();
    std::cout << "Estado limpiado correctamente con clear()." << std::endl;

    return 0;
}