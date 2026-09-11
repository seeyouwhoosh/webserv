#pragma once
#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>

enum ParseState
{
	REQUEST_LINE,
	HEADERS,
	BODY,
	CHUNK_SIZE,
	CHUNK_DATA,
	CHUNK_CRLF,
	COMPLETE
};

enum ParseResult
{
	PARSE_INCOMPLETE,
	PARSE_COMPLETE,
	PARSE_ERROR
};

class HTTPmessage;

class Parser
{
public:
	Parser();
	~Parser();

	void appendData(const std::string& data); //buffer
	void reset();
	
	ParseResult parse(HTTPmessage& request);

	bool parseRequestLine(const std::string& line, HTTPmessage& request);

private:
	std::string _buffer;
	ParseState _state;
	std::size_t _contentLength;
	std::size_t _bodyReceived;
	std::size_t _chunkSize;

	bool isValidMethod(const std::string& method) const;
	bool isValidVersion(const std::string& version) const;
	bool isValidPath(const std::string& path) const;

	bool parseHeader(const std::string& line, HTTPmessage& request);
	bool parseContentLength(const HTTPmessage& request);
	bool hasChunkedEncoding(const HTTPmessage& request) const;
	bool hasContentLength(const HTTPmessage& request) const;//borrar esto si queremos que ignore el content-length y solo haga chunked
	bool hasHost(const HTTPmessage& request) const;
	bool getNextLine(std::string& line);
	
	ParseResult parseBody(HTTPmessage& request);

	ParseResult parseChunkSize();//leer tamaño del chunk
	ParseResult parseChunkData(HTTPmessage& request);
	ParseResult parseChunkCRLF();
};

#endif