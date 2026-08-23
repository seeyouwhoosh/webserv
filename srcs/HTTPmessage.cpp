#include <cctype>
#include <fstream>

#include "HTTPmessage.hpp"

HTTPmessage::HTTPmessage()
{
	allowedMethods.push_back("GET");
	allowedMethods.push_back("POST");
	allowedMethods.push_back("DELETE");

	rs_startline.version = "HTTP/1.1";
}

HTTPmessage::HTTPmessage(ServerConfig &server)
{
	HTTPmessage();
	serverconf = &server;
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
	return rs_startline.version;
}

int	HTTPmessage::getStatusCode() const
{
	return rs_startline.status_code;
}

void HTTPmessage::setVersion(const std::string& version)
{
	rs_startline.version = version;
}

void HTTPmessage::setStatusCode(const int code)
{
	this->rs_startline.status_code = code;
	this->rs_startline.reason_phrase = responses[code];
}

void HTTPmessage::setHeader(const std::string& name, const std::string& value)
{
	rq_headers[name] = value;
}

const std::map<std::string, std::string>& HTTPmessage::getHeaders() const
{
	return rq_headers;
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

/* Returns location index if it exist, it not, returns -1 */
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
int	HTTPmessage::openFile(std::string &file, std::fstream &fs, LocationConfig &locationconf)
{
	struct stat	st;

	if (stat(file.c_str(), &st) < 0)
		return NOT_FOUND;
	else
	{
		if ((st.st_mode & S_IFMT) == S_IFREG)
		{
			if (access(file.c_str(), F_OK | R_OK) < 0)
				return FORBIDDEN;
			fs.open(file.c_str(), std::fstream::in);
			if (fs.fail() == true)
				return INTERNAL_SERVER_ERROR;
			rs_content_length = st.st_size;
		}
		else if ((st.st_mode & S_IFMT) == S_IFDIR)
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
				file = tmp;
				break;
			}
		}
	}
	return OK;
}

/* If it's a file, see if it's accesible and open it. If it's a directory, see if autoindex is on and search in the indexes list to search for the first available one */
int	HTTPmessage::openFile(std::string &file, std::fstream &fs, ServerConfig &serverconf)
{
	struct stat	st;

	if (stat(file.c_str(), &st) < 0)
		return NOT_FOUND;
	else
	{
		if ((st.st_mode & S_IFMT) == S_IFREG)
		{
			if (access(file.c_str(), F_OK | R_OK) < 0)
				return FORBIDDEN;
			fs.open(file.c_str(), std::fstream::in);
			if (fs.fail() == true)
				return INTERNAL_SERVER_ERROR;
			rs_content_length = st.st_size;
		}
		else if ((st.st_mode & S_IFMT) == S_IFDIR)
		{
			if (serverconf.autoindex == AUTOINDEX_OFF)
				return NOT_FOUND;
			for (size_t i = 0; i < serverconf.indexes.size(); i++)
			{
				std::string	tmp = file + "/" + serverconf.indexes[i];
				if (access(tmp.c_str(), F_OK | R_OK) < 0)
					continue;
				fs.open(tmp.c_str(), std::fstream::in);
				if (fs.fail() == true)
					return INTERNAL_SERVER_ERROR;
				file = tmp;
				break;
			}
		}
	}
	return OK;
}

/* Constructs the full path of file or directory into string file and returns the current location */
LocationConfig	*HTTPmessage::getPathfile(std::string &file)
{
	size_t query_pos = file.find('?');
	std::string query; // decide what to do with query after

	if (query_pos != std::string::npos)
	{
		query = file.substr(query_pos + 1);
		file = file.substr(0, query_pos);
	}

	int	location_pos = get_location(serverconf->locations, file);
	if (location_pos < 0)
	{
		file = serverconf->root + file;
		return NULL;
	}
	else
	{
		LocationConfig	&locationconf = serverconf->locations[location_pos];
		file = locationconf.root + file;
		return &locationconf;
	}
}

/* Get the file descriptor and fill the gaps for the response accordingly (such as status code, body, content lenght and other needed headers) */
int	HTTPmessage::httpGET()
{
	std::string		file = rq_startline.path;
	std::fstream	fs;
	LocationConfig	*locationconf = getPathfile(file);

	if (!locationconf)
	{
		if ((rs_startline.status_code = openFile(file, fs, *serverconf)) != OK)
			return -1;
	}
	else
	{
		if ((rs_startline.status_code = openFile(file, fs, *locationconf)) != OK)
			return -1;
	}

	rs_body.resize(rs_content_length);
	fs.read(&rs_body[0], rs_content_length);
	if (fs.fail() == true)
	{
		rs_body.clear();
		rs_startline.status_code = INTERNAL_SERVER_ERROR;
		return -1;
	}
	rs_startline.status_code = OK;
	rs_startline.reason_phrase = responses[OK];
	rs_content_type = getMIMEtype(getFileExtension(file));
	return 0;
}

int	HTTPmessage::openErrorFile(int error, std::fstream &fs)
{
	std::vector<LocationConfig>	locations = serverconf->locations;
	bool				use_default = false;
	std::stringstream	error_str;
	error_str << error;
	std::string			file = error_str.str();
	file.append(".html");

	int					len, max_len, location_pos = 0;
	size_t				i = 0;
	std::string			path = rq_startline.path;
	for (; i < locations.size(); i++)
	{
		len = locations[i].name.length();
		if (!path.find(locations[i].name))
		{
			if (len > max_len)
			{
				location_pos = i;
				max_len = len;
			}
		}
	}
	if (max_len > 0)
	{
		try
		{
			file = locations[location_pos].root + locations[location_pos].error_pages.at(error);
		}
		catch (const std::exception& e)
		{
			max_len = 0;
		}
	}
	if (!max_len)
	{
		try
		{
			file = serverconf->root + serverconf->error_pages.at(error);
		}
		catch (const std::exception& e)
		{
			use_default = true;
		}
	}

	if (use_default)
		file = std::string(ERRPAGE_DIR) + "/" + error_str.str();

	struct stat	st;
	if (stat(file.c_str(), &st) < 0)
		return -1;
	else
	{
		if ((st.st_mode & S_IFMT) != S_IFREG)
			return -1;
		if (access(file.c_str(), F_OK | R_OK) < 0)
			return -1;
		try
		{
			fs.open(file.c_str(), std::fstream::in);
		}
		catch(const std::exception& e)
		{
			return -1;
		}
		rs_content_length = st.st_size;
	}
	return 0;
}
static long	generateErrorFile(int error, std::string phrase, std::fstream &fs)
{
	std::stringstream	error_str;
	error_str << error;
	std::string			file;
	file = ERRPAGE_DIR + error_str.str() + ".html";
	try
	{
		fs.open(file.c_str(), std::fstream::out | std::fstream::in);
	}
	catch(const std::exception& e)
	{
		return -1;
	}
	fs << "<!DOCTYPE html>\n<html>\n  <head>\n    <title>";
	fs << error_str << " " << phrase;
	fs << "\n/title>\n  </head>\n  <body>\n    <h1>";
	fs << error_str << " " << phrase; 
	fs << "</h1>\n  </body>\n</html>";

	return fs.gcount();
}


int	HTTPmessage::generateResponse()
{
	int	error = 0;

	if (rs_startline.status_code != BAD_REQUEST)
	{
		std::string	m = rq_startline.method;
		int	e_method = matchMethod(m);
		switch (e_method)
		{
		case GET:
			if (httpGET() < 0)
				error = rs_startline.status_code;
			break;
	/* 	case POST:
			if (httpPOST() < 0)
				break;
		case DELETE:
			if (httpDELETE() < 0)
				break; */
		default:
			break;
		}
	}
	if (error)
	{
		std::fstream	err_file;
		if (openErrorFile(error, err_file) < 0)
		{
			long	len = generateErrorFile(error, rs_startline.reason_phrase, err_file);
			if (len < 0)
			{
				rs_startline.status_code = INTERNAL_SERVER_ERROR;
				return -1;
			}
			rs_body.resize(len);
			err_file.read(&rs_body[0], len);
			if (err_file.fail() == true)
			{
				rs_body.clear();
				rs_startline.status_code = INTERNAL_SERVER_ERROR;
				return -1;
			}
			rs_content_length = len;
		}
	}

	rs_startline.reason_phrase = std::string(responses[rs_startline.status_code]);
	std::stringstream	response_buff;
	response_buff << rs_startline.version << " ";
	response_buff << rs_startline.status_code << " " << rs_startline.reason_phrase << "\n";

	response_buff << "Date: " << rs_date << "\n";
	if (rs_startline.status_code == 201 || (rs_startline.status_code / 100) == 3)
		response_buff << "Location: " << rs_location << "\n";
	if (!rs_body.empty())
	{
		response_buff << "Content-Type: " << rs_content_type << "\n";
		response_buff << "Content-Length: " << rs_content_length << "\n";
	}
	if (rs_startline.status_code == 405)
	{
		response_buff << "Allow: " << rs_content_type << "\n";
		for (size_t i = 0; i < allowedMethods.size(); i++)
		{
			response_buff << allowedMethods[i];
			if (i != allowedMethods.size() - 1)
				response_buff << ", ";
			else
				response_buff << "\n";
		}
	}
	return 0;
}
