#include "HTTPmessage.hpp"
#include <cctype>

HTTPmessage::HTTPmessage() {}

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

std::ostream	&HTTPmessage::write(const char *buf, size_t n)
{
	return client_buffer.write(buf, n);
}

int	method_match(std::string &s)
{
	std::string	methods[] = {
		"GET",
		"POST",
		"DELETE",
		"HEAD",
		"TRACE",
		""
	};

	int	e_methods[] = {
		GET,
		POST,
		DELETE,
		HEAD,
		TRACE,
		BAD_METHOD,
	};

	for (size_t i = 0; methods[i] != ""; i++)
	{
		if (s == methods[i])
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

/* see if request is valid, if it is, returns 0, else return -1 and sets status_code */
int	HTTPmessage::request_is_valid()
{
	std::string			s;
	request_startline_t	r;

	if (all_whitespace(client_buffer.str()))
	{
		response_startline.status_code = BAD_REQUEST;
		return -1;
	}

	client_buffer >> s;
	if ((request_startline.method = method_match(s)) == BAD_METHOD)
	{
		response_startline.status_code = METHOD_NOT_ALLOWED;
		return -1;
	}
	
}