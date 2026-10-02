#include "RequestParser.hpp"
#include "HTTPmessage.hpp"
#include <sstream>
#include <iostream>

bool RequestParser::parseRequestLine(const std::string& line,
									 HTTPmessage& request)
{
	std::istringstream stream(line);

	std::string method;
	std::string path;
	std::string version;
	std::string extra;

	if (!(stream >> method >> path >> version))
		return false;
	if (stream >> extra)
		return false;
	if (!isValidMethod(method))
		return false;
	if (!isValidVersion(version))
		return false;
	if (!isValidPath(path))
		return false;

	request.rq_startline.method = method;
	request.rq_startline.raw_path = path;
	request.rq_startline.version = version;

	return true;
}

bool RequestParser::isValidMethod(const std::string& method) const
{
	if (method == "POST")
		return true;
	if (method == "GET")
		return true;
	if (method == "DELETE")
		return true;
	return false;
}

bool RequestParser::isValidVersion(const std::string& version) const
{
	return version == "HTTP/1.1";
}

bool RequestParser::isValidPath(const std::string& path) const
{
	if (path.empty())
		return false;
	if (path[0] != '/')
		return false;

	return true;
}