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

/* Searches for error.html in the location corresponding to the current path.
If location exists, and error_file is set, attempts to open file.
Else if error_file in location is not set, or location is not set, it uses global config.
If everything fails, uses the default given by the webserver.
If error_file does not exist in any case or it fails somehow, returns -1 */
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
			fs.open(file.c_str(), std::ios::in | std::ios::binary);
		}
		catch(const std::exception& e)
		{
			return -1;
		}
		rs_content_length = st.st_size;
	}
	return 0;
}

/* Creates an error file */
static long	generateErrorFile(int error, std::string phrase, std::fstream &fs)
{
	std::stringstream	error_str;
	error_str << error;
	std::string			file;
	long				count;

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

	count = fs.gcount();
	fs.close();
	return count;
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
				break; */
		case DELETE:
			if (httpDELETE() < 0)
				break;
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
				setStatusCode(INTERNAL_SERVER_ERROR);
				return -1;
			}
			rs_body.resize(len);
			err_file.read(&rs_body[0], len);
			if (err_file.fail() == true)
			{
				rs_body.clear();
				setStatusCode(INTERNAL_SERVER_ERROR);
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
