#pragma once
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "Config.hpp"

enum	e_status_codes
{
	OK						= 200,
	BAD_REQUEST				= 400,
	FORBIDDEN				= 403,
	NOT_FOUND				= 404,
	METHOD_NOT_ALLOWED		= 405,
	UNSUPPORTED_MEDIA_TYPE	= 415,
	INTERNAL_SERVER_ERROR	= 500
};

std::map<int, const char *>	response = {
	{OK, "OK"},
	{BAD_REQUEST, "Bad Request"},
	{FORBIDDEN, "Forbidden"},
	{NOT_FOUND, "Not Found"},
	{METHOD_NOT_ALLOWED, "Method Not Allowed"},
	{UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type"},
	{INTERNAL_SERVER_ERROR, "Internal Server Error"}
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

typedef t_accept std::map<std::string, std::vector<std::string>>;

typedef	struct t_rq_startline
{
	int			method;
	std::string	target;
	std::string	protocol;
};

typedef	struct t_rs_startline
{
	std::string	protocol;
	int			status_code;
	std::string	reason_phrase;
};

class HTTPmessage
{
private:
	ServerConfig						&serverconf;
	std::stringstream					client_buffer;
	std::vector<std::string>			allowed_methods;
	// std::map<std::string, std::string>	cache_control;
	// std::map<std::string, std::string>	cookies;
	// std::vecto<std::string>				clear_site_data;

	t_rq_startline	rq_startline;
	std::string		rq_header;
	std::string		rq_body;

	t_rs_startline	rs_startline;
	std::string		rs_header;
	std::string		rs_body;

	/* Response headers */
	std::string		rs_location; // 3XX or 201
	size_t			rs_content_length;
	std::string		rs_date;
	std::string		rs_content_type;
	std::string		rs_allowed_methods; //solo se usa si es 405
	std::string		rs_last_modified;

	int				method_match(std::string &s);
	LocationConfig	&get_file_path(std::string &file);
	int				open_file(std::string file, std::fstream &fs, LocationConfig &locationconf);
public:
	HTTPmessage();
	HTTPmessage(const HTTPmessage &other);
	HTTPmessage	&operator=(const HTTPmessage &other);
	~HTTPmessage();

	/* Request headers */
	std::string	rq_host;
	int			rq_keep_alive;
	t_accept	rq_accept;
	std::string	rq_content_type;
	bool		rq_chunked;
	size_t		rq_content_length;
	


	std::ostream	&write(const char *buf, size_t n);
	
	int				request_is_valid();


	int		http_get();

	int	generate_response(std::string &s);
/* 
	http_post();
	http_delete();
	http_head();

	;
	generate_response(); */
};


