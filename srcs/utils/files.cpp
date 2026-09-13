#include "utils.hpp"

std::string	getFileExtension(std::string file)
{
	size_t dot = file.find_last_of('.');
	size_t slash = file.find_last_of('/');

	if (dot == std::string::npos || dot < slash + 1)
		return "plain";

	return file.substr(dot + 1);
}

bool isFileEmpty(std::fstream &file)
{
	return file.peek() == std::ios::traits_type::eof();
}

int	checkFileStatus(int flags, std::string &file)
{
	struct stat	st;

	if (stat(file.c_str(), &st) < 0)
		return NOT_FOUND;

	if (access(file.c_str(), flags) < 0)
		return FORBIDDEN;

	return OK;
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

std::string	getMIMEtype(std::string ext)
{
	stringmap::iterator it = mime_types.find(ext);

	if (it == mime_types.end())
		return "application/octet-stream";

	return it->second;
}

/* Appends path with file, manages primarily backslashes. Modifies file */
void	refactorFile(std::string &path, std::string &file)
{
	if (path.empty())
		return;
	if (file[0] == '/')
		file.erase(0,1);

	if (path[path.size() - 1] == '/')
		file = path + file;
	else
		file = path + "/" + file;
}
