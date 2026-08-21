#include <cctype>
#include <fstream>

#include "HTTPmessage.hpp"

HTTPmessage::HTTPmessage()
{
	allowedMethods.push_back("GET");
	allowedMethods.push_back("POST");
	allowedMethods.push_back("DELETE");

	rs_startline.version = "HTTP/1.1";

	responses[OK]                    = "OK";
	responses[BAD_REQUEST]           = "Bad Request";
	responses[FORBIDDEN]             = "Forbidden";
	responses[NOT_FOUND]             = "Not Found";
	responses[METHOD_NOT_ALLOWED]    = "Method Not Allowed";
	responses[UNSUPPORTED_MEDIA_TYPE] = "Unsupported Media Type";
	responses[INTERNAL_SERVER_ERROR] = "Internal Server Error";

	mime_types["plain"] = "text/plain";
	mime_types["html"]  = "text/html";
	mime_types["htm"]   = "text/htm";
	mime_types["css"]   = "text/css";
	mime_types["csv"]   = "text/csv";
	mime_types["xml"]   = "text/xml";

	mime_types["png"]   = "image/png";
	mime_types["gif"]   = "image/gif";
	mime_types["jpeg"]  = "image/jpeg";
	mime_types["avif"]  = "image/avif";
	mime_types["jpg"]   = "image/jpg";
	mime_types["svg"]   = "image/svg+xml";

	mime_types["mp4"]   = "video/mp4";
	mime_types["mpeg"]  = "video/mpeg";
	mime_types["webm"]  = "video/webm";

	mime_types["json"]  = "application/json";
	mime_types["pdf"]   = "application/pdf";
	mime_types["rtf"]   = "application/rtf";
	mime_types["zip"]   = "application/zip";

	mime_types["aac"]   = "audio/aac";
	mime_types["midi"]  = "audio/midi";
	mime_types["wav"]   = "audio/wav";

	mime_types["ttf"]   = "font/ttf";
	mime_types["otf"]   = "font/otf";
	mime_types["woff"]  = "font/woff";
	mime_types["woff2"] = "font/woff2";

	mime_types["mjs"]   = "application/javascript";
	mime_types["js"]    = "application/javascript";
	mime_types["md"]    = "text/markdown";
	mime_types["avi"]   = "video/x-msvideo";
	mime_types["bin"]   = "application/octet-stream";
	mime_types["doc"]   = "application/msword";
	mime_types["docx"]  = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	mime_types["epub"]  = "application/epub+zip";
	mime_types["jar"]   = "application/java-archive";
	mime_types["mpkg"]  = "application/vnd.apple.installer+xml";
	mime_types["ogx"]   = "application/ogg";
	mime_types["php"]   = "application/x-httpd-php";
	mime_types["ppt"]   = "application/vnd.ms-powerpoint";
	mime_types["pptx"]  = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	mime_types["rar"]   = "application/vnd.rar";
	mime_types["odp"]   = "application/vnd.oasis.opendocument.presentation";
	mime_types["ods"]   = "application/vnd.oasis.opendocument.spreadsheet";
	mime_types["odt"]   = "application/vnd.oasis.opendocument.text";
	mime_types["sh"]    = "application/x-sh";
	mime_types["tar"]   = "application/x-tar";
	mime_types["xhtml"] = "application/xhtml+xml";
	mime_types["xls"]   = "application/vnd.ms-excel";
	mime_types["xlsx"]  = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
	mime_types["7z"]    = "application/x-7z-compressed";

	mime_types["m4a"]   = "audio/mp4";
	mime_types["mp3"]   = "audio/mpeg";
	mime_types["oga"]   = "audio/ogg";
	mime_types["opus"]  = "audio/ogg";
	mime_types["weba"]  = "audio/webm";
}

HTTPmessage::~HTTPmessage() {}

HTTPmessage::HTTPmessage(const HTTPmessage &other)
{
	*this = other;
}

HTTPmessage	&HTTPmessage::operator=(const HTTPmessage &other)
{
	(void)other;
	return *this;
}

const std::string&	HTTPmessage::getMethod() const
{
	return rs_startline.method;
}

int	HTTPmessage::getStatusCode() const
{
	return rs_startline.status_code;
}

void HTTPmessage::setMethod(const std::string& method)
{
	rs_startline.method = method;
}

void HTTPmessage::setStatusCode(const int code)
{
	this->rs_startline.status_code = code;
	this->rs_startline.reason_phrase = responses[code];
}

void HTTPmessage::setHeader(const std::string& name, const std::string& value)
{
	rs_headers[name] = value;
}

const std::map<std::string, std::string>& HTTPmessage::getHeaders() const
{
	return rs_headers;
}

void HTTPmessage::setBody(const std::string& body)
{
	this->rs_body = body;
}

const std::string& HTTPmessage::getBody() const
{
	return rs_body;
}

std::ostream	&HTTPmessage::write(const char *buf, size_t n)
{
	return clientBuffer.write(buf, n);
}

int	HTTPmessage::matchMethod(std::string &method)
{
	int	e_methods[] = {
		GET,
		POST,
		DELETE,
		HEAD,
		TRACE,
		BAD_METHOD,
	};

	for (size_t i = 0; i < allowedMethods.size(); i++)
	{
		if (method == allowedMethods[i])
			return e_methods[i];
	}
	return BAD_METHOD;
}

static bool	all_whitespace(std::string s)
{
	for (size_t i = 0; i < s.size(); i++)
	{
		if (!std::isblank(s[i]) || s[i] != '\n' || s[i] != '\r')
			return false;
	}
	return true;
}

static int	get_location(std::vector<LocationConfig> locations, std::string file)
{
	std::string	location_name;
	size_t		len;
	size_t		max_len = 0;
	size_t		location_pos = 0;

	for (size_t i = 0; i < locations.size(); i++)
	{
		len = locations[i].name.length();
		if (!file.find(locations[i].name))
		{
			if (len > max_len)
			{
				location_pos = i;
				max_len = len;
			}
		}
	}
	if (!max_len)
		return -1;
	else
		return location_pos;
}

/* If it's a file, see if it's accesible and open it. If it's a directory, see if autoindex is on and search in the indexes list to search for the first available one */
int	HTTPmessage::openFile(std::string file, std::fstream &fs, LocationConfig &locationconf)
{
	struct stat	st;

	if (stat(file.c_str(), &st) < 0)
		return NOT_FOUND;
	else
	{
		if (st.st_mode & S_IFMT == S_IFREG)
		{
			if (access(file.c_str(), F_OK | R_OK) < 0)
				return FORBIDDEN;
			fs.open(file.c_str(), std::fstream::in);
			if (fs.fail() == true)
				return INTERNAL_SERVER_ERROR;
			rs_content_length = st.st_size;
		}
		else if (st.st_mode & S_IFMT == S_IFDIR)
		{
			if (locationconf.autoindex == AUTOINDEX_OFF)
				return NOT_FOUND;
			for (size_t i = 0; i < locationconf.indexes.size(); i++)
			{
				std::string	tmp = file + "/" + locationconf.indexes[i];
				if (access(tmp.c_str(), F_OK | R_OK) < 0)
					continue;
				fs.open(tmp.c_str(), std::fstream::in);
				if (fs.fail() == true)
					return INTERNAL_SERVER_ERROR;
				break;
			}
		}
	}
	return OK;
}

/* Constructs the full path of file or directory into string file and returns the current location */
LocationConfig	&HTTPmessage::getPathfile(std::string &file)
{
	size_t query_pos = file.find('?');
	std::string query; // decide what to do with query after

	if (query_pos != std::string::npos)
	{
		query = file.substr(query_pos + 1);
		file = file.substr(0, query_pos);
	}

	int	location_pos = get_location(serverconf.locations, file);
	LocationConfig	&locationconf = serverconf.locations[location_pos];
	if (location_pos < 0)
		file = serverconf.root + file;
	else
		file = locationconf.root + file;
}

/* Get the file descriptor and fill the gaps for the response accordingly (such as status code, body, content lenght and other needed headers) */
int	HTTPmessage::httpGET()
{
	std::string		file = rq_startline.path;
	std::fstream	fs;

	LocationConfig	&locationconf = getPathfile(file);
	if ((rs_startline.status_code = openFile(file, fs, locationconf)) != OK)
		return -1;

	char	*buff;
	fs.read(buff, rs_content_length);
	if (fs.fail() == true)
	{
		rs_startline.status_code = INTERNAL_SERVER_ERROR;
		return -1;
	}
	rs_body = std::string(buff);
	rs_startline.status_code = OK;
	rs_startline.reason_phrase = responses[OK];
	return 0;
}

int	HTTPmessage::generateResponse()
{
	if (rs_startline.status_code == BAD_REQUEST)
	{
		rs_startline.reason_phrase = std::string(responses[rs_startline.status_code]);
		rs_headers["Content-Type"] = "text/html";
		rs_headers[""] = "";
	}

	std::string	m = rs_startline.method;

	int	e_method = matchMethod(m);
	switch (e_method)
	{
	case GET:
		if (httpGET() < 0)
			break;
	case POST:
		if (httpPOST() < 0)
			break;
	case DELETE:
		if (httpDELETE() < 0)
			break;
	default:
		return BAD_METHOD;
	}
	rs_startline.reason_phrase = std::string(responses[rs_startline.status_code]);
}
