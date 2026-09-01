#include "HTTPmessage.hpp"

/* Appends path with file, manages primarily backslashes. Modifies file */
static void	refactorFile(std::string &path, std::string &file)
{
	if (path.empty())
		return;
	if (path[path.size() - 1] == '/')
		path.erase(path.size() - 1);
	if (file == "/")
	{
		file = path;
		return;
	}
	if (file[0] == '/')
		file = path + file;
	else
		file = path + "/" + file;
	return;
}

/* Returns location index if it exist, it not, returns -1 */
int	get_location(std::vector<LocationConfig> &locations, std::string file)
{
	size_t		max_len = 0;
	size_t		location_pos = -1;

	for (size_t i = 0; i < locations.size(); i++)
	{
		size_t	len = locations[i].name.length();
		
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

int HTTPmessage::createAutoindex(std::string dir_path)
{
	std::stringstream	page;
	DIR *dir = opendir(dir_path.c_str());
	if (!dir)
	{
		if (errno == EACCES)
			return FORBIDDEN;
		if (errno == ENOENT)
			return NOT_FOUND;
		return INTERNAL_SERVER_ERROR;
	}

	if (!dir_path.empty() && dir_path[dir_path.length() - 1] != '/')
		dir_path += "/";

	page << "<!DOCTYPE html><html><head><title>Index of " << rq_startline.path << "</title>"
			<< "<style>"
			<< "table { width: 65%; border-collapse: collapse; }"
			<< "th, td { text-align: left; font-family: \"Courier New\", Courier, monospace; padding: 4px 8px; }"
			<< "</style></head><body>"
			<< "<h1>Index of " << rq_startline.path << "</h1><hr>"
			<< "<table><thead><tr id=\"theader\"><th>Name</th><th>Last modified</th><th>Size</th></tr></thead>"
			<< "<tbody id=\"tbody\">";

	struct dirent	*entry;
	struct stat		st;
	char buffer[64];

	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;

		if (name == ".")
			continue;

		if (name == "..")
		{
			if (rq_startline.path == "/")
				continue;

			std::string parent_path = rq_startline.path;
			if (parent_path.length() > 1 && parent_path[parent_path.length() - 1] == '/')
				parent_path.erase(parent_path.length() - 1);

			size_t last_slash = parent_path.find_last_of('/');
			if (last_slash != std::string::npos)
				parent_path = parent_path.substr(0, last_slash + 1);
			else
				parent_path = "/";

			page << "<tr><td><a href=\"" << parent_path << "\">Parent directory/</a></td>"
					<< "<td>-</td><td>-</td></tr>";
			continue;
		}

		std::string full_path = dir_path + name;

		if (stat(full_path.c_str(), &st) < 0)
			continue;

		bool is_dir = S_ISDIR(st.st_mode);
		if (is_dir)
			name += "/";

		time_t mod_time = st.st_mtime;
		struct tm *timeinfo = std::localtime(&mod_time);
		if (timeinfo)
			std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", timeinfo);
		else
			buffer[0] = '\0';

		page << "<tr><td><a href=\"" << name << "\">" << name << "</a></td>"
				<< "<td>" << buffer << "</td>"
				<< "<td>";

		if (is_dir)
			page << "-";
		else
			page << st.st_size;

		page << "</td></tr>";
	}

	closedir(dir);

	page << "</tbody></table></body></html>";

	rs_body = page.str();
	rs_content_length = rs_body.length();
	rs_content_type = getMIMEtype("html");
	return OK;
}

/* If it's a file, see if it's accesible and open it. If it's a directory, see if autoindex is on and search in the indexes list to search for the first available one */
int	HTTPmessage::openFile(std::string &file, std::fstream &fs, LocationConfig &locationconf)
{
	struct stat							st;
	struct stat							index_st;
	std::vector<std::string>::iterator	it;

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
		return OK;
	}
	else if ((st.st_mode & S_IFMT) == S_IFDIR)
	{
		for (it = locationconf.indexes.begin(); it != locationconf.indexes.end(); it++)
		{
			if (it->empty())
				continue;
			std::string	tmp = *it;
			refactorFile(file, tmp);

			if (stat(tmp.c_str(), &index_st) == 0 && (index_st.st_mode & S_IFMT) == S_IFREG)
			{
				if (access(tmp.c_str(), R_OK) < 0)
					return FORBIDDEN;
				fs.open(tmp.c_str(), std::ios::in | std::ios::binary);
				if (!fs.is_open())
					return INTERNAL_SERVER_ERROR;
				rs_content_length = index_st.st_size;

				file = tmp;
				return OK;
			}
			if (locationconf.autoindex == AUTOINDEX_ON)
				return createAutoindex(file);
			return FORBIDDEN;
		}
	}
	return NOT_FOUND;
}

/* If it's a file, see if it's accesible and open it. If it's a directory, see if autoindex is on and search in the indexes list to search for the first available one */
int	HTTPmessage::openFile(std::string &file, std::fstream &fs, ServerConfig &serverconf)
{
	struct stat							st;
	struct stat							index_st;
	std::vector<std::string>::iterator	it;

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
		return OK;
	}
	else if ((st.st_mode & S_IFMT) == S_IFDIR)
	{
		for (it = serverconf.indexes.begin(); it != serverconf.indexes.end(); it++)
		{
			if (it->empty())
				continue;
			std::string	tmp = *it;
			refactorFile(file, tmp);

			if (stat(tmp.c_str(), &index_st) == 0 && (index_st.st_mode & S_IFMT) == S_IFREG)
			{
				if (access(tmp.c_str(), R_OK) < 0)
					return FORBIDDEN;
				fs.open(tmp.c_str(), std::ios::in | std::ios::binary);
				if (!fs.is_open())
					return INTERNAL_SERVER_ERROR;
				rs_content_length = index_st.st_size;

				file = tmp;
				return OK;
			}
			if (serverconf.autoindex == AUTOINDEX_ON)
				return createAutoindex(file);
			return FORBIDDEN;
		}
	}
	return NOT_FOUND;
}

/* Constructs the full path of file or directory into string file and returns the current location */
LocationConfig	*HTTPmessage::getPathLocation(std::string &file)
{
	if (serverconf == NULL)
	{
		setStatusCode(INTERNAL_SERVER_ERROR);
		return NULL;
	}
	size_t query_pos = file.find('?');
	std::string query; // decide what to do with query after

	if (query_pos != std::string::npos)
	{
		rq_startline.query = file.substr(query_pos + 1);
		file = file.substr(0, query_pos);
	}
	rq_startline.path = file;

	int	location_pos = get_location(serverconf->locations, file);

	if (location_pos < 0)
	{
		std::string root = serverconf->root;
		refactorFile(root, file);
		return NULL;
	}

	LocationConfig	&locationconf = serverconf->locations[location_pos];
	std::string root = locationconf.root.empty() ? serverconf->root : locationconf.root;
	refactorFile(root, file);

	return &locationconf;
}

/* Get the file descriptor and fill the gaps for the response accordingly (such as status code, body, content lenght and other needed headers) */
int	HTTPmessage::httpGET() //implementar cgi
{
	full_path = rq_startline.path;
	LocationConfig	*locationconf = getPathLocation(full_path);
	std::fstream	fs;

	int status;
	if (!locationconf)
		status = openFile(full_path, fs, *serverconf);
	else
		status = openFile(full_path, fs, *locationconf);

	if (status != OK)
		return -setStatusCode(status);

	if (fs.is_open())
	{
		rs_body.resize(rs_content_length);
		fs.read(&rs_body[0], rs_content_length);
		if (fs.fail())
		{
			rs_body.clear();
			return -setStatusCode(INTERNAL_SERVER_ERROR);
		}
		rs_content_type = getMIMEtype(getFileExtension(full_path));
	}
	setStatusCode(OK);
	return 0;
}

int	HTTPmessage::httpPOST() // implementar cgi
{
	full_path = rq_startline.path;
	bool			create = false;

	if (rq_body.size() != rq_content_length)
		return -setStatusCode(BAD_REQUEST);

	struct stat	st;

	getPathLocation(full_path);
	if (stat(full_path.c_str(), &st) < 0)
		create = true;

	std::fstream	fs;

	fs.open(full_path.c_str(), std::ios::out | std::ios::binary);
	if (!fs.is_open())
		return -setStatusCode(FORBIDDEN);
	fs.write(rs_body.c_str(), rq_content_length);

	if (fs.fail())
	{
		fs.close();
		return -setStatusCode(INTERNAL_SERVER_ERROR);
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
	getPathLocation(full_path);

	struct stat	st;
	if (stat(full_path.c_str(), &st) < 0)
		return -setStatusCode(NOT_FOUND);

	if ((st.st_mode & S_IFMT) == S_IFDIR)
		return -setStatusCode(NOT_IMPLEMENTED);

	if (access(full_path.c_str(), F_OK | R_OK |X_OK) < 0 || std::remove(full_path.c_str()) != 0)
		return -setStatusCode(FORBIDDEN);

	return 0;
}