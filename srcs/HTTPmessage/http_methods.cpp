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
LocationConfig	*HTTPmessage::getUploadPath(std::string &full_path)
{
	if (serverconf == NULL)
		return NULL;

	full_path = parsePath();

	int	location_pos = getLocationIndex(serverconf->locations, full_path);

	if (location_pos < 0)
	{
		std::string upload_store = serverconf->upload_store;
		refactorFile(upload_store, full_path);
		CGIparse(NULL);
		return NULL;
	}

	LocationConfig	&locationconf = serverconf->locations[location_pos];
	std::string upload_store = locationconf.upload_store.empty() ? serverconf->upload_store : locationconf.upload_store;
	refactorFile(upload_store, full_path);

	CGIparse(&locationconf);

	return &locationconf;
}

void	parseMultipart()
{
	;
}

int	HTTPmessage::httpPOST() // implementar cgi
{
	if (rq_body.size() != rq_content_length)
		return -setStatusCode(BAD_REQUEST);

	full_path = rq_startline.raw_path;

	bool			create = false;
	int				status;
	struct stat		st;
	
	getFullPath(full_path);
	if (stat(full_path.c_str(), &st) == 0)
	{
		if ((st.st_mode & S_IFMT) != S_IFREG || (!full_path.empty() && full_path[full_path.length() - 1] == '/'))
			return -setStatusCode(METHOD_NOT_ALLOWED);
	}

	if (rq_startline.iscgi)
	{
		status = handleCGI();
		if (status != OK)
			return -setStatusCode(status);
		return 0;
	}

	if (rq_headers["Content-Type"] == "application/x-www-form-urlencoded")
		return -setStatusCode(UNPROCESSABLE_CONTENT);

	if (access(full_path.c_str(), F_OK) < 0)
		create = true;
	if (access(full_path.c_str(), W_OK) < 0)
		return -setStatusCode(FORBIDDEN);
	
	std::fstream	fs;
	fs.open(full_path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);

	if (!fs.is_open())
		return -setStatusCode(FORBIDDEN);

	if (rq_headers["Content-Type"] == "multipart/form-data")
		parseMultipart();
	fs.write(rq_body.c_str(), rq_content_length);

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
