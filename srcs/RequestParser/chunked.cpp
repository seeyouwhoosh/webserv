#include "RequestParser.hpp"
#include "HTTPmessage.hpp"
#include <sstream>

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

ParseResult RequestParser::parseChunkData(HTTPmessage& request)
{
	if (_buffer.size() < _chunkSize)
		return PARSE_INCOMPLETE;

	request.rq_body += _buffer.substr(0, _chunkSize);
	_buffer.erase(0, _chunkSize);

	_state = CHUNK_CRLF;

	return PARSE_INCOMPLETE;
}

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