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

//check if it exists 
//check if it is a file or directory
// check that the size is countable
// returns fd if it's a feasible file, 0 if it is a directory, -error if its's error
int HTTPmessage::get_file(std::string file)
{
	size_t query_pos = file.find('?');
	std::string query;

	if (query_pos != std::string::npos)
	{
		query = file.substr(query_pos + 1);
		file = file.substr(0, query_pos);
	}

	int	location_pos = get_location(serverconf->locations, file);
	if (location_pos < 0)
		file = serverconf->root + file;
	else
		file = serverconf->locations[location_pos].root + file;

	struct stat	st;

	if (stat(file.c_str(), &st) < 0)
		return -NOT_FOUND;
	else
	{
		if (st.st_mode & S_IFMT == S_IFREG)
		{
			if (access(file.c_str(), F_OK | R_OK) < 0)
				return -FORBIDDEN;

		}
		if (st.st_mode & S_IFMT == S_IFDIR)
		{

		}
	}


}


/* get the file descriptor and fill the gaps for the response accordingly (such as status code, body, content lenght and other needed headers) */
int	HTTPmessage::http_get()
{
	get_file(request_startline.target);
}