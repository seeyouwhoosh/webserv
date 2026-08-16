#pragma once
#include <string>

class HTTPmessage
{
private:

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
};


