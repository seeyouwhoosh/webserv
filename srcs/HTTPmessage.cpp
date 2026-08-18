#include <cctype>
#include <fstream>

#include "HTTPmessage.hpp"

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

int	HTTPmessage::method_match(std::string &s)
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

/* See if request is valid, if it is, returns 0, else return -1 and sets status_code */
int	HTTPmessage::request_is_valid()
{
	std::string			s;
	request_startline_t	r;

	if (all_whitespace(client_buffer.str()))
	{
		rs_startline.status_code = BAD_REQUEST;
		return -1;
	}

	client_buffer >> s;
	if ((rq_startline.method = method_match(s)) == BAD_METHOD)
	{
		rs_startline.status_code = METHOD_NOT_ALLOWED;
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
/* If it's a file, see if it's accesible and open it. If it's a directory, see if autoindex is on and search in the indexes list to search for the first available one */
int	HTTPmessage::open_file(std::string file, std::fstream &fs, LocationConfig &locationconf)
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
LocationConfig	&HTTPmessage::get_file_path(std::string &file)
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
int	HTTPmessage::http_get()
{
	std::string		file = rq_startline.target;
	std::fstream	fs;

	LocationConfig	&locationconf = get_file_path(file);
	if ((rs_startline.status_code = open_file(file, fs, locationconf)) != OK)
		return -1;

	char	*buff;
	fs.read(buff, rs_content_length);
	if (fs.fail() == true)
	{
		rs_startline.status_code = INTERNAL_SERVER_ERROR;
		return -1;
	}
	rs_body = std::string(buff);
}

int	HTTPmessage::generate_response(std::string &s)
{
	rs_startline.protocol = "HTTP/1.1";

	int	method = method_match(s);
	switch (method)
	{
	case GET:
		http_get();
		break;
	case POST:
		http_post();
		break;
	case DELETE:
		http_delete();
	case HEAD:
		http_head();
	case TRACE:
		http_trace();
	default:
		return BAD_METHOD;
	}
	rs_startline.reason_phrase = std::string(response[rs_startline.status_code]);
}
