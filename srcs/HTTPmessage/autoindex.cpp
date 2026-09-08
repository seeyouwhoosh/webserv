#include "HTTPmessage.hpp"

void	HTTPmessage::autoindexParentPath(std::stringstream &page)
{
	if (rq_startline.raw_path == "/")
		return;

	std::string parent_path = rq_startline.raw_path;
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

	autoindexHtmlHead(page, rq_startline.raw_path);

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
