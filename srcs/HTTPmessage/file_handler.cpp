#include "HTTPmessage.hpp"
#include <iostream>//borrar

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

	if (query_pos != std::string::npos)
	{
		rq_startline.query = file.substr(query_pos + 1);
		file = file.substr(0, query_pos);
	}
	rq_startline.file = file;
	return file;
}

/* Constructs the full path of file or directory into string file and returns the current location */
LocationConfig *HTTPmessage::getFullPath(std::string &full_path)
{
	if (serverconf == NULL)
		return NULL;

	full_path = parsePath();
	int location_pos = getLocationIndex(serverconf->locations, full_path);

	if (location_pos < 0)
	{
		std::string root = serverconf->root;
		refactorFile(root, full_path);
		CGIparse(NULL);
		return NULL;
	}

	LocationConfig &locationconf = serverconf->locations[location_pos];
	std::string root = locationconf.root.empty()
		? serverconf->root
		: locationconf.root;
	refactorFile(root, full_path);

	CGIparse(&locationconf);

	std::cout << "checkpoint: " << full_path << std::endl;
	return &locationconf;
}

/* When the GET path is like "/thing" meaning to grab a file but it's actually a directory*/
int	HTTPmessage::manageActuallyDir()
{
	if (full_path.empty() || full_path[full_path.length() - 1] == '/')
		return 0;

	std::string redirect_url = full_path + "/";
	
	if (!rq_startline.query.empty())
		redirect_url += "?" + rq_startline.query;

	rs_headers["Location"] = redirect_url;

	rs_body = "<html><body><h1>301 Moved Permanently</h1></body></html>\n";
	rs_content_length = rs_body.length();
	rs_headers["Content-Type"] = "text/html";

	return 1;
}
