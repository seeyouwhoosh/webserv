#pragma once

#include <unistd.h> /* open, close, read, write, access, execve, fork, pipe, dup2, chdir, opendir */
#include <dirent.h> /* readdir, closedir */
#include <sys/stat.h> /* stat */

#include <cstdio>

#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>

#include "Config.hpp"
#include "WebServer.hpp"

#define ERRPAGE_DIR	"error_pages" //por cambiar

enum	StatusCodes
{
	OK						= 200,
	CREATED					= 201,
	BAD_REQUEST				= 400,
	FORBIDDEN				= 403,
	NOT_FOUND				= 404,
	METHOD_NOT_ALLOWED		= 405,
	UNSUPPORTED_MEDIA_TYPE	= 415,
	INTERNAL_SERVER_ERROR	= 500,
	NOT_IMPLEMENTED			= 501
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
	std::string	query;
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
extern std::vector<std::string>		allowedMethods;

class HTTPmessage
{
private:	
	/* General variables */
	ServerConfig				*serverconf;
	std::vector<std::string>	cgi_env;
	std::string					full_path;
	std::string					server_port;
	size_t						totalSent;
	
	/* Response variables */
	t_rs_startline	rs_startline;
	std::string		rs_header;
	std::string		rs_date;
	std::string		rs_location;		/* Only used if status code is 3XX or 201 */
	std::string		rs_methods_allowed;	/* Only used if status code is 405 */
	std::string		rs_body;			/* Used if it's a GET request or error response */
	size_t			rs_content_length;	/* Only used if there is a body */
	std::string		rs_content_type;	/* Only used if there is a body. Format: type/extension_type */


	/* Helper functions */
	int				matchMethod(std::string &method);
	LocationConfig	*getPathLocation(std::string &file);
	int				generateErrorResponse(int error);
	int				openErrorFile(int error, std::fstream &fs);
	int				openFile(std::string &file, std::fstream &fs, LocationConfig &locationconf);
	int				openFile(std::string &file, std::fstream &fs, ServerConfig &serverconf);
	int				createAutoindex(std::string dir);
	void			setResponse();
	void			fillCGIenv();

	/* HTTP methods */
	int				httpGET();
	int				httpPOST();
	int				httpDELETE();

public:
	/* Orthodox Canonical Form */
	HTTPmessage();
	HTTPmessage(const HTTPmessage &other);
	HTTPmessage(ServerConfig *server);
	HTTPmessage	&operator=(const HTTPmessage &other);
	~HTTPmessage();

	/* Request variables */
	t_rq_startline	rq_startline;
	stringmap		rq_headers;
	std::string		rq_body;
	std::string		rq_host;
	int				rq_keep_alive;
	t_accept		rq_accept;
	std::string		rq_content_type;
	bool			rq_chunked;
	size_t			rq_content_length;
	std::string		rq_user_agent;

	/* Getters and setters */
	const std::string&	getVersion() const;
	int					getStatusCode() const;
	const stringmap&	getHeaders() const;
	const std::string&	getBody() const;
	ServerConfig		*getServerConf() const;
	void				setVersion(const std::string& method);
	int					setStatusCode(const int code);
	void				setHeader(const std::string& name, const std::string& value);
	void				setBody(const std::string& body);
	void				setServerConf(ServerConfig& serverconf);
	void				setServerPort(std::string serverPort);
	std::string			getServerPort();


	/* Main user functions */
	int					generateResponse();
	bool				isComplete() const;
	int					sendResponse(int client);
	void				clear();
};

/* INITs */

void		init_responses();

void		init_MIMEtypes();

void		init_allowedMethods();

/* Miscelaneous helper functions */

std::string	getFileExtension(std::string file);

std::string	getMIMEtype(std::string ext);

int			get_location(std::vector<LocationConfig> &locations, std::string file);
