#include "HTTPmessage.hpp"
#include "utils.hpp"

/* Orthodox canonical form */
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

/* Getters and setters */

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

/**
 * @brief Shows the response produced
 */
void	HTTPmessage::printAHeaders()
{
	std::cout << "----------------- RESPONSE ----------------\n" << rs_header;
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