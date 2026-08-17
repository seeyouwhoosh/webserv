#pragma once
#include <string>

class HTTPmessage
{
private:
	std::string	request_startline;
	std::string	request_header;
	std::string	request_body;
	std::string	response_startline;
	std::string	response_header;
	std::string	response_body;

public:
	HTTPmessage();
	HTTPmessage(HTTPmessage &other);
	HTTPmessage	&operator=(HTTPmessage &other);
	~HTTPmessage();

	/* Request headers */
	std::string	host;
	int			keep_alive;
	
	/* Response headers */
	int			status_code;

	http_get();
	http_post();
	http_delete();
	http_head();

	request_is_valid();
	generate_response();
};


