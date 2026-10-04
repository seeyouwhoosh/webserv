#include "RequestParser.hpp"
#include "HTTPmessage.hpp"
#include <iostream>

RequestParser::RequestParser()
	: _state(REQUEST_LINE), _contentLength(0),
	  _bodyReceived(0), _chunkSize(0)
{
}

RequestParser::~RequestParser()
{
}

void RequestParser::clear()
{
	_state = REQUEST_LINE;
	_contentLength = 0;
	_bodyReceived = 0;
	_chunkSize = 0;
}

void RequestParser::appendData(const std::string& data)
{
	_buffer += data;
}

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
				if (!request.rq_header.empty())
					request.rq_header.append("\n");
				request.rq_header.append(line);
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
			ParseResult result = parseChunkCRLF();

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

	return PARSE_COMPLETE;
}