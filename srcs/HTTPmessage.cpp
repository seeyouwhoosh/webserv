#include "HTTPmessage.hpp"
#include "ConfigParser.hpp"
#include "CGIHandler.hpp"
#include "utils.hpp"

/* ----------------------- Orthodox Canonical Form ----------------------- */

HTTPmessage::HTTPmessage()
{
	serverconf = NULL;
	locationconf = NULL;
	client_fd = 0;
	rs_startline.version = "HTTP/1.1";

	rs_startline.reason_phrase = "";
	rs_startline.status_code = 0;
	full_path = "";
	upload_name = "";
	rs_header = "";
	rs_body = "";
	rs_content_length = 0;

	rq_startline.method = "";
	rq_startline.raw_path = "";
	rq_startline.version = "";
	rq_body = "";
	rq_content_length = 0;
}

HTTPmessage::~HTTPmessage() {}

HTTPmessage::HTTPmessage(const HTTPmessage &other)
{
	*this = other;
}

HTTPmessage	&HTTPmessage::operator=(const HTTPmessage &other)
{
	if (this == &other)
		return *this;
	rs_startline.version = other.rs_startline.version;
	serverconf = other.serverconf;
	client_fd = client_fd;

	rs_startline.reason_phrase = other.rs_startline.reason_phrase;
	rs_startline.status_code = other.rs_startline.status_code;
	full_path = other.full_path;
	upload_name = other.upload_name;
	rs_header = other.rs_header;
	rs_body = other.rs_body;
	rs_content_length = other.rs_content_length;

	rq_startline.method = other.rq_startline.method;
	rq_startline.raw_path = other.rq_startline.raw_path;
	rq_body = other.rq_body;
	rq_content_length = other.rq_content_length;
	return *this;
}

/* ------------------ Getters and setters and booleans ------------------ */

/**
 * @brief Set HTTP response status code and phrase
*/
int HTTPmessage::setStatusCode(const int code)
{
	this->rs_startline.status_code = code;
	this->rs_startline.reason_phrase = responses[code];
	return code;
}

/**
 * @brief Set HTTP response header
*/
void HTTPmessage::setHeader(const std::string& name, const std::string& value)
{
	rs_headers[name] = value;
}

/**
 * @brief Get HTTP response headers
*/
const std::string HTTPmessage::getHeader(const std::string& key) const
{
	stringmap::const_iterator	res = rs_headers.find(key);
	if (res == rs_headers.end())
		return "";
	return res->second;
}

/**
 * @brief Set HTTP response body
*/
void HTTPmessage::setBody(const std::string& body)
{
	this->rs_body = body;
	this->rs_content_length = body.size();
}

/**
 * @brief Set HTTP serverconf
*/
void	HTTPmessage::setServerConf(ServerConfig *serverconf)
{
	this->serverconf = serverconf;
}

/**
 * @brief Set client fd
*/
void	HTTPmessage::setClientfd(int fd)
{
	client_fd = fd;
}

/**
 * @brief Match method given in parameters
 * @return If non matches, returns -1, else returns method enum
*/
int	HTTPmessage::matchMethod(std::string &method)
{
	if (method == "GET")
		return GET;
	else if (method == "POST")
		return POST;
	else if (method == "DELETE")
		return DELETE;
	else
		return BAD_METHOD;
}

ServerConfig	*HTTPmessage::getServerConf() const
{
	return serverconf;
}

/**
 * @brief Shows the request received with headers but no body
 */
void	HTTPmessage::printQHeaders()
{
	std::cout << "\n---------- REQUEST HEADER FROM FD " << client_fd << " ----------\n" <<
				rq_startline.method << " " << rq_startline.raw_path << " " <<
				rq_startline.version << "\n" << rq_header << std::endl;
}

void	HTTPmessage::setCGIHandler(CGIHandler *cgi)
{
	this->cgi = cgi;
}

void	HTTPmessage::setServerPort(std::string port)
{
	server_port = port;
}

/**
 * @brief Shows the response produced
 */
void	HTTPmessage::printAHeaders()
{
	std::cout << "----------------- RESPONSE ----------------\n" << rs_header;
}

/* ------------------------- Main user functions ------------------------- */

/**
 * @brief Creates an error file into the default error pages directory established in the header
 */
static std::string	generateErrorFile(int error, std::string phrase)
{
	std::string	error_str = toString(error);
	std::stringstream	content;

	content << "<!DOCTYPE html>\n<html>\n  <head>\n\t<title>";
	content << error_str << " " << phrase;
	content << "</title>\n  </head>\n  <body>\n\t<h1>";
	content << error_str << " " << phrase; 
	content << "</h1>\n  </body>\n</html>\n";

	return content.str();
}

/**
 * @brief Given a int error, try to open in, if there is no problem, pass
 * it to the response body. Else, generate own error file in the default
 * error pages directory
 */
int	HTTPmessage::generateErrorResponse(int error)
{
	std::fstream	fs;

	if (error == 0)
		return 0;
	if (rs_startline.status_code != error)
		setStatusCode(error);
	if (!openErrorFile(error, fs))
	{
		rs_body.resize(rs_content_length);
		fs.read(&rs_body[0], rs_content_length);
		bool read_failed = fs.fail();

		if (fs.is_open())
			fs.close();

		if (read_failed)
		{
			rs_body.clear();
			return -1;
		}
		return 0;
	}

	rs_body = generateErrorFile(error, rs_startline.reason_phrase);
	rs_content_length = rs_body.size();

	rs_headers["Content-Type"] = "text/html";
	return 0;
}

/**
 * @brief If no problems were detected in the parsing, matches the method
 * and redirects to the correspondent method function.
 * 
 * @note METHOD FUNCTIONS (in case of success) FILL THE NECESSARY RESPONSE
 * VARIABLES, EXCEPT FOR Date, WHICH IS GENERATED AT THE TIME OF INSERTION
 * 
 * @return If method function returns -1, error is set and the error
 * generating function is called
 * 
 * Else if the response is actually a CGI, returns the writing out pipe
 * of the script
 */
int	HTTPmessage::generateResponse()
{
	int	error = 0;
	int	status = 0;

	if (SHOW_LOGS)
		printQHeaders();
	if (rs_startline.status_code == OK)
	{
		std::string	m = rq_startline.method;
		int	e_method = matchMethod(m);
		switch (e_method)
		{
		case GET:
			if ((status = httpGET()) < 0)
				error = rs_startline.status_code;
			break;
		case POST:
			if ((status = httpPOST()) < 0)
				error = rs_startline.status_code;
			break;
		case DELETE:
			if ((status = httpDELETE()) < 0)
				error = rs_startline.status_code;
			break;
		default:
			setAllowHeader();
			error = setStatusCode(NOT_IMPLEMENTED);
			break;
		}
	}
	else
		error = rs_startline.status_code;
	if (error && generateErrorResponse(error) < 0)
		return -1;

	if (status == IS_CGI)
		return IS_CGI;
	return 0;
}

/**
 * @brief Creates a response string
 * @return Return HTTP response
 */
std::string	HTTPmessage::setResponse()
{
	std::stringstream	responseStream;

	responseStream << rs_startline.version << " "
					<< rs_startline.status_code << " "
					<< rs_startline.reason_phrase << "\r\n";

	rs_headers["Date"] = getDate();
	for (stringmap::iterator it = rs_headers.begin(); it != rs_headers.end(); it++)
		responseStream << it->first << ": " << it->second << "\r\n";

	if (rs_startline.status_code != 204 && rs_startline.status_code != 304)
		responseStream << "Content-Length: " << rs_content_length << "\r\n";

	responseStream << "\r\n";

	rs_header = responseStream.str();
	if (SHOW_LOGS)
		printAHeaders();
	return rs_header + rs_body;
}

void	HTTPmessage::clear()
{
	rs_startline.reason_phrase.clear();
	rs_startline.status_code = 0;
	full_path.clear();
	upload_name.clear();
	rs_header.clear();
	rs_body.clear();
	rs_content_length = 0;
	rs_headers.clear();

	rq_startline.method.clear();
	rq_startline.raw_path.clear();
	rq_startline.version.clear();
	rq_body.clear();
	rq_content_length = 0;
	rq_headers.clear();
	rq_header.clear();
}

/* ----------------------------- HTTP methods ----------------------------- */

/**
 * @brief Get the file descriptor and fill the gaps for the response
 * accordingly (such as status code, body, content length and other needed
 * headers)
 */
int HTTPmessage::httpGET()
{
	full_path = rq_startline.raw_path;
	struct stat st;
	LocationConfig *locationconf = getFullPath(full_path);

	if (locationconf && std::find(locationconf->allowed_methods.begin(), locationconf->allowed_methods.end(), "GET") == locationconf->allowed_methods.end())
		return setStatusCode(METHOD_NOT_ALLOWED);

	/* Return directive */
	if (locationconf && (locationconf->return_status.first != 0 || !locationconf->return_status.second.empty()))
	{
		if (locationconf->return_status.first == 0 && !locationconf->return_status.second.empty())
		{
			rs_headers["Location"] = locationconf->return_status.second;
			setStatusCode(FOUND);
		}
		else if (locationconf->return_status.first != 0)
		{
			setStatusCode(locationconf->return_status.first);
			if (!locationconf->return_status.second.empty())
			{
				std::string val = locationconf->return_status.second;
				if (val.find("http://") == 0 || val.find("https://") == 0 || val[0] == '/')
					rs_headers["Location"] = val;
				else 
				{
					rs_headers["Content-Type"] = mime_types["plain"];
					rs_body = val;
					rs_content_length = val.size();
				}
			}
		}
		return 0;
	}

	/* When user forgets to write a '/' */
	if (stat(full_path.c_str(), &st) == 0 &&
		(st.st_mode & S_IFMT) == S_IFDIR &&
		manageActuallyDir())
	{
		setStatusCode(MOVED_PERMANENTLY);
		return 0;
	}

	int status;
	std::fstream fs;

	if (cgi->isCGI())
	{
		status = cgi->execute(*this);
		if (cgi->getPipeOut() > 0)
			return IS_CGI;
	}
	else
	{
		if (!locationconf && !serverconf)
			return -setStatusCode(INTERNAL_SERVER_ERROR);
		status = openFile(full_path, fs, locationconf);
	}

	if (status != OK)
		return -setStatusCode(status);

	if (fs.is_open())
	{
		rs_body.resize(rs_content_length);
		fs.read(&rs_body[0], rs_content_length);
		if (fs.fail())
		{
			rs_body.clear();
			return -setStatusCode(INTERNAL_SERVER_ERROR);
		}
		rs_headers["Content-Type"] = getMIMEtype(getFileExtension(full_path));
	}
	setStatusCode(OK);
	return 0;
}

int	HTTPmessage::httpPOST()
{
	if (rq_body.size() != rq_content_length)
		return -setStatusCode(BAD_REQUEST);

	full_path = rq_startline.raw_path;
	locationconf = getFullPath(full_path);
	
	if (locationconf && std::find(locationconf->allowed_methods.begin(), locationconf->allowed_methods.end(), "POST") == locationconf->allowed_methods.end())
		return setStatusCode(METHOD_NOT_ALLOWED);
	if (rq_content_length > locationconf->max_body_size)
		return -setStatusCode(CONTENT_TOO_LARGE);

	/* Handles cgi */
	int	status;
	if (cgi->isCGI())
	{
		status = cgi->execute(*this);
		if (status == OK && cgi->getPipeIn() > 0 && cgi->getPipeOut() > 0)
			return IS_CGI;
		else
			return -setStatusCode(status);
	}

	/* Handles x-www-form-urlencoded and multipart/form-data */
	if ((status = handleSpecialType()) != 1)
		return status;

	if ((locationconf && locationconf->upload_store.empty()))
		return -setStatusCode(NOT_FOUND);

	if (locationconf->upload_store.empty())
	{
		setStatusCode(OK);
		return 0;
	}

	struct stat	st;
	std::string	upload_store = locationconf->upload_store;

	if (stat(upload_store.c_str(), &st) < 0 || access(upload_store.c_str(), X_OK | W_OK) < 0)
		return -setStatusCode(FORBIDDEN);

	if (upload_name.empty())
	{
		std::string content_type = rq_headers["content-type"];
		size_t dot = full_path.find_last_of('.');
		size_t slash = full_path.find_last_of('/');

		if (dot == std::string::npos || (content_type == getMIMEtype("plain")
				|| content_type == getMIMEtype("")))
		{
			if (slash != std::string::npos)
				upload_name = full_path.substr(slash + 1);
			else
				upload_name = full_path;
		}

		if (upload_name.empty())
			upload_name = "upload_" + toString(time(NULL));
	}
	refactorFile(upload_store, upload_name);
	full_path = upload_name;

	std::fstream	fs;
	fs.open(full_path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);

	if (!fs.is_open())
		return -setStatusCode(FORBIDDEN);

	fs.write(rq_body.c_str(), rq_content_length);

	if (fs.fail())
	{
		fs.close();
		return -setStatusCode(INTERNAL_SERVER_ERROR);
	}
	fs.close();

	setStatusCode(CREATED);
	return 0;
}

int	HTTPmessage::httpDELETE()
{
	full_path = rq_startline.raw_path;
	locationconf = getFullPath(full_path);

	if (locationconf && std::find(locationconf->allowed_methods.begin(), locationconf->allowed_methods.end(), "DELETE") == locationconf->allowed_methods.end())
		return setStatusCode(METHOD_NOT_ALLOWED);
	struct stat	st;
	if (stat(full_path.c_str(), &st) < 0)
		return -setStatusCode(NOT_FOUND);

	if ((st.st_mode & S_IFMT) == S_IFDIR)
		return -setStatusCode(NOT_IMPLEMENTED);

	if (access(full_path.c_str(), F_OK) < 0 || std::remove(full_path.c_str()) != 0)
		return -setStatusCode(FORBIDDEN);

	setStatusCode(NO_CONTENT);
	return 0;
}

/* -------------------------------- Autoindex -------------------------------- */

void	HTTPmessage::autoindexParentPath(std::stringstream &page)
{
	if (rq_startline.raw_path == "/")
		return;

	std::string parent_path = rq_startline.raw_path;
	if (parent_path.length() > 1 && parent_path[parent_path.length() - 1] == '/')
		parent_path.erase(parent_path.length() - 1);

	size_t last_slash = parent_path.find_last_of('/');
	if (last_slash != std::string::npos)
		parent_path = parent_path.substr(0, last_slash + 1);
	else
		parent_path = "/";

	page << "<tr><td><a href=\"" << parent_path << "\">Parent directory/</a></td></tr>";
}

void	HTTPmessage::autoindexAddElement(std::stringstream &page, const std::string &dir_path, std::string &name)
{
	std::string		fullFilePath = dir_path + name;
	char buffer[20];

	struct stat	st;
	if (stat(fullFilePath.c_str(), &st) < 0)
		return;

	bool is_dir = ((st.st_mode & S_IFMT) == S_IFDIR);
	if (is_dir)
		name += "/";

	time_t		mod_time = st.st_mtime;
	struct tm	*timeinfo = std::localtime(&mod_time);
	if (timeinfo)
		std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", timeinfo);
	else
		buffer[0] = '\0';

	page << "<tr><td><a href=\"" << name << "\">" << name << "</a></td>"
			<< "<td>" << buffer << "</td>"
			<< "<td>";

	if (is_dir)
		page << "-";
	else
		page << st.st_size;

	page << "</td></tr>";
}

void	HTTPmessage::autoindexHtmlHead(std::stringstream &page, std::string virtual_path)
{
	page << "<!DOCTYPE html><html><head><title>Index of " << virtual_path << "</title>"
		<< "<style>"
		<< "table { width: 65%; border-collapse: collapse; }"
		<< "th, td { text-align: left; font-family: \"Courier New\", Courier, monospace; padding: 4px 8px; }"
		<< "</style></head><body>"
		<< "<h1>Index of " << virtual_path << "</h1><hr>"
		<< "<table><thead><tr id=\"theader\"><th>Name</th><th>Last modified</th><th>Size</th></tr></thead>"
		<< "<tbody id=\"tbody\">";
}

int HTTPmessage::createAutoindex(std::string dir_path)
{
	std::stringstream	page;
	DIR *dir = opendir(dir_path.c_str());
	if (!dir)
	{
		if (errno == EACCES)
			return FORBIDDEN;
		if (errno == ENOENT)
			return NOT_FOUND;
		return INTERNAL_SERVER_ERROR;
	}

	if (!dir_path.empty() && dir_path[dir_path.length() - 1] != '/')
		dir_path += "/";

	autoindexHtmlHead(page, rq_startline.raw_path);

	struct dirent	*entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;

		if (name == ".")
			continue;
		if (name == "..")
		{
			autoindexParentPath(page);
			continue;
		}
		autoindexAddElement(page, dir_path, name);
	}

	closedir(dir);

	page << "</tbody></table></body></html>";

	rs_body = page.str();
	rs_content_length = rs_body.length();
	rs_headers["Content-Type"] = "text/html";
	return OK;
}

/* ----------------------------- File management ----------------------------- */

/**
 * @brief If it's a file, see if it's accesible and open it. If it's a
 * directory, see if autoindex is on and search in the indexes list to search
 * for the first available one
 */
int	HTTPmessage::openFile(std::string &file, std::fstream &fs, LocationConfig *locationconf)
{
	struct stat							st;
	struct stat							index_st;
	std::vector<std::string>::iterator	it;

	if (stat(file.c_str(), &st) < 0)
		return NOT_FOUND;

	if ((st.st_mode & S_IFMT) == S_IFREG)
	{
		if (access(file.c_str(), F_OK | R_OK) < 0)
			return FORBIDDEN;
		fs.open(file.c_str(), std::ios::in | std::ios::binary);
		if (!fs.is_open())
			return INTERNAL_SERVER_ERROR;
		rs_content_length = st.st_size;
		return OK;
	}
	else if ((st.st_mode & S_IFMT) == S_IFDIR)
	{
		HTTPContext	*config = !locationconf ? static_cast<HTTPContext *>(serverconf) : static_cast<HTTPContext *>(locationconf);
		for (it = config->indexes.begin(); it != config->indexes.end(); it++)
		{
			if (it->empty())
				continue;
			std::string	tmp = *it;
			refactorFile(file, tmp);

			if (stat(tmp.c_str(), &index_st) == 0 && (index_st.st_mode & S_IFMT) == S_IFREG)
			{
				if (access(tmp.c_str(), R_OK) < 0)
					return FORBIDDEN;
				fs.open(tmp.c_str(), std::ios::in | std::ios::binary);
				if (!fs.is_open())
					return INTERNAL_SERVER_ERROR;
				rs_content_length = index_st.st_size;

				file = tmp;
				return OK;
			}
		}
		if (config->autoindex == AUTOINDEX_ON)
			return createAutoindex(file);
		return FORBIDDEN;
	}
	return NOT_FOUND;
}

/* Constructs the full path of file or directory into string file and returns the current location */
LocationConfig *HTTPmessage::getFullPath(std::string &full_path)
{
	if (serverconf == NULL)
		return NULL;

	cgi->separateQuery(full_path);
	request_uri = full_path;
	int location_pos = getLocationIndex(serverconf->locations, full_path);

	if (location_pos < 0)
	{
		std::string root = serverconf->root;
		refactorFile(root, full_path);
		cgi->parseFiletoCGI(full_path, locationconf);
		return NULL;
	}

	locationconf = &serverconf->locations[location_pos];
	std::string		root;
	if (!locationconf->alias.empty())
	{
		root = locationconf->alias;
		full_path.erase(0, locationconf->name.length());
	}
	else
		root = locationconf->root.empty() ? serverconf->root : locationconf->root;
	refactorFile(root, full_path);

	cgi->parseFiletoCGI(full_path, locationconf);

	return locationconf;
}

/**
 * @brief When the GET path is like "/thing" meaning to grab a file but it's actually a directory
 * @return 0 if it's not a directory, true if it is */
bool	HTTPmessage::manageActuallyDir()
{
	if (full_path.empty() || full_path[full_path.length() - 1] == '/')
		return false;

	std::string redirect_url = request_uri + "/";
	
	if (!cgi->getQuery().empty())
		redirect_url += "?" + cgi->getQuery();

	rs_headers["Location"] = redirect_url;

	rs_body = "<html><body><h1>301 Moved Permanently</h1></body></html>\n";
	rs_content_length = rs_body.length();
	rs_headers["Content-Type"] = "text/html";

	return true;
}

/**
 * @brief Searches for error.html in the location corresponding to the current path.
 * 
 * If location exists, and error_file is set, attempts to open file.
 * 
 * Else if error_file in location is not set, or location is not set, it uses global config.
 * 
 * If everything fails, uses the default given by the webserver.
 * 
 * @return If error_file does not exist in any case or it fails somehow, returns -1
 */
int	HTTPmessage::openErrorFile(int error, std::fstream &fs)
{
	std::string	error_str = toString(error);
	std::string	err_file;

	if (locationconf)
	{
		std::map<int, std::string>::iterator it = locationconf->error_pages.find(error);
		if (it != locationconf->error_pages.end())
			err_file = it->second;
	}
	else
	{
		std::map<int, std::string>::iterator it = serverconf->error_pages.find(error);
		if (it != locationconf->error_pages.end())
			err_file = it->second;
	}
	if (err_file.empty())
	{
		err_file = error_str + ".html";
		err_file = "/" + err_file;
		err_file = ERRPAGE_DIR + err_file;
	}

	refactorFile(serverconf->root, err_file);

	struct stat	st;
	if (stat(err_file.c_str(), &st) < 0)
		return -1;
	if ((st.st_mode & S_IFMT) != S_IFREG)
		return -1;
	if (access(err_file.c_str(), F_OK | R_OK) < 0)
		return -1;

	fs.open(err_file.c_str(), std::ios::in | std::ios::binary);
	if (!fs.is_open() || fs.fail())
		return -1;
	if (isFileEmpty(fs))
	{
		fs.close();
		return -1;
	}
	rs_content_length = st.st_size;
	rs_headers["Content-Type"] = getMIMEtype(getFileExtension(err_file));
	return 0;
}

void	HTTPmessage::setAllowHeader()
{
	std::stringstream	header;

	full_path = rq_startline.raw_path;
	locationconf = getFullPath(full_path);

	for (std::vector<std::string>::const_iterator it = 
		locationconf->allowed_methods.begin();
		 it != locationconf->allowed_methods.end(); ++it)
	{
		if (it != locationconf->allowed_methods.begin())
			header << ", ";
		header << *it;
	}

	rs_headers["Allow"] = header.str();
}

/* ------------------------------- POST helpers ------------------------------- */

void	HTTPmessage::processURLencoded()
{
	std::stringstream	page;
	std::stringstream	ss;
	ss << rq_body;

	page << "<!DOCTYPE html><html><head><title>Received form answers</title>"
		<< "<style>"
		<< "table { width: 65%; border-collapse: collapse; }"
		<< "th, td { text-align: left; font-family: \"Courier New\", Courier, monospace; padding: 4px 8px; }"
		<< "</style></head><body>"
		<< "<h1>Received form answers</h1><hr>"
		<< "<table><thead><tr id=\"theader\"><th>Field</th><th>Value</th></tr></thead>"
		<< "<tbody id=\"tbody\">";

	std::string	pair;
	while (std::getline(ss, pair, '&'))
	{
		urlDecode(pair);
		size_t	separator = pair.find('=');

		if (separator == std::string::npos)
		{
			page << "<tr><td>" << pair << "</td><td></td>";
			continue;
		}

		page << "<tr><td>" << pair.substr(0, separator) << "</td>"
			<< "<td>" << pair.substr(separator + 1) << "</td>";
	}
	page << "</tbody></table></body></html>";
	rs_body = page.str();
	rs_content_length = rs_body.length();
	rs_headers["Content-Type"] = "text/html";
}

int HTTPmessage::parseMultipartToBody(const std::string& contentHeader)
{
	size_t bPos = contentHeader.find("boundary=");
	if (bPos == std::string::npos)
		return 0;

	bPos += 9;
	size_t bEnd = contentHeader.find(';', bPos);

	std::string boundary = contentHeader.substr(
		bPos, bEnd == std::string::npos ? std::string::npos : bEnd - bPos
	);

	size_t first = boundary.find_first_not_of(" \t");
	if (first == std::string::npos)
		return -1;

	size_t last = boundary.find_last_not_of(" \t");
	boundary = boundary.substr(first, last - first + 1);

	if (boundary.size() >= 2 &&
		boundary[0] == '"' &&
		boundary[boundary.size() - 1] == '"')
	{
		boundary = boundary.substr(1, boundary.size() - 2);
	}

	if (boundary.empty())
		return -1;

	boundary.insert(0, "--");

	std::string body_copy;
	size_t pos = 0;

	while ((pos = rq_body.find(boundary, pos)) != std::string::npos)
	{
		size_t after_boundary = pos + boundary.size();

		if (rq_body.compare(after_boundary, 2, "--") == 0)
			break;

		if (rq_body.compare(after_boundary, 2, "\r\n") != 0)
		{
			pos = after_boundary;
			continue;
		}

		pos = after_boundary + 2;

		size_t header_end = rq_body.find("\r\n\r\n", pos);
		size_t sep_len = 4;

		if (header_end == std::string::npos)
		{
			header_end = rq_body.find("\n\n", pos);
			sep_len = 2;

			if (header_end == std::string::npos)
				return -1;
		}

		size_t filename_pos;
		if ((filename_pos = rq_body.find("filename=", pos))
				!= std::string::npos &&
			filename_pos < header_end)
		{
			filename_pos += 9;

			if (filename_pos >= header_end)
				return -1;

			if (rq_body[filename_pos] != '"')
				return -1;

			size_t filename_end = rq_body.find('"', filename_pos + 1);

			if (filename_end == std::string::npos ||
				filename_end >= header_end)
				return -1;

			upload_name.assign(
				rq_body, filename_pos + 1,
				filename_end - filename_pos - 1
			);
		}

		size_t content_start = header_end + sep_len;
		size_t content_end = rq_body.find(boundary, content_start);

		if (content_end == std::string::npos)
			return -1;

		size_t bodypart_end = content_end;

		if (bodypart_end >= 2 &&
			rq_body.compare(bodypart_end - 2, 2, "\r\n") == 0)
			bodypart_end -= 2;
		else if (bodypart_end >= 1 &&
				 rq_body[bodypart_end - 1] == '\n')
			--bodypart_end;

		body_copy.append(
			rq_body, content_start, bodypart_end - content_start
		);

		pos = content_end;
	}

	if (body_copy.empty())
		return -1;

	rq_body.swap(body_copy);
	rq_content_length = rq_body.size();

	return 1;
}

/**
 * @brief Handles Multipart and x-www-form-urlencoded
 * @return Returns 0 if all is done, and just return. Returns -1 if there
 * was an error. Returns 1 if there is still need to process things
 */
int	HTTPmessage::handleSpecialType()
{
	stringmap::iterator it = rq_headers.find("content-type");
	if (it == rq_headers.end())
		return 1;

	if (it->second.find("multipart/form-data") != std::string::npos)
	{
		if (parseMultipartToBody(it->second) < 0)
			return setStatusCode(BAD_REQUEST);
	}
	else if (it->second.find("application/x-www-form-urlencoded") != std::string::npos)
	{
		processURLencoded();
		setStatusCode(OK);
		return 0;
	}

	return 1;
}
