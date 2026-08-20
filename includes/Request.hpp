#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>

class Request
{
private:
	std::string _method;
	std::string _path;
	std::string _version;
	std::map<std::string, std::string> _headers;
	std::string _body;

public:
	Request();
	~Request();

	const std::string& getMethod() const;
	const std::string& getPath() const;
	const std::string& getVersion() const;

	void setMethod(const std::string& method);
	void setPath(const std::string& path);
	void setVersion(const std::string& version);

	void setHeader(const std::string& name, const std::string& value);
	const std::map<std::string, std::string>& getHeaders() const;

	void setBody(const std::string& body);//content-length
	const std::string& getBody() const;
};

#endif