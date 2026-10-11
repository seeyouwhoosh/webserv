#include <fstream>
#include "HTTPmessage.hpp"
#include "ConfigParser.hpp"
#include "CGIHandler.hpp"
#include "utils.hpp"
#include <iostream>

/**
 * @brief Searches for error.html in the location corresponding to the current path.
 * 
 * If location exists, and error_file is set, attempts to open file.
 * 
 * Else if error_file in location is not set, or location is not set, it uses global config.
 * 
 * If everything fails, uses the default given by the webserver.
 * 
 * @return If error_file does not exist in any case or it fails somehow, returns -1
 */
int	HTTPmessage::openErrorFile(int error, std::fstream &fs)
{
	std::string	error_str = toString(error);
	std::string	err_file = error_str + ".html";
	int			pos = (serverconf) ? getLocationIndex(serverconf->locations, rq_startline.raw_path) : -1;
	
	std::string	file;
	if (pos >= 0 && pos < static_cast<int>(serverconf->locations.size()))
	{
		const LocationConfig &location = serverconf->locations[pos];
		std::map<int, std::string>::const_iterator it = location.error_pages.find(error);
		if (it != location.error_pages.end())
			file = location.root + "/" + it->second;
	}
	if (file.empty() && serverconf)
	{
		std::map<int, std::string>::const_iterator it = serverconf->error_pages.find(error);
		if (it != serverconf->error_pages.end())
			file = serverconf->root + "/" + it->second;
	}
	if (file.empty())
		file = std::string(ERRPAGE_DIR) + "/" + error_str + ".html";

	struct stat	st;
	if (stat(file.c_str(), &st) < 0)
		return -1;
	if ((st.st_mode & S_IFMT) != S_IFREG)
		return -1;
	if (access(file.c_str(), F_OK | R_OK) < 0)
		return -1;

	fs.open(file.c_str(), std::ios::in | std::ios::binary);
	if (!fs.is_open() || fs.fail())
		return -1;
	if (isFileEmpty(fs))
	{
		fs.close();
		return -1;
	}
	rs_content_length = st.st_size;
	rs_headers["Content-type"] = getMIMEtype(getFileExtension(file));
	return 0;
}

/**
 * @brief Creates an error file into the default error pages directory established in the header
 */
static std::string	generateErrorFile(int error, std::string phrase, std::fstream &fs)
{
	std::string	error_str = toString(error);
	std::string			file;

	file = std::string(ERRPAGE_DIR) + "/" + error_str + ".html";
	fs.open(file.c_str(), std::fstream::out | std::fstream::in | std::fstream::trunc | std::fstream::binary);
	if (!fs.is_open() || fs.fail())
		return "";

	std::stringstream	content;
	content << "<!DOCTYPE html>\n<html>\n  <head>\n\t<title>";
	content << error_str << " " << phrase;
	content << "</title>\n  </head>\n  <body>\n\t<h1>";
	content << error_str << " " << phrase; 
	content << "</h1>\n  </body>\n</html>\n";
	std::string body = content.str();

	fs << body;
	bool read_failed = fs.fail();

	if (fs.is_open())
		fs.close();

	if (read_failed)
		return "";
	return body;
}

/**
 * @brief Given a int error, try to open in, if there is no problem, pass it to the response body. Else, generate own error file in the default error pages directory
 */
int	HTTPmessage::generateErrorResponse(int error)
{
	std::fstream	err_file;

	if (!openErrorFile(error, err_file) && rs_content_length > 0)
	{
		rs_body.resize(rs_content_length);
		err_file.read(&rs_body[0], rs_content_length);
		bool read_failed = err_file.fail();

		if (err_file.is_open())
			err_file.close();

		if (read_failed)
		{
			rs_body.clear();
			return -setStatusCode(INTERNAL_SERVER_ERROR);
		}
		return 0;
	}

	rs_body = generateErrorFile(error, rs_startline.reason_phrase, err_file);
	rs_content_length = rs_body.size();
	if (!rs_content_length)
		return -setStatusCode(INTERNAL_SERVER_ERROR);

	rs_headers["Content-type"] = "text/html";
	return 0;
}

void	HTTPmessage::setAllowHeader()
{
	std::stringstream	header;

	for (std::vector<std::string>::iterator it = locationconf->allowed_methods.begin(); it != locationconf->allowed_methods.end(); it++)
	{
		header << *it;
		if (it != locationconf->allowed_methods.end() - 1)
			header << ", ";
		else
			header << "\r\n";
	}
	rs_headers["Allow"] = header.str();
}

/**
 * @brief If no problems were detected in the parsing, matches the method and redirects to the correspondent method function.
 * @note METHOD FUNCTIONS (in case of success) FILL THE NECESSARY RESPONSE VARIABLES, EXCEPT FOR Date, WHICH IS GENERATED AT THE TIME OF INSERTION
 * @return If method function returns -1, error is set and the error generating function is called
 * 
 * Else if the response is actually a CGI, returns the writing out pipe of the script */
int	HTTPmessage::generateResponse()
{
	int	error = 0;
	int	status = 0;

	if (SHOW_LOGS)
		printQHeaders();
	if (rs_startline.status_code == OK)
	{
		std::string	m = rq_startline.method;
		int	e_method = matchMethod(m);
		switch (e_method)
		{
		case GET:
			if ((status = httpGET()) < 0)
				error = rs_startline.status_code;
			break;
		case POST:
			if ((status = httpPOST()) < 0)
				error = rs_startline.status_code;
			break;
		case DELETE:
			if ((status = httpDELETE()) < 0)
				error = rs_startline.status_code;
			break;
		default:
			setAllowHeader();
			error = setStatusCode(METHOD_NOT_ALLOWED);
			break;
		}
	}
	else
		error = rs_startline.status_code;
	if (error && generateErrorResponse(error) < 0)
		return -1;

	if (status == IS_CGI)
		return IS_CGI;
	rs_headers["Date"] = getDate();
	return 0;
}

/**
 * @brief Creates a response string
 * @return Return HTTP response
 */
std::string	HTTPmessage::setResponse()
{
	std::stringstream	responseStream;

	responseStream << rs_startline.version << " " << rs_startline.status_code << " " << rs_startline.reason_phrase << "\r\n";

	for (stringmap::iterator it = rs_headers.begin(); it != rs_headers.end(); it++)
		responseStream << it->first << ": " << it->second << "\r\n";

	if (rs_startline.status_code != 204 && rs_startline.status_code != 304)
		responseStream << "Content-Length: " << rs_content_length << "\r\n";

	responseStream << "\r\n";

	rs_header = responseStream.str();
	if (SHOW_LOGS)
		printAHeaders();
	return rs_header + rs_body;
}
