#include "Parser.hpp"
#include <sstream>
#include <map>
#include <algorithm>

Parser::Parser()
	: _state(REQUEST_LINE), _contentLength(0), _bodyReceived(0)
{
}

Parser::~Parser()
{
}

Parser::Parser(const Parser &other)
{
	*this = other;
}

Parser	&Parser::operator=(const Parser &other)
{
	this->_state = other._state;
	this->_contentLength = other._contentLength;
	this->_bodyReceived = other._bodyReceived;
	return *this;
}

//1.Request Line

bool Parser::parseRequestLine(const std::string& line, HTTPmessage& request)
{
	std::istringstream stream(line); //return the string so it can be read with >>

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
	request.rq_startline.path = path;
	request.rq_startline.version = version;

	return true;
}

bool Parser::isValidMethod(const std::string& method) const
{
	if (method == "GET")
		return true;
	if (method == "POST")
		return true;
	if (method == "DELETE")
		return true;
	
	return false;
}

bool Parser::isValidVersion(const std::string& version) const
{
	return version == "HTTP/1.1";
}

bool Parser::isValidPath(const std::string& path) const
{
	if (path.empty())
		return false;
	if (path[0] != '/')
		return false;

	return true;
}

//2.Headers

bool Parser::parseHeader(const std::string& line, HTTPmessage& request)
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

	while (!value.empty() && value[0] == ' ')
		value.erase(0, 1);
	if (request.rq_headers.find(name) != request.rq_headers.end())
		return false;

	request.rq_headers[name] = value;
	return true;
}

void Parser::appendData(const std::string& data)
{
	_buffer += data;
}

bool Parser::parseContentLength(const HTTPmessage& request)
{
	std::map<std::string, std::string>::const_iterator it;

	it = request.rq_headers.find("Content-Length");

	if (it == request.rq_headers.end())
	{
		_contentLength = 0;
		return true;
	}
	if (it->second.empty())
		return false;
	if (it->second[0] == '-')
		return false;

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

//to get next line
bool Parser::getNextLine(std::string& line)
{
	std::string::size_type end = _buffer.find("\r\n");

	if (end == std::string::npos)
		return false;

	line = _buffer.substr(0, end);

	_buffer.erase(0, end + 2);

	return true;
}

bool Parser::parseBody(HTTPmessage& request)
{
	std::size_t remaining = _contentLength - _bodyReceived;
	std::size_t toRead = std::min(remaining, _buffer.size());

	request.rq_body += _buffer.substr(0, toRead);

	_bodyReceived += toRead;

	_buffer.erase(0, toRead);

	if (_bodyReceived == _contentLength)
		_state = COMPLETE;

	return true;
}

bool Parser::hasChunkedEncoding(const HTTPmessage& request) const
{
	std::map<std::string, std::string>::const_iterator it;

	it = request.rq_headers.find("Transfer-Encoding");

	if (it == request.rq_headers.end())
		return false;

	return it->second == "chunked";
}

ParseResult Parser::parseRequest(HTTPmessage& request)
{
	if (_state == REQUEST_LINE)
	{
		std::string line;

		if (!getNextLine(line))
			return PARSE_INCOMPLETE;

		if (!parseRequestLine(line, request))
			return PARSE_ERROR;

		_state = HEADERS;
	}
	if (_state == HEADERS)
	{
		std::string line;

		while (getNextLine(line))
		{
			if (line.empty())
			{
				if (hasChunkedEncoding(request))
				{
					_state = CHUNK_SIZE;
				}
				else
				{
					if (!parseContentLength(request))
						return PARSE_ERROR;
					if (_contentLength == 0)
						_state = COMPLETE;
					else
						_state = BODY;
				}
				break;
			}
			if (!parseHeader(line, request))
				return PARSE_ERROR;
		}
	}
	if (_state == BODY)
	{
		if (!parseBody(request))
			return PARSE_ERROR;
	}

	if (_state == COMPLETE)
		return PARSE_COMPLETE;

	return PARSE_INCOMPLETE;
}