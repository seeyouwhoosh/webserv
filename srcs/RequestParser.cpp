/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xwu <xwu@student.42urduliz.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/29 22:20:15 by xwu               #+#    #+#             */
/*   Updated: 2026/10/17 18:37:19 by xwu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestParser.hpp"

/* ----------------------- Orthodox Canonical Form ----------------------- */

RequestParser::RequestParser()
	: _state(REQUEST_LINE), _contentLength(0),
	  _bodyReceived(0), _chunkSize(0)
{
}

RequestParser::RequestParser(const RequestParser &other)
	: _state(other._state), _contentLength(other._contentLength),
	  _bodyReceived(other._bodyReceived), _chunkSize(other._chunkSize)
{
}

RequestParser	&RequestParser::operator=(const RequestParser &other)
{
	_state = other._state;
	_contentLength = other._contentLength;
	_bodyReceived = other._bodyReceived;
	_chunkSize = other._chunkSize;
	return *this;
}

RequestParser::~RequestParser()
{
}

/* -------------------------- Main user functions -------------------------- */

/**
 * @brief Resets the parser state and clears the counters for a new request.
 */
void RequestParser::clear()
{
	_state = REQUEST_LINE;
	_contentLength = 0;
	_bodyReceived = 0;
	_chunkSize = 0;
}

/**
 * @brief Appends newly received data to the parser buffer.
 */
void RequestParser::appendData(const std::string& data)
{
	_buffer += data;
}

/**
 * @brief Parses the buffered data according to the current parser state.
 * 
 * @return Returns whether the request is complete, incomplete, or invalid.
 */
ParseResult RequestParser::parse(HTTPmessage& request)
{
	while (_state != COMPLETE)
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
		else if (_state == HEADERS)
		{
			std::string line;

			if (!getNextLine(line))
				return PARSE_INCOMPLETE;
			if (line.empty())
			{
				if (!hasHost(request))
					return PARSE_ERROR;
				if (hasChunkedEncoding(request) && hasContentLength(request))
					return PARSE_ERROR;
				if (request.rq_headers.find("transfer-encoding")
					!= request.rq_headers.end()
					&& !hasChunkedEncoding(request))
					return PARSE_ERROR;
				if (hasChunkedEncoding(request))
					_state = CHUNK_SIZE;
				else
				{
					if (!parseContentLength(request))
						return PARSE_ERROR;
					if (_contentLength == 0)
						_state = COMPLETE;
					else
						_state = BODY;
				}
			}
			else
			{
				if (!parseHeader(line, request))
					return PARSE_ERROR;
			}
		}
		else if (_state == BODY)
		{
			return parseBody(request);
		}
		else if (_state == CHUNK_SIZE)
		{
			ParseResult result = parseChunkSize();

			if (result == PARSE_ERROR)
				return PARSE_ERROR;
			if (result == PARSE_COMPLETE)
				return PARSE_COMPLETE;
			if (result == PARSE_INCOMPLETE && _state == CHUNK_SIZE)
				return PARSE_INCOMPLETE;
		}
		else if (_state == CHUNK_DATA)
		{
			ParseResult result = parseChunkData(request);

			if (result == PARSE_ERROR)
				return PARSE_ERROR;
			if (result == PARSE_COMPLETE)
				return PARSE_COMPLETE;
			if (result == PARSE_INCOMPLETE && _state == CHUNK_DATA)
				return PARSE_INCOMPLETE;
		}
		else if (_state == CHUNK_CRLF)
		{
			ParseResult result = parseChunkCRLF(request);

			if (result == PARSE_ERROR)
				return PARSE_ERROR;
			if (result == PARSE_COMPLETE)
				return PARSE_COMPLETE;
			if (result == PARSE_INCOMPLETE && _state == CHUNK_CRLF)
				return PARSE_INCOMPLETE;
		}
		else
			return PARSE_ERROR;
	}
	request.rq_content_length = request.rq_body.size();
	return PARSE_COMPLETE;
}

/* ----------------------------- Request line ----------------------------- */

/**
 * @brief Parses the request line and stores the method, path, and HTTP version.
 */
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
	else if (method == "GET")
		return true;
	else if (method == "DELETE")
		return true;
	else if (method == "HEAD" || method == "PATCH" || method == "QUERY" || method == "PUT" || method == "CONNECT" || method == "TRACE" || method == "OPTIONS")
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

/* -------------------------------- Headers -------------------------------- */

/**
 * @brief Parses a header line, validates its format, and stores it in the request.
 */
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

	if (request.rq_headers.find(name) != request.rq_headers.end())
		return false;

	request.rq_headers[name] = value;

	if (!request.rq_header.empty())
		request.rq_header.append("\n");
	request.rq_header.append(line);
	return true;
}

/**
 * @brief Validates and extracts the Content-Length header.
 */
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

/* -------------------------------- Chunked -------------------------------- */

/**
 * @brief Parses the next chunk size and updates the parser state.
 */
ParseResult RequestParser::parseChunkSize()
{
	std::string line;
	std::string sizeText;
	std::string::size_type separator;
	std::istringstream stream;
	std::size_t size;

	if (!getNextLine(line))
		return PARSE_INCOMPLETE;

	separator = line.find(';');

	if (separator != std::string::npos)
		sizeText = line.substr(0, separator);
	else
		sizeText = line;

	if (sizeText.empty())
		return PARSE_ERROR;

	for (std::string::size_type i = 0; i < sizeText.size(); i++)
	{
		char c = sizeText[i];

		if (!((c >= '0' && c <= '9')
			|| (c >= 'a' && c <= 'f')
			|| (c >= 'A' && c <= 'F')))
			return PARSE_ERROR;
	}

	stream.str(sizeText);
	stream >> std::hex >> size;

	if (stream.fail())
		return PARSE_ERROR;

	_chunkSize = size;

	if (_chunkSize == 0)
	{
		_state = CHUNK_CRLF;
		return PARSE_INCOMPLETE;
	}

	_state = CHUNK_DATA;

	return PARSE_INCOMPLETE;
}

/**
 * @brief Reads a complete chunk from the buffer and appends it to the request body.
 */
ParseResult RequestParser::parseChunkData(HTTPmessage& request)
{
	if (_buffer.size() < _chunkSize)
		return PARSE_INCOMPLETE;

	request.rq_body += _buffer.substr(0, _chunkSize);
	_buffer.erase(0, _chunkSize);

	_state = CHUNK_CRLF;

	return PARSE_INCOMPLETE;
}

/**
 * @brief Validates and consumes the CRLF following a chunk.
 * @note Marks the request as complete when the terminating chunk is reached.
 */
ParseResult RequestParser::parseChunkCRLF(HTTPmessage& request)
{
	if (_buffer.size() < 2)
		return PARSE_INCOMPLETE;

	if (_buffer[0] != '\r' || _buffer[1] != '\n')
		return PARSE_ERROR;

	_buffer.erase(0, 2);

	if (_chunkSize == 0)
	{
		request.rq_content_length = request.rq_body.size();
		_state = COMPLETE;
		return PARSE_COMPLETE;
	}

	_state = CHUNK_SIZE;

	return PARSE_INCOMPLETE;
}

/* ---------------------------------- Body ---------------------------------- */

/**
 * @brief Extracts the next CRLF-terminated line from the buffer.
 */
bool RequestParser::getNextLine(std::string& line)
{
	std::string::size_type end = _buffer.find("\r\n");

	if (end == std::string::npos)
		return false;

	line = _buffer.substr(0, end);
	_buffer.erase(0, end + 2);

	return true;
}

/**
 * @brief Reads the request body according to Content-Length,
 * updating the received byte count until the body is complete.
 */
ParseResult RequestParser::parseBody(HTTPmessage& request)
{
	std::size_t remaining = _contentLength - _bodyReceived;
	std::size_t toRead = std::min(remaining, _buffer.size());

	request.rq_body += _buffer.substr(0, toRead);

	_bodyReceived += toRead;

	_buffer.erase(0, toRead);

	if (_bodyReceived == _contentLength)
	{
		request.rq_content_length = _contentLength;
		_state = COMPLETE;
		return PARSE_COMPLETE;
	}

	return PARSE_INCOMPLETE;
}
