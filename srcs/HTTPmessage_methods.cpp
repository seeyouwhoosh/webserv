#include "HTTPmessage.hpp"

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