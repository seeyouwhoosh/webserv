#pragma once
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "Config.hpp"
#define ERRPAGE_DIR	"error_pages"

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
}	t_rq_startline;

typedef	struct t_rs_startline
{
	std::string	version;
	int			status_code;
	std::string	reason_phrase;
}	t_rs_startline;

typedef std::map<std::string, std::vector<std::string> > t_accept;
typedef std::map<std::string, std::string> stringmap;

extern stringmap					mime_types;
extern std::map<int, const char *>	responses;

class HTTPmessage
{
private:	
	/* General variables */
	ServerConfig				*serverconf;
	std::stringstream			clientBuffer;
	std::vector<std::string>	allowedMethods;
	// stringmap	cache_control;
	// stringmap	cookies;
	// std::vecto<std::string>	clear_site_data;

	/* Response message */
	t_rs_startline	rs_startline;
	std::string		rs_body;

	/* Response headers */
	std::string		rs_location; // 3XX or 201
	size_t			rs_content_length;
	std::string		rs_date;
	std::string		rs_content_type;
	std::string		rs_methods_allowed; //solo se usa si es 405
	std::string		rs_last_modified;

	int				matchMethod(std::string &method);
	LocationConfig	*getPathfile(std::string &file);
	int				openErrorFile(int error, std::fstream &fs);
	int				openFile(std::string &file, std::fstream &fs, LocationConfig &locationconf);
	int				openFile(std::string &file, std::fstream &fs, ServerConfig &serverconf);

	int				httpGET();

public:
	HTTPmessage();
	HTTPmessage(const HTTPmessage &other);
	HTTPmessage(ServerConfig &server);
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
	
	int				generateResponse();

	const std::string&	getMethod() const;
	int					getStatusCode() const;
	const stringmap&	getHeaders() const;
	const std::string&	getBody() const;
	void	setVersion(const std::string& method);
	void	setStatusCode(const int code);
	void	setHeader(const std::string& name, const std::string& value);
	void	setBody(const std::string& body);
	std::ostream	&write(const char *buf, size_t n);
};

void		init_responses();
void		init_MIMEtypes();
std::string	getFileExtension(std::string file);
std::string	getMIMEtype(std::string ext);