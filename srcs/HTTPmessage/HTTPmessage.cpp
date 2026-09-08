

#include "HTTPmessage.hpp"

HTTPmessage::HTTPmessage()
{
	serverconf = NULL;
	server_port = "";
	client_addr = "";
	rs_startline.version = "HTTP/1.1";

	rs_startline.reason_phrase = "";
	rs_startline.status_code = 0;
	full_path = "";
	totalSent = 0;
	rs_header = "";
	rs_body = "";
	rs_content_length = 0;

	rq_startline.file = "";
	rq_startline.iscgi = false;
	rq_startline.method = "";
	rq_startline.path_info = "";
	rq_startline.query = "";
	rq_startline.raw_path = "";
	rq_startline.version = "";
	rq_body = "";
	rq_content_length = 0;
}

HTTPmessage::HTTPmessage(ServerConfig *server)
{
	rs_startline.version = "HTTP/1.1";
	serverconf = server;
	server_port = "";
	client_addr = "";

	rs_startline.reason_phrase = "";
	rs_startline.status_code = 0;
	full_path = "";
	totalSent = 0;
	rs_header = "";
	rs_body = "";
	rs_content_length = 0;

	rq_startline.file = "";
	rq_startline.iscgi = false;
	rq_startline.method = "";
	rq_startline.path_info = "";
	rq_startline.query = "";
	rq_startline.raw_path = "";
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
	totalSent = 0;
	server_port = other.server_port;
	client_addr = other.client_addr;

	rs_startline.reason_phrase = other.rs_startline.reason_phrase;
	rs_startline.status_code = other.rs_startline.status_code;
	full_path = other.full_path;
	totalSent = other.totalSent;
	rs_header = other.rs_header;
	rs_body = other.rs_body;
	rs_content_length = other.rs_content_length;

	rq_startline.file = other.rq_startline.file;
	rq_startline.iscgi = other.rq_startline.iscgi;
	rq_startline.method = other.rq_startline.method;
	rq_startline.path_info = other.rq_startline.path_info;
	rq_startline.query = other.rq_startline.query;
	rq_startline.raw_path = other.rq_startline.raw_path;
	rq_body = other.rq_body;
	rq_content_length = other.rq_content_length;
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

/* Set client address */
void	HTTPmessage::setClientAddress(std::string client_address)
{
	client_addr = client_address;
}

/* Get client address */
std::string	HTTPmessage::getClientAddress() const
{
	return client_addr;
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
	rs_startline.reason_phrase.clear();
	rs_startline.status_code = 0;
	full_path.clear();
	totalSent = 0;
	rs_header.clear();
	rs_body.clear();
	rs_content_length = 0;

	rq_startline.file.clear();
	rq_startline.iscgi = false;
	rq_startline.method.clear();
	rq_startline.path_info.clear();
	rq_startline.query.clear();
	rq_startline.raw_path.clear();
	rq_startline.version.clear();
	rq_body.clear();
	rq_content_length = 0;
}