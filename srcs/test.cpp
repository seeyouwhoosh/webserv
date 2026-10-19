
#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "ConfigParser.hpp"

static void separator()
{
    std::cout << "========================================\n";
}

static void printContext(const HTTPContext &ctx)
{
    std::cout << "  root: " << ctx.root << "\n";

    std::cout << "  indexes: ";
    if (ctx.indexes.empty())
        std::cout << "(empty)";
    for (size_t i = 0; i < ctx.indexes.size(); ++i)
    {
        if (i)
            std::cout << ", ";
        std::cout << ctx.indexes[i];
    }
    std::cout << "\n";

    std::cout << "  autoindex: ";
    if (ctx.autoindex == AUTOINDEX_UNSET)
        std::cout << "UNSET";
    else if (ctx.autoindex == AUTOINDEX_ON)
        std::cout << "ON";
    else if (ctx.autoindex == AUTOINDEX_OFF)
        std::cout << "OFF";
    else
        std::cout << "INVALID (" << ctx.autoindex << ")";
    std::cout << "\n";

    std::cout << "  str_max_body_size: "
              << ctx.str_max_body_size << "\n";
    std::cout << "  max_body_size: "
              << ctx.max_body_size << "\n";

    std::cout << "  str_timeout: "
              << ctx.str_timeout << "\n";
    std::cout << "  timeout: "
              << ctx.timeout << "\n";

    std::cout << "  str_cgi_timeout: "
              << ctx.str_cgi_timeout << "\n";
    std::cout << "  cgi_timeout: "
              << ctx.cgi_timeout << "\n";

    std::cout << "  error_pages:\n";
    if (ctx.error_pages.empty())
        std::cout << "    (empty)\n";

    for (std::map<int, std::string>::const_iterator it =
             ctx.error_pages.begin();
         it != ctx.error_pages.end(); ++it)
    {
        std::cout << "    " << it->first
                  << " -> " << it->second << "\n";
    }

    std::cout << "  cgi_handlers:\n";
    if (ctx.cgi_handlers.empty())
        std::cout << "    (empty)\n";

    for (cgi_handler_type::const_iterator it =
             ctx.cgi_handlers.begin();
         it != ctx.cgi_handlers.end(); ++it)
    {
        std::cout << "    " << it->first
                  << " -> " << it->second << "\n";
    }
}

static void printLocation(const LocationConfig &loc)
{
    separator();
    std::cout << "LOCATION: " << loc.name << "\n";

    printContext(loc);

    std::cout << "  alias: " << loc.alias << "\n";

    std::cout << "  allowed_methods: ";
    if (loc.allowed_methods.empty())
        std::cout << "(empty)";
    for (size_t i = 0; i < loc.allowed_methods.size(); ++i)
    {
        if (i)
            std::cout << ", ";
        std::cout << loc.allowed_methods[i];
    }
    std::cout << "\n";

    std::cout << "  upload_store: "
              << loc.upload_store << "\n";

    std::cout << "  return_status: "
              << loc.return_status.first << " -> "
              << loc.return_status.second << "\n";
}

static void printServer(const ServerConfig &server, size_t number)
{
    separator();
    std::cout << "SERVER #" << number << "\n";

    printContext(server);

    std::cout << "  server_name: ";
    if (server.server_name.empty())
        std::cout << "(empty)";
    for (size_t i = 0; i < server.server_name.size(); ++i)
    {
        if (i)
            std::cout << ", ";
        std::cout << server.server_name[i];
    }
    std::cout << "\n";

    std::cout << "  listens:\n";
    if (server.listens.empty())
        std::cout << "    (empty)\n";

    for (size_t i = 0; i < server.listens.size(); ++i)
    {
        std::cout << "    " << server.listens[i].host
                  << ":" << server.listens[i].port << "\n";
    }

    for (size_t i = 0; i < server.locations.size(); ++i)
        printLocation(server.locations[i]);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0]
                  << " <config_file>\n";
        return 1;
    }

    ConfigParser parser;
    std::vector<ServerConfig> servers;

    std::cout << "Parsing configuration: "
              << argv[1] << "\n";

    int result = parser.parseConfig(argv[1], &servers);

    if (result != 0)
    {
        std::cerr << "\n[FAIL] Configuration parsing failed.\n";
        if (!parser.error_par.empty())
            std::cerr << parser.error_par << "\n";
        return 1;
    }

    std::cout << "\n[PASS] Configuration parsed successfully.\n";
    std::cout << "Servers found: " << servers.size() << "\n";

    separator();
    std::cout << "GLOBAL CONFIGURATION\n";
    printContext(parser.globalconf);

    for (size_t i = 0; i < servers.size(); ++i)
        printServer(servers[i], i + 1);

    separator();
    std::cout << "END OF CONFIGURATION\n";

    return 0;
}
