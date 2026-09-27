#include "ConfigParser.hpp"

int main(int argc, char* argv[]) {
	std::string filepath = (argc > 1) ? argv[1] : "../../config_files/sample.conf";

	ConfigParser				parser;
	std::vector<ServerConfig>	servers;

	if (parser.parseConfig(&filepath[0], &servers) < 0)
		return 0;


	return 0;
}
