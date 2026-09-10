#include "HTTPmessage.hpp"

/* Get the file descriptor and fill the gaps for the response accordingly (such as status code, body, content lenght and other needed headers) */
int	HTTPmessage::httpGET()
{
	full_path = rq_startline.raw_path;
	struct stat		st;
	LocationConfig	*locationconf = getFullPath(full_path);

	if (stat(full_path.c_str(), &st) == 0 && (st.st_mode & S_IFMT) == S_IFDIR && manageActuallyDir())
	{
		setStatusCode(MOVED_PERMANENTLY);
		return 0;
	}

	int				status;
	std::fstream	fs;

	if (rq_startline.iscgi)
		status = handleCGI();
	else
	{
		if (!locationconf && !serverconf)
			return -setStatusCode(INTERNAL_SERVER_ERROR);
		if (!locationconf)
			status = openFile(full_path, fs, *serverconf);
		else	
			status = openFile(full_path, fs, *locationconf);
	}

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
		rs_headers["Content-Type"] = getMIMEtype(getFileExtension(full_path));
	}
	setStatusCode(OK);
	return 0;
}

/* Constructs the full path of file or directory into string file and returns the current location (upload file edition) */
int	HTTPmessage::getUploadPath(LocationConfig *locationconf)
{
	if (locationconf->upload_store.empty())
		return NOT_FOUND;

	std::string	upload_name = "upload_" + toString(time(NULL));
	refactorFile(locationconf->upload_store, upload_name);

	return OK;
}

void	HTTPmessage::processURLencoded()
{
	std::stringstream	page;
	std::stringstream	ss;
	ss << rq_body;

	page << "<!DOCTYPE html><html><head><title>Received form answers</title>"
		<< "<style>"
		<< "table { width: 65%; border-collapse: collapse; }"
		<< "th, td { text-align: left; font-family: \"Courier New\", Courier, monospace; padding: 4px 8px; }"
		<< "</style></head><body>"
		<< "<h1>Received form answers</h1><hr>"
		<< "<table><thead><tr id=\"theader\"><th>Field</th><th>Value</th></tr></thead>"
		<< "<tbody id=\"tbody\">";

	std::string	pair;
	while (std::getline(ss, pair, '&'))
	{
		size_t	separator = pair.find('=');

		if (separator == std::string::npos)
		{
			page << "<tr><td>" << pair << "</td><td></td>";
			continue;
		}

		page << "<tr><td>" << pair.substr(0, separator) << "</td>"
			<< "<td>" << pair.substr(separator + 1) << "</td>";
	}
	page << "</tbody></table></body></html>";
	rs_body = page.str();
	rs_content_length = rs_body.length();
	rs_headers["Content-Type"] = "text/html";
}

int	HTTPmessage::httpPOST()
{
	if (rq_body.size() != rq_content_length)
		return -setStatusCode(BAD_REQUEST);

	full_path = rq_startline.raw_path;
	LocationConfig	*locationconf = getFullPath(full_path);
	
	int				status;
	if (rq_startline.iscgi)
	{
		status = handleCGI();
		if (status != OK)
			return -setStatusCode(status);
		return 0;
	}

	if (rq_headers["Content-Type"] == "multipart/form-data")
		return -setStatusCode(NOT_IMPLEMENTED);
	if (rq_headers["Content-Type"] == "application/x-www-form-urlencoded")
	{
		processURLencoded();
		setStatusCode(OK);
		return 0;
	}

	if (locationconf->upload_store.empty())
		return -setStatusCode(NOT_FOUND);
	
	struct stat	st;
	if (stat(locationconf->upload_store.c_str(), &st) < 0 || access(locationconf->upload_store.c_str(), X_OK) < 0)
		return -setStatusCode(FORBIDDEN);

	std::string	upload_name = "upload_" + toString(time(NULL));
	refactorFile(locationconf->upload_store, upload_name);
	full_path = upload_name;

	std::fstream	fs;
	fs.open(full_path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);

	if (!fs.is_open())
		return -setStatusCode(FORBIDDEN);

	fs.write(rq_body.c_str(), rq_content_length);

	if (fs.fail())
	{
		fs.close();
		return -setStatusCode(INTERNAL_SERVER_ERROR);
	}
	fs.close();

	setStatusCode(CREATED);
	return 0;
}

int	HTTPmessage::httpDELETE()
{
	full_path = rq_startline.raw_path;
	getFullPath(full_path);

	struct stat	st;
	if (stat(full_path.c_str(), &st) < 0)
		return -setStatusCode(NOT_FOUND);

	if ((st.st_mode & S_IFMT) == S_IFDIR)
		return -setStatusCode(NOT_IMPLEMENTED);

	if (access(full_path.c_str(), F_OK | R_OK |X_OK) < 0 || std::remove(full_path.c_str()) != 0)
		return -setStatusCode(FORBIDDEN);

	setStatusCode(NO_CONTENT);
	return 0;
}
