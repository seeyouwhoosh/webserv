#include "HTTPmessage.hpp"

/* Appends path with file, manages primarily backslashes. Modifies file */
static void	refactorFile(std::string &path, std::string &file)
{
	if (path.empty())
		return;
	if (file[0] == '/')
		file.erase(0,0);

	if (path[path.size() - 1] == '/')
		file = path + file;
	else
		file = path + "/" + file;
}

/* Returns location index if it exist, it not, returns -1 */
int	getLocationIndex(std::vector<LocationConfig> &locations, std::string path)
{
	size_t		max_len = 0;
	size_t		location_pos = -1;

	for (size_t i = 0; i < locations.size(); i++)
	{
		size_t	len = locations[i].name.length();
		
		if (!path.find(locations[i].name))
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

void	HTTPmessage::autoindexParentPath(std::stringstream &page)
{
	if (rq_startline.path == "/")
		return;

	std::string parent_path = rq_startline.path;
	if (parent_path.length() > 1 && parent_path[parent_path.length() - 1] == '/')
		parent_path.erase(parent_path.length() - 1);

	size_t last_slash = parent_path.find_last_of('/');
	if (last_slash != std::string::npos)
		parent_path = parent_path.substr(0, last_slash + 1);
	else
		parent_path = "/";

	page << "<tr><td><a href=\"" << parent_path << "\">Parent directory/</a></td></tr>";
}

void	HTTPmessage::autoindexAddElement(std::stringstream &page, std::string dir_path, std::string name)
{
	std::string		fullFilePath = dir_path + name;
	char buffer[20];

	struct stat	st;
	if (stat(fullFilePath.c_str(), &st) < 0)
		return;

	bool is_dir = ((st.st_mode & S_IFMT) == S_IFDIR);
	if (is_dir)
		name += "/";

	time_t		mod_time = st.st_mtime;
	struct tm	*timeinfo = std::localtime(&mod_time);
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

void	HTTPmessage::autoindexHtmlHead(std::stringstream &page, std::string virtual_path)
{
	page << "<!DOCTYPE html><html><head><title>Index of " << virtual_path << "</title>"
		<< "<style>"
		<< "table { width: 65%; border-collapse: collapse; }"
		<< "th, td { text-align: left; font-family: \"Courier New\", Courier, monospace; padding: 4px 8px; }"
		<< "</style></head><body>"
		<< "<h1>Index of " << virtual_path << "</h1><hr>"
		<< "<table><thead><tr id=\"theader\"><th>Name</th><th>Last modified</th><th>Size</th></tr></thead>"
		<< "<tbody id=\"tbody\">";
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

	autoindexHtmlHead(page, rq_startline.path);

	struct dirent	*entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;

		if (name == ".")
			continue;
		if (name == "..")
		{
			autoindexParentPath(page);
			continue;
		}
		autoindexAddElement(page, dir_path, name);
	}

	closedir(dir);

	page << "</tbody></table></body></html>";

	rs_body = page.str();
	rs_content_length = rs_body.length();
	rs_headers["Content-Type"] = "text/html";
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
		}
		if (locationconf.autoindex == AUTOINDEX_ON)
			return createAutoindex(file);
		return FORBIDDEN;
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
		}
		if (serverconf.autoindex == AUTOINDEX_ON)
			return createAutoindex(file);
		return FORBIDDEN;
	}
	return NOT_FOUND;
}

/* Cleans the given raw_path from request and returns a string without queries and path_info */
std::string	HTTPmessage::parsePath()
{
	std::string	file = rq_startline.raw_path;
	size_t		query_pos = file.find('?');
	size_t		app_pos = file.find('.');

	if (query_pos != std::string::npos)
	{
		rq_startline.query = file.substr(query_pos + 1);
		file = file.substr(0, query_pos);
	}
	if (app_pos != std::string::npos)
	{
		size_t		app_end = file.find('/', app_pos);
		std::string	app_ext = (app_end == std::string::npos) ? file.substr(app_pos + 1) : file.substr(app_pos, app_end - app_pos - 1);
		if (!matchCGI(app_ext).empty())
		{
			rq_startline.iscgi = true;
			rq_startline.path_info = file.substr(app_end);
			file = file.substr(0, app_end);
		}
		else
			rq_startline.iscgi = false;
	}
	rq_startline.file = file;
	return file;
}

/* Constructs the full path of file or directory into string file and returns the current location */
LocationConfig	*HTTPmessage::getFullPath(std::string &full_path)
{
	if (serverconf == NULL)
	{
		setStatusCode(INTERNAL_SERVER_ERROR);
		return NULL;
	}
	full_path = parsePath();

	int	location_pos = getLocationIndex(serverconf->locations, full_path);

	if (location_pos < 0)
	{
		std::string root = serverconf->root;
		refactorFile(root, full_path);
		return NULL;
	}

	LocationConfig	&locationconf = serverconf->locations[location_pos];
	std::string root = locationconf.root.empty() ? serverconf->root : locationconf.root;
	refactorFile(root, full_path);

	return &locationconf;
}

/* When the GET path is like "/thing" meaning to grab a file but it's actually a directory*/
int	HTTPmessage::manageActuallyDir()
{
	if (full_path.empty() || full_path[full_path.length() - 1] == '/')
		return 0;
	setStatusCode(MOVED_PERMANENTLY);

	std::string redirect_url = full_path + "/";
	
	if (!rq_startline.query.empty())
		redirect_url += "?" + rq_startline.query;

	rs_headers["Location"] = redirect_url;

	rs_body = "<html><body><h1>301 Moved Permanently</h1></body></html>\n";
	rs_content_length = rs_body.length();
	rs_headers["Content-Type"] = "text/html";

	return 1;
}
