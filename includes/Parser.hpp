#pragma once
#include <string>

#include "HTTPmessage.hpp"

enum ParseState
{
	REQUEST_LINE,
	HEADERS,
	BODY,
	CHUNK_SIZE,
	CHUNK_DATA,
	COMPLETE
};

enum ParseResult
{
	PARSE_INCOMPLETE,
	PARSE_COMPLETE,
	PARSE_ERROR
};

class Parser
{
public:
	Parser();
	Parser(const Parser &other);
	Parser	&operator=(const Parser &other);
	~Parser();

	ParseResult parseRequest(HTTPmessage& request);
	
	private:
	std::string _buffer;
	ParseState _state;
	std::size_t _contentLength;
	std::size_t _bodyReceived;

	bool isValidMethod(const std::string& method) const;
	bool isValidVersion(const std::string& version) const;
	bool isValidPath(const std::string& path) const;
	void appendData(const std::string& data); //buffer
	bool parseRequestLine(const std::string& line, HTTPmessage& request);
	bool parseHeader(const std::string& line, HTTPmessage& request);
	bool hasChunkedEncoding(const HTTPmessage& request) const;
	bool parseContentLength(const HTTPmessage& request);
	bool getNextLine(std::string& line);
	
	bool parseBody(HTTPmessage& request);
};
