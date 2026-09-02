

#include "HTTPmessage.hpp"

HTTPmessage::HTTPmessage()
{
	rs_startline.version = "HTTP/1.1";
	totalSent = 0;
	serverconf = NULL;
}

HTTPmessage::HTTPmessage(ServerConfig *server)
{
	rs_startline.version = "HTTP/1.1";
	serverconf = server;
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
	totalSent = 0;
	return *this;
}

/* Get HTTP response version */
const std::string&	HTTPmessage::getVersion() const
{
	return rs_startline.version;
}

/* Get HTTP response status code */
int	HTTPmessage::getStatusCode() const
{
	return rs_startline.status_code;
}

/* Set HTTP response version */
void HTTPmessage::setVersion(const std::string& version)
{
	rs_startline.version = version;
}

/* Set HTTP response status code and phrase */
int HTTPmessage::setStatusCode(const int code)
{
	this->rs_startline.status_code = code;
	this->rs_startline.reason_phrase = responses[code];
	return code;
}

/* Set HTTP response headers */
void HTTPmessage::setHeader(const std::string& name, const std::string& value)
{
	rs_headers[name] = value;
}

/* Get HTTP response headers */
const std::map<std::string, std::string>& HTTPmessage::getHeaders() const
{
	return rs_headers;
}

/* Set HTTP response body */
void HTTPmessage::setBody(const std::string& body)
{
	this->rs_body = body;
}

/* Get HTTP response body */
const std::string& HTTPmessage::getBody() const
{
	return rs_body;
}

/* Set HTTP serverconf */
void	HTTPmessage::setServerConf(ServerConfig& serverconf)
{
	this->serverconf = &serverconf;
}

/* Set HTTP serverconf */
ServerConfig	*HTTPmessage::getServerConf() const
{
	return this->serverconf;
}

/* Set HTTP server port */
void	HTTPmessage::setServerPort(std::string ServerPort)
{
	server_port = ServerPort;
}

/* Get HTTP server port */
std::string	HTTPmessage::getServerPort() const
{
	return server_port;
}

/* Match method given in parameters. If non matches, returns -1, else returns method enum */
int	HTTPmessage::matchMethod(std::string &method)
{
	int	e_methods[] = {
		GET,
		POST,
		DELETE,
		BAD_METHOD,
	};

	for (size_t i = 0; i < allowedMethods.size(); i++)
	{
		if (method == allowedMethods[i])
			return e_methods[i];
	}
	return BAD_METHOD;
}

bool	HTTPmessage::isComplete() const
{
	size_t total_expected = rs_header.length() + rs_body.length();

	return totalSent == total_expected;
}

void	HTTPmessage::clear()
{
	;
}