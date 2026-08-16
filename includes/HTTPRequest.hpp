#pragma once
#include <string>

class HTTPRequest
{
private:

public:
	HTTPRequest();
	~HTTPRequest();

	/* Request headers */
	std::string	host;
	
	/* Response headers */

	http_get();
	http_post();
	http_delete();
	http_head();
};

HTTPRequest::HTTPRequest()
{
}

HTTPRequest::~HTTPRequest()
{
}
