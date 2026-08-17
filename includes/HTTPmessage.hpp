#pragma once
#include <string>
#include <sstream>
#include <map>

enum	e_status_codes
{
	BAD_REQUEST = 400,
	METHOD_NOT_ALLOWED = 405,
};

std::map<e_status_codes, const char *>	response = {
	{BAD_REQUEST, "Bad Request"},
	{METHOD_NOT_ALLOWED, "Method Not Allowed"},
};

enum	e_request_method
{
	BAD_METHOD = -1,
	GET,
	POST,
	DELETE,
	HEAD,
	TRACE
};

typedef	struct request_startline_t
{
	int			method;
	std::string	target;
	std::string	protocol;
};

typedef	struct response_startline_t
{
	std::string	protocol;
	int			status_code;
	std::string	reason_phrase;
};

class HTTPmessage
{
private:
	std::stringstream		client_buffer;
	request_startline_t		request_startline;
	std::string				request_header;
	std::string				request_body;

	response_startline_t	response_startline;
	std::string				response_header;
	std::string				response_body;

	bool	method_match(std::string &s);

public:
	HTTPmessage();
	HTTPmessage(const HTTPmessage &other);
	HTTPmessage	&operator=(const HTTPmessage &other);
	~HTTPmessage();

	/* Request headers */
	std::string		q_host;
	int				q_keep_alive;
	std::string		q_content_type;
	size_t			q_content_length;
	
	/* Response headers */
	std::string		a_location;
	size_t			a_content_length;

	std::ostream	&write(const char *buf, size_t n);
	
	int				request_is_valid();
/* 	http_get();
	http_post();
	http_delete();
	http_head();

	;
	generate_response(); */
};


