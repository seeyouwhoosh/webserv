#include <cctype>
#include <fstream>

#include "HTTPmessage.hpp"

/* Searches for error.html in the location corresponding to the current path.
If location exists, and error_file is set, attempts to open file.
Else if error_file in location is not set, or location is not set, it uses global config.
If everything fails, uses the default given by the webserver.
If error_file does not exist in any case or it fails somehow, returns -1 */
int	HTTPmessage::openErrorFile(int error, std::fstream &fs)
{
	std::stringstream			error_str;
	error_str << error;
	std::string					file = error_str.str() + ".html";
	int							pos = (serverconf) ? get_location(serverconf->locations, rq_startline.path) : -1;

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
	{
		file = std::string(ERRPAGE_DIR) + "/" + error_str.str() + ".html";
	}

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
	rs_content_length = st.st_size;

	return 0;
}

/* Creates an error file into the default error pages directory established in the header */
static long	generateErrorFile(int error, std::string phrase, std::fstream &fs)
{
	std::stringstream	error_str;
	error_str << error;
	std::string			file;
	long				count;

	file = ERRPAGE_DIR;
	file += "/" + error_str.str() + ".html";
	try
	{
		fs.open(file.c_str(), std::fstream::out | std::fstream::in);
	}
	catch(const std::exception& e)
	{
		return -1;
	}

	fs << "<!DOCTYPE html>\n<html>\n  <head>\n    <title>";
	fs << error_str.str() << " " << phrase;
	fs << "\n/title>\n  </head>\n  <body>\n    <h1>";
	fs << error_str.str() << " " << phrase; 
	fs << "</h1>\n  </body>\n</html>";

	count = fs.gcount();
	fs.close();
	return count;
}

/* Given a int error, try to open in, if there is no problem, pass it to the response body. Else, generate own error file in the default error pages directory */
int	HTTPmessage::generateErrorResponse(int error)
{
	std::fstream	err_file;

	if (openErrorFile(error, err_file) < 0)
	{
		long	len = generateErrorFile(error, rs_startline.reason_phrase, err_file);
		if (len < 0)
		{
			setStatusCode(INTERNAL_SERVER_ERROR);
			return -1;
		}
		rs_body.resize(len);
		err_file.read(&rs_body[0], len);
		if (err_file.fail() == true)
		{
			rs_body.clear();
			setStatusCode(INTERNAL_SERVER_ERROR);
			return -1;
		}
		rs_content_length = len;
	}
	if (rs_content_length > 0)
	{
		rs_body.resize(rs_content_length);
		err_file.read(&rs_body[0], rs_content_length);
	}
	if (err_file.fail() == true)
	{
		rs_body.clear();
		setStatusCode(INTERNAL_SERVER_ERROR);
		return -1;
	}
	return 0;
}

/* If no problems were detected in the parsing, matches the method and redirects to the correspondent method function. *NOTE: METHOD FUNCTIONS (in case of success) FILL THE NECESSARY RESPONSE VARIABLES (RS_VARIABLE), EXCEPT FOR RS_DATE, WHICH IS GENERATED AT THE TIME OF INSERTION*
If method function returns -1, error is set and the error generating function is called */
int	HTTPmessage::generateResponse()
{
	int	error = 0;

	if (rs_startline.status_code != BAD_REQUEST)
	{
		std::string	m = rq_startline.method;
		int	e_method = matchMethod(m);
		switch (e_method)
		{
		case GET:
			if (httpGET() < 0)
				error = rs_startline.status_code;
			break;
		case POST:
			if (httpPOST() < 0)
				error = rs_startline.status_code;
			break;
		case DELETE:
			if (httpDELETE() < 0)
				error = rs_startline.status_code;
			break;
		default:
			break;
		}
	}
	if (error && generateErrorResponse(error) < 0)
		return -1;
	return 0;
}

int	HTTPmessage::sendResponse(int client) //
{
	setResponse();

	if (send(client, responseStream.str().c_str(), responseStream.str().length(), 0) < 0)
		return error_msg(errno, "send", NULL);
	return 0;
}

void	HTTPmessage::setResponse()
{
	responseStream << rs_startline.version << " " << rs_startline.status_code << " " << rs_startline.reason_phrase << "\r\n";

	responseStream << "Date: " << rs_date << "\r\n";
	if (rs_startline.status_code == 201 || (rs_startline.status_code / 100) == 3)
		responseStream << "Location: " << rs_location << "\r\n";
	if (!rs_body.empty())
	{
		responseStream << "Content-Type: " << rs_content_type << "\r\n";
		responseStream << "Content-Length: " << rs_content_length << "\r\n";
	}
	if (rs_startline.status_code == 405)
	{
		responseStream << "Allow: ";
		for (size_t i = 0; i < allowedMethods.size(); i++)
		{
			responseStream << allowedMethods[i];
			if (i != allowedMethods.size() - 1)
				responseStream << ", ";
			else
				responseStream << "\r\n";
		}
	}

	if (!rs_body.empty())
		responseStream << "\r\n" << rs_body << "\r\n" ;
}
