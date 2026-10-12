#include "HTTPmessage.hpp"
#include "ConfigParser.hpp"
#include "CGIHandler.hpp"
#include "utils.hpp"

/**
 * @brief Get the file descriptor and fill the gaps for the response accordingly (such as status code, body, content length and other needed headers)
 */
int HTTPmessage::httpGET()
{
	full_path = rq_startline.raw_path;
	struct stat st;
	LocationConfig *locationconf = getFullPath(full_path);

	/* return directive */
	if (locationconf && (locationconf->return_status.first != 0 || !locationconf->return_status.second.empty()))
	{
		if (locationconf->return_status.first == 0 && !locationconf->return_status.second.empty())
		{
			rs_headers["Location"] = locationconf->return_status.second;
			setStatusCode(FOUND);
		}
		else if (locationconf->return_status.first != 0)
		{
			setStatusCode(locationconf->return_status.first);
			if (!locationconf->return_status.second.empty())
			{
				std::string val = locationconf->return_status.second;
				if (val.find("http://") == 0 || val.find("https://") == 0 || val[0] == '/')
					rs_headers["Location"] = val;
				else 
				{
					rs_headers["Content-Type"] = mime_types["plain"];
					rs_body = val;
					rs_content_length = val.size();
				}
			}
		}
		return 0;
	}

	/* when user forgets to write a '/' */
	if (stat(full_path.c_str(), &st) == 0 &&
		(st.st_mode & S_IFMT) == S_IFDIR &&
		manageActuallyDir())
	{
		setStatusCode(MOVED_PERMANENTLY);
		return 0;
	}

	int status;
	std::fstream fs;

	if (cgi->isCGI())
	{
		status = cgi->execute(*this);
		if (cgi->getPipeOut() > 0)
			return IS_CGI;
	}
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

int HTTPmessage::parseMultipartToBody(const std::string& contentHeader)
{
	size_t	bPos = contentHeader.find("boundary=");
	if (bPos == std::string::npos)
		return false;

	std::string	boundary = "--" + contentHeader.substr(bPos + 9);
	if (boundary.length() > 2 && boundary[2] == '"')
	{
		boundary.erase(2, 1);
		if (!boundary.empty() && boundary[boundary.length() - 1] == '"')
			boundary.erase(boundary.length() - 1, 1);
	}

	std::string	body_copy = "";
	size_t		pos = 0;

	while ((pos = rq_body.find(boundary, pos)) != std::string::npos)
	{
		if (rq_body.substr(pos, boundary.length() + 2) == boundary + "--")
			break;

		pos += boundary.length();

		size_t	header_end = rq_body.find("\r\n\r\n", pos);
		size_t	sep_len = 4;
		if (header_end == std::string::npos)
		{
			header_end = rq_body.find("\n\n", pos);
			sep_len = 2;
			if (header_end == std::string::npos)
				break;
		}
		std::string	headers = rq_body.substr(pos, header_end - pos + sep_len);

		size_t	filename_pos;
		if ((filename_pos = headers.find("filename=")) != std::string::npos)
		{
			if (headers[filename_pos + 9] != '\"')
				return -1;
			size_t		filename_end = headers.find("\"", filename_pos + 10);
			upload_name = headers.substr(filename_pos + 10, filename_end - filename_pos - 10);
		}

		size_t	content_start = header_end + sep_len;

		size_t	content_end = rq_body.find(boundary, content_start);
		if (content_end == std::string::npos)
			break;

		size_t bodypart_end = content_end;
		if (bodypart_end >= content_start + 2 && rq_body.substr(bodypart_end - 2, 2) == "\r\n")
			bodypart_end -= 2;
		else if (bodypart_end >= content_start + 1 && rq_body[bodypart_end - 1] == '\n')
			bodypart_end -= 1;

		body_copy.append(rq_body, content_start, bodypart_end - content_start);

		pos = content_end;
	}

	if (body_copy.empty())
		return -1;

	rq_body = body_copy;
	rq_content_length = rq_body.size();

	return 1;
}

int	HTTPmessage::handleSpecialType()
{
	stringmap::iterator it = rq_headers.find("content-type");

	if (it != rq_headers.end())
	{
		if (it->second.find("multipart/form-data") != std::string::npos)
			return parseMultipartToBody(it->second);
		if (it->second.find("application/x-www-form-urlencoded") != std::string::npos)
		{
			processURLencoded();
			setStatusCode(OK);
			return 0;
		}
	}

	return 1;
}

int	HTTPmessage::httpPOST()
{
	if (rq_body.size() != rq_content_length)
		return -setStatusCode(BAD_REQUEST);

	full_path = rq_startline.raw_path;
	LocationConfig	*locationconf = getFullPath(full_path);
	
	if (rq_content_length > locationconf->max_body_size)
		return setStatusCode(CONTENT_TOO_LARGE);

	int				status;
	if (cgi->isCGI())
	{
		status = cgi->execute(*this);
		if (status == OK && cgi->getPipeIn() > 0 && cgi->getPipeOut() > 0)
		{
			std::cout << "EVERYTHING OK AS CGI: " << status << std::endl;
			return IS_CGI;
		}
	}

	if ((status = handleSpecialType()) != 1)
		return status;

	if ((locationconf && locationconf->upload_store.empty()))
		return -setStatusCode(NOT_FOUND);
	
	struct stat	st;
	std::string	upload_store = locationconf->upload_store;

	if (stat(upload_store.c_str(), &st) < 0 || access(upload_store.c_str(), X_OK | W_OK) < 0)
		return -setStatusCode(FORBIDDEN);

	if (upload_name.empty())
		upload_name = "upload_" + toString(time(NULL));
	refactorFile(upload_store, upload_name);
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
