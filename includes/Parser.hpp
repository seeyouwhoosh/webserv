#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include "Request.hpp"

enum ParseState
{
	REQUEST_LINE,
	HEADERS,
	BODY,
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
	~Parser();

	void appendData(const std::string& data); //buffer
	ParseResult parse(Request& request);

	bool parseRequestLine(const std::string& line, Request& request);

private:
	std::string _buffer;
	ParseState _state;
	std::size_t _contentLength;
	std::size_t _bodyReceived;

	bool isValidMethod(const std::string& method) const;
	bool isValidVersion(const std::string& version) const;
	bool isValidPath(const std::string& path) const;

	bool parseHeader(const std::string& line, Request& request);
	bool hasCompleteLine() const;
	bool parseContentLength(const Request& request);
	bool getNextLine(std::string& line);
	
	bool parseBody(Request& request);
};

#endif