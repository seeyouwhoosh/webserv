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

#define ERRPAGE_DIR	"error_pages"

enum	StatusCodes
{
	OK						= 200,
	CREATED					= 201,
	MOVED_PERMANENTLY		= 301,
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
	std::string		rs_body;			/* Used if it's a GET request or error response */
	size_t			rs_content_length;	/* Only used if there is a body */
	stringmap		rs_headers;

	/* Helper functions */
	int				matchMethod(std::string &method);
	int				openFile(std::string &file, std::fstream &fs, LocationConfig &locationconf);
	int				openFile(std::string &file, std::fstream &fs, ServerConfig &serverconf);
	void			setResponse();
	void			setAllowHeader();
	int				getLocation(std::vector<LocationConfig> &locations, std::string file);

	LocationConfig	*getPathLocation(std::string &file);
	int				generateErrorResponse(int error);
	int				openErrorFile(int error, std::fstream &fs);
	
	void			autoindexParentPath(std::stringstream &page);
	void			autoindexAddElement(std::stringstream &page, std::string dir_path, std::string name);
	void			autoindexHtmlHead(std::stringstream &page, std::string virtual_path);
	int				createAutoindex(std::string dir);
	
	void			fillCGIenv();

	int			manageActuallyDir();

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
	std::string		rq_body;
	size_t			rq_content_length;
	stringmap		rq_headers;

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
	std::string			getServerPort() const;

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

int			getLocation(std::vector<LocationConfig> &locations, std::string file);
