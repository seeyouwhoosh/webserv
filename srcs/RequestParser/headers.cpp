#include "RequestParser.hpp"
#include "HTTPmessage.hpp"
#include <sstream>
#include <map>
#include <cctype>
#include <iostream>

bool RequestParser::parseHeader(const std::string& line,
								HTTPmessage& request)
{
	std::string::size_type separator = line.find(':');

	if (separator == std::string::npos)
		return false;
	if (separator == 0)
		return false;

	for (std::string::size_type i = 0; i < separator; i++)
	{
		if (line[i] == ' ' || line[i] == '\t')
			return false;
	}

	std::string name = line.substr(0, separator);
	std::string value = line.substr(separator + 1);

	for (std::string::size_type i = 0; i < name.size(); i++)
		name[i] = std::tolower(name[i]);

	while (!value.empty() && value[0] == ' ')
		value.erase(0, 1);

	std::cout << "He llegado hasta aquí " << std::endl;

	if (request.rq_headers.find(name) != request.rq_headers.end())
		return false;

	request.rq_headers[name] = value;

	return true;
}

bool RequestParser::parseContentLength(const HTTPmessage& request)
{
	std::map<std::string, std::string>::const_iterator it;

	it = request.rq_headers.find("content-length");

	if (it == request.rq_headers.end())
	{
		_contentLength = 0;
		return true;
	}

	if (it->second.empty())
		return false;

	for (std::string::size_type i = 0; i < it->second.size(); i++)
	{
		if (it->second[i] < '0' || it->second[i] > '9')
			return false;
	}

	std::istringstream stream(it->second);

	std::size_t length;
	std::string extra;

	if (!(stream >> length))
		return false;
	if (stream >> extra)
		return false;

	_contentLength = length;

	return true;
}

bool RequestParser::hasChunkedEncoding(const HTTPmessage& request) const
{
	std::map<std::string, std::string>::const_iterator it;
	std::string value;

	it = request.rq_headers.find("transfer-encoding");

	if (it == request.rq_headers.end())
		return false;

	value = it->second;

	for (std::string::size_type i = 0; i < value.size(); i++)
		value[i] = std::tolower(value[i]);

	while (!value.empty() && value[0] == ' ')
		value.erase(0, 1);

	while (!value.empty() && value[value.size() - 1] == ' ')
		value.erase(value.size() - 1, 1);

	return value == "chunked";
}

bool RequestParser::hasContentLength(const HTTPmessage& request) const
{
	return request.rq_headers.find("content-length")
		!= request.rq_headers.end();
}

bool RequestParser::hasHost(const HTTPmessage& request) const
{
	std::map<std::string, std::string>::const_iterator it;

	it = request.rq_headers.find("host");

	if (it == request.rq_headers.end())
		return false;

	if (it->second.empty())
		return false;

	return true;
}