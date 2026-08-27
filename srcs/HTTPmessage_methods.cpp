#include "HTTPmessage.hpp"

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
			fs.open(file.c_str(), std::ios::in | std::ios::binary);
			if (!fs.is_open())
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
				fs.open(tmp.c_str(), std::ios::in | std::ios::binary);
				if (!fs.is_open())
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

	if ((st.st_mode & S_IFMT) == S_IFREG)
	{
		if (access(file.c_str(), F_OK | R_OK) < 0)
			return FORBIDDEN;
		fs.open(file.c_str(), std::ios::in | std::ios::binary);
		if (!fs.is_open())
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
			fs.open(tmp.c_str(), std::ios::in | std::ios::binary);
			if (!fs.is_open())
				return INTERNAL_SERVER_ERROR;
			file = tmp;
			break;
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
		rq_startline.query = file.substr(query_pos + 1);
		file = file.substr(0, query_pos);
		rq_startline.path = file;
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
int	HTTPmessage::httpGET() //implementar cgi
{
	full_path = rq_startline.path;
	LocationConfig	*locationconf = getPathfile(full_path);
	
	std::fstream	fs;
	if (!locationconf)
	{
		if ((rs_startline.status_code = openFile(full_path, fs, *serverconf)) != OK)
			return -1;
	}
	else
	{
		if ((rs_startline.status_code = openFile(full_path, fs, *locationconf)) != OK)
			return -1;
	}

	rs_body.resize(rs_content_length);
	fs.read(&rs_body[0], rs_content_length);
	if (fs.fail())
	{
		rs_body.clear();
		setStatusCode(INTERNAL_SERVER_ERROR);
		return -1;
	}
	setStatusCode(OK);
	rs_content_type = getMIMEtype(getFileExtension(full_path));
	return 0;
}

int	HTTPmessage::httpPOST() // implementar cgi
{
	full_path = rq_startline.path;
	bool			create = false;

	if (rq_body.size() != rq_content_length)
	{
		setStatusCode(BAD_REQUEST);
		return -1;
	}
	
	struct stat	st;

	getPathfile(full_path);
	if (stat(full_path.c_str(), &st) < 0)
		create = true;

	std::fstream	fs;

	fs.open(full_path.c_str(), std::ios::out | std::ios::binary);
	if (!fs.is_open())
	{
		setStatusCode(FORBIDDEN);
		return -1;
	}
	fs.write(rs_body.c_str(), rq_content_length);

	if (fs.fail())
	{
		fs.close();
		setStatusCode(INTERNAL_SERVER_ERROR);
		return -1;
	}
	fs.close();

	if (create)
		setStatusCode(CREATED);
	else
		setStatusCode(OK);

	return 0;
}

int	HTTPmessage::httpDELETE()
{
	full_path = rq_startline.path;
	getPathfile(full_path);

	struct stat	st;
	if (stat(full_path.c_str(), &st) < 0)
	{
		setStatusCode(NOT_FOUND);
		return -1;
	}
	if ((st.st_mode & S_IFMT) == S_IFDIR)
	{
		setStatusCode(NOT_IMPLEMENTED);
		return -1;
	}

	if (access(full_path.c_str(), F_OK | R_OK |X_OK) < 0 || std::remove(full_path.c_str()) != 0)
	{
		setStatusCode(FORBIDDEN);
		return -1;
	}

	return 0;
}