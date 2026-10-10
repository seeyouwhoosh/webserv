#pragma once

#include <unistd.h> /* open, close, read, write, access, execve, fork, pipe, dup2, chdir, opendir */
#include <dirent.h> /* readdir, closedir */
#include <sys/stat.h> /* stat */
#include <sys/wait.h> /* wait */
#include <sys/socket.h>

#include <cstdio>
#include <cctype>
#include <ctime>

#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>

class ServerConfig;
class LocationConfig;

#define ERRPAGE_DIR	"error_pages"
#ifndef BUFFER_SIZE
# define BUFFER_SIZE 1024
#endif

enum	StatusCodes
{
	OK						= 200,
	CREATED					= 201,
	NO_CONTENT				= 204,
	MOVED_PERMANENTLY		= 301,
	FOUND					= 302,
	BAD_REQUEST				= 400,
	FORBIDDEN				= 403,
	NOT_FOUND				= 404,
	METHOD_NOT_ALLOWED		= 405,
	CONTENT_TOO_LARGE		= 413,
	UNSUPPORTED_MEDIA_TYPE	= 415,
	UNPROCESSABLE_CONTENT	= 422,
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
	std::string	raw_path;
	std::string	version;
}	t_rq_startline;

typedef	struct t_rs_startline
{
	std::string	version;
	int			status_code;
	std::string	reason_phrase;
}	t_rs_startline;

typedef std::map<std::string, std::string> stringmap;

extern stringmap					mime_types;
extern std::map<int, const char *>	responses;

class HTTPmessage
{
private:	
	/* General variables */
	CGIHandler		*cgi;
	ServerConfig	*serverconf;
	LocationConfig	*locationconf;
	std::string		full_path;
	std::string		upload_name;
	
	std::string		cgi_path; //
	std::string		server_port; //
	std::string		client_addr; //
	int				client_fd; // For the logs
	
	/* Response variables */
	t_rs_startline	rs_startline;
	std::string		rs_header;
	std::string		rs_body;
	size_t			rs_content_length;
	stringmap		rs_headers;

	/* HTTP methods */
	int				httpGET();
	int				httpPOST();
	int				httpDELETE();

	/* Autoindex */
	void			autoindexParentPath(std::stringstream &page);
	void			autoindexAddElement(std::stringstream &page,
										const std::string &dir_path,
										std::string &name);
	void			autoindexHtmlHead(std::stringstream &page,
										std::string virtual_path);
	int				createAutoindex(std::string dir);

	/* File management */
	int				openFile(std::string &file, std::fstream &fs, LocationConfig &locationconf);
	int				openFile(std::string &file, std::fstream &fs, ServerConfig &serverconf);
	LocationConfig	*getFullPath(std::string &file);
	int				openErrorFile(int error, std::fstream &fs);
	int				generateErrorResponse(int error);
	bool			manageActuallyDir();

	/* Other */
	int				handleSpecialType();
	int				parseMultipartToBody(const std::string &contentHeader);
	void			processURLencoded();
	int				matchMethod(std::string &method);
	void			setAllowHeader();
	void			printQHeaders();
	void			printAHeaders();

public:
	/* Orthodox Canonical Form */
	HTTPmessage();
	HTTPmessage(const HTTPmessage &other);
	HTTPmessage	&operator=(const HTTPmessage &other);
	~HTTPmessage();

	/* Request variables */
	t_rq_startline	rq_startline;
	std::string		rq_body;
	std::string		rq_header;
	size_t			rq_content_length;
	stringmap		rq_headers;

	/* Getters and setters */
	int					setStatusCode(const int code);
	void				setHeader(const std::string& name, const std::string& value);
	std::string			getClientAddress() const;
	const std::string	getHeader(const std::string& key) const;
	void				setBody(const std::string& body);
	void				setServerConf(ServerConfig *serverconf);
	ServerConfig		*getServerConf() const;
	void				setServerPort(std::string serverPort);
	void				setClientAddress(std::string client_address);
	void				setClientfd(int fd);

	/* Main user functions */
	int					generateResponse();
	std::string			setResponse();
	void				clear();
};
