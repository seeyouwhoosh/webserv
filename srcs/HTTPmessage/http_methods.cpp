#include "HTTPmessage.hpp"

/* Get the file descriptor and fill the gaps for the response accordingly (such as status code, body, content lenght and other needed headers) */
int	HTTPmessage::httpGET() //implementar cgi
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

int	HTTPmessage::httpPOST() // implementar cgi
{
	full_path = rq_startline.raw_path;
	bool			create = false;

	if (rq_body.size() != rq_content_length)
		return -setStatusCode(BAD_REQUEST);

	getFullPath(full_path);
	if (checkFileStatus(F_OK, full_path) < 0)
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