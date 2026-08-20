#include "Request.hpp"

Request::Request()
{
}

Request::~Request()
{
}

const std::string& Request::getMethod() const
{
	return _method;
}

const std::string& Request::getPath() const
{
	return _path;
}

const std::string& Request::getVersion() const
{
	return _version;
}

void Request::setMethod(const std::string& method)
{
	_method = method;
}

void Request::setPath(const std::string& path)
{
	_path = path;
}

void Request::setVersion(const std::string& version)
{
	_version = version;
}

void Request::setHeader(const std::string& name, const std::string& value)
{
	_headers[name] = value;
}

const std::map<std::string, std::string>& Request::getHeaders() const
{
	return _headers;
}

void Request::setBody(const std::string& body)
{
	_body = body;
}

const std::string& Request::getBody() const
{
	return _body;
}