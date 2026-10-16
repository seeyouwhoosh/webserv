#include "RequestParser.hpp"
#include "HTTPmessage.hpp"
#include <algorithm>
#include "utils.hpp"

bool RequestParser::getNextLine(std::string& line)
{
	std::string::size_type end = _buffer.find("\r\n");

	if (end == std::string::npos)
		return false;

	line = _buffer.substr(0, end);
	_buffer.erase(0, end + 2);

	return true;
}

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

		/*
		if (!parsedBody.empty())
		{
			request.rq_body = parsedBody;
			request.rq_content_length = parsedBody.size();
		}
		*/

		return PARSE_COMPLETE;
	}

	return PARSE_INCOMPLETE;
}