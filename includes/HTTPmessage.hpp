#pragma once
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "Config.hpp"

enum	StatusCodes
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

enum	RequestMethod
{
	BAD_METHOD = -1,
	GET,
	POST,
	DELETE,
	HEAD,
	TRACE
};

typedef	struct t_rq_startline
{
	std::string	method;
	std::string	path;
	std::string	version;
};

typedef	struct t_rs_startline
{
	std::string	method;
	int			status_code;
	std::string	reason_phrase;
};

typedef std::map<std::string, std::vector<std::string>> t_accept;
typedef	std::map<std::string, std::string> stringmap;

class HTTPmessage
{
private:

	/* General variables */
	ServerConfig				&serverconf;
	std::stringstream			clientBuffer;
	std::vector<std::string>	allowedMethods;
	// stringmap	cache_control;
	// stringmap	cookies;
	// std::vecto<std::string>	clear_site_data;

	/* Response message */
	t_rs_startline	rs_startline;
	stringmap		rs_headers;
	std::string		rs_body;

	/* Response headers */
	std::string		rs_location; // 3XX or 201
	size_t			rs_content_length;
	std::string		rs_date;
	std::string		rs_content_type;
	std::string		rs_methods_allowed; //solo se usa si es 405
	std::string		rs_last_modified;

	int				matchMethod(std::string &method);
	LocationConfig	&getPathfile(std::string &file);
	int				openFile(std::string file, std::fstream &fs, LocationConfig &locationconf);

	public:
	HTTPmessage();
	HTTPmessage(const HTTPmessage &other);
	HTTPmessage	&operator=(const HTTPmessage &other);
	~HTTPmessage();

	/* Request message */
	t_rq_startline	rq_startline;
	stringmap		rq_headers;
	std::string		rq_body;

	/* Request headers */
	std::string	rq_host;
	int			rq_keep_alive;
	t_accept	rq_accept;
	std::string	rq_content_type;
	bool		rq_chunked;
	size_t		rq_content_length;
	

	std::ostream	&write(const char *buf, size_t n);
	int				httpGET();
	int				generateResponse(std::string &m);

/* 
	http_post();
	http_delete();
	http_head();
*/
	const std::string&	getMethod() const;
	int					getStatusCode() const;
	const stringmap&	getHeaders() const;
	const std::string&	getBody() const;

	void	setMethod(const std::string& method);
	void	setStatusCode(const int code);
	void	setHeader(const std::string& name, const std::string& value);
	void	setBody(const std::string& body); //content-length
};


