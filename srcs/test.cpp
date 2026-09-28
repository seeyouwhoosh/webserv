#include "ConfigParser.hpp"
#include <iostream>

void printVector(const std::vector<std::string>& vec) {
    std::cout << "[ ";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i + 1 < vec.size() ? ", " : "");
    }
    std::cout << " ]\n";
}

void printMap(const std::map<int, std::string>& mapData) {
    for (std::map<int, std::string>::const_iterator it = mapData.begin(); it != mapData.end(); ++it) {
        std::cout << "	" << it->first << " -> " << it->second << "\n";
    }
	std::cout << std::endl;
}

void printMapString(const std::map<std::string, std::string>& mapData) {
    for (std::map<std::string, std::string>::const_iterator it = mapData.begin(); it != mapData.end(); ++it) {
        std::cout << "	ext: " << it->first << " -> binary: " << it->second << "\n";
    }
	std::cout << std::endl;
}

// --- Impresión de LocationConf ---

int main(int argc, char* argv[]) {
	std::string filepath = (argc > 1) ? argv[1] : "config_files/sample.conf";

	ConfigParser				parser;
	std::vector<ServerConfig>	servers;

	std::cout << "Atempting to open " << filepath << std::endl;
	if (parser.parseConfig(&filepath[0], &servers) < 0)
		return 0;

	for (std::vector<ServerConfig>::iterator it = servers.begin(); it != servers.end(); it++)
	{
		std::cout << "\n------- SERVER ------\n";
		std::cout << "server_name: "; printVector(it->server_name);
		std::cout << "root: " << it->root << std::endl;
		std::cout << "autoindex: " << it->autoindex << std::endl;
		std::cout << "max_body_size: "<< it->str_max_body_size << std::endl;
		std::cout << "index: "; printVector(it->indexes);
		std::cout << "error_page: "; printMap(it->error_pages);
		std::cout << "cgi_handler: "; printMapString(it->cgi_handlers);

		std::cout << "listens: " << std::endl;
		for (std::vector<t_listen>::iterator jt = it->listens.begin(); jt != it->listens.end(); jt++)
		{
			std::cout << "	- host: " << jt->host << " -> port: " << jt->port << std::endl;
		}
		std::cout << "\n------- LOCATIONS ------\n";
		for (std::vector<LocationConfig>::iterator jt = it->locations.begin(); jt != it->locations.end(); jt++)
		{
			std::cout << "\n------- Location " << jt->name << " ------\n";
			std::cout << "root: " << jt->root << std::endl;
			std::cout << "autoindex: " << jt->autoindex << std::endl;
			std::cout << "max_body_size: "<< jt->str_max_body_size << std::endl;
			std::cout << "upload: " << jt->upload_store << std::endl;
			std::cout << "index: "; printVector(jt->indexes);
			std::cout << "error_page: "; printMap(jt->error_pages);
			std::cout << "return: " << jt->return_status.first << " -> uri: " << jt->return_status.second << std::endl;
			std::cout << "cgi_handler: "; printMapString(jt->cgi_handlers);
			std::cout << "allowed_methods: "; printVector(jt->allowed_methods);
		}
		std::cout << "\n\n";
	}

	return 0;
}
