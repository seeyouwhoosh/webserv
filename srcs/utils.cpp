/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xwu <xwu@student.42urduliz.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/30 13:35:00 by xwu               #+#    #+#             */
/*   Updated: 2026/07/30 13:35:00 by xwu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"
#include "ConfigParser.hpp"
#include "HTTPmessage.hpp"

stringmap					mime_types;
std::map<int, const char *>	responses;

void	init_MIMEtypes()
{
	mime_types["plain"] = "text/plain";
	mime_types["html"]  = "text/html";
	mime_types["htm"]   = "text/htm";
	mime_types["css"]   = "text/css";
	mime_types["csv"]   = "text/csv";
	mime_types["xml"]   = "text/xml";

	mime_types["png"]   = "image/png";
	mime_types["gif"]   = "image/gif";
	mime_types["jpeg"]  = "image/jpeg";
	mime_types["avif"]  = "image/avif";
	mime_types["jpg"]   = "image/jpg";
	mime_types["svg"]   = "image/svg+xml";

	mime_types["mp4"]   = "video/mp4";
	mime_types["mpeg"]  = "video/mpeg";
	mime_types["webm"]  = "video/webm";

	mime_types["json"]  = "application/json";
	mime_types["pdf"]   = "application/pdf";
	mime_types["rtf"]   = "application/rtf";
	mime_types["zip"]   = "application/zip";
	mime_types["cgi"]   = "application/cgi";

	mime_types["aac"]   = "audio/aac";
	mime_types["midi"]  = "audio/midi";
	mime_types["wav"]   = "audio/wav";

	mime_types["ttf"]   = "font/ttf";
	mime_types["otf"]   = "font/otf";
	mime_types["woff"]  = "font/woff";
	mime_types["woff2"] = "font/woff2";

	mime_types["mjs"]   = "application/javascript";
	mime_types["js"]    = "application/javascript";
	mime_types["md"]    = "text/markdown";
	mime_types["avi"]   = "video/x-msvideo";
	mime_types["bin"]   = "application/octet-stream";
	mime_types["doc"]   = "application/msword";
	mime_types["docx"]  = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	mime_types["epub"]  = "application/epub+zip";
	mime_types["jar"]   = "application/java-archive";
	mime_types["mpkg"]  = "application/vnd.apple.installer+xml";
	mime_types["ogx"]   = "application/ogg";
	mime_types["php"]   = "application/x-httpd-php";
	mime_types["ppt"]   = "application/vnd.ms-powerpoint";
	mime_types["pptx"]  = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	mime_types["rar"]   = "application/vnd.rar";
	mime_types["odp"]   = "application/vnd.oasis.opendocument.presentation";
	mime_types["ods"]   = "application/vnd.oasis.opendocument.spreadsheet";
	mime_types["odt"]   = "application/vnd.oasis.opendocument.text";
	mime_types["sh"]    = "application/x-sh";
	mime_types["tar"]   = "application/x-tar";
	mime_types["xhtml"] = "application/xhtml+xml";
	mime_types["xls"]   = "application/vnd.ms-excel";
	mime_types["xlsx"]  = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
	mime_types["7z"]    = "application/x-7z-compressed";

	mime_types["m4a"]   = "audio/mp4";
	mime_types["mp3"]   = "audio/mpeg";
	mime_types["oga"]   = "audio/ogg";
	mime_types["opus"]  = "audio/ogg";
	mime_types["weba"]  = "audio/webm";
}

void	init_responses()
{
	responses[OK]						= "OK";
	responses[CREATED]					= "Created";
	responses[NO_CONTENT]				= "No Content";
	responses[MOVED_PERMANENTLY]		= "Moved Permanently";
	responses[FOUND]					= "Found";
	responses[BAD_REQUEST]				= "Bad Request";
	responses[FORBIDDEN]				= "Forbidden";
	responses[NOT_FOUND]				= "Not Found";
	responses[METHOD_NOT_ALLOWED]		= "Method Not Allowed";
	responses[CONTENT_TOO_LARGE]		= "Content Too Large";
	responses[UNSUPPORTED_MEDIA_TYPE]	= "Unsupported Media Type";
	responses[UNPROCESSABLE_CONTENT]	= "Unprocessable Content";
	responses[INTERNAL_SERVER_ERROR]	= "Internal Server Error";
	responses[NOT_IMPLEMENTED]			= "Not Implemented";
	responses[GATEWAY_TIMEOUT]			= "Gateway Timeout";
}

/**
 * @brief outputs error message if msg is not NULL (else, picks from strerror) and return -err_code
 * @note Takes into account the SHoW_LOGS macro
 w*/
int	sendError(int err_code, const char *func, const char *msg)
{
	if (!SHOW_LOGS)
		return -err_code;
	std::cerr << "\033[1;31m[Error] \033[0;31m";
	if (func)
		std::cerr << func << ": ";
	if (msg)
		std::cerr << msg << "\n\033[0m";
	else
		std::cerr << strerror(err_code) << "\n\033[0m";
	return -err_code;
}
/**
 * @brief Gets the date in the format "day-of-week, day month year, hour:minutes:second GMT"
 */
std::string	getDate()
{
	time_t		rawtime;
	struct tm	*tm;

	time(&rawtime);
	tm = gmtime(&rawtime);

	if (!tm)
		return "";

	char buffer[30];
	if (strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", tm) == 0)
		return "";

	return std::string(buffer);
}

int	pipePipes(int pipe_in[2], int pipe_out[2])
{
	if (pipe(pipe_out) < 0)
		return -1;

	if (pipe(pipe_in) < 0)
	{
		close(pipe_out[0]);
		close(pipe_out[1]);
		return -1;
	}
	return 0;
}
/**
 * @brief Sets a fd to non blocking
 * @return When success, returns 0, else, closes fd and returns -1
 */
int	setNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
	{
		close(fd);
		return 1;
	}
	return 0;
}

void	closePipes(int pipe_in[2], int pipe_out[2])
{
	close(pipe_in[0]);
	close(pipe_in[1]);
	close(pipe_out[0]);
	close(pipe_out[1]);
}

/**
 * @brief Prepares listening socket
 * 
 * Created a socket, sets it to non blocking, bind it to addr and starts listening
 * 
 * @return If it fails, return value < 0, else returns socket
 * @note Prints out error messages */
int	listenSocket(struct addrinfo *addr, int listen_num)
{
	int	serversock;
	int	status = 1;
	int	yes = 1;

	if (!addr)
		return sendError(-1, "listenSocket", "not a valid struct");

	for (struct addrinfo *p = addr; p != NULL; p = p->ai_next)
	{
		if ((serversock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
		{
			status = errno;
			continue;
		}
		if (setNonBlocking(serversock) < 0)
			return sendError(errno, "fcntl", NULL);
		setsockopt(serversock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
		if (bind(serversock, p->ai_addr, p->ai_addrlen) < 0)
		{
			status = errno;
			close(serversock);
			serversock = -1;
			continue;
		}
		if (listen(serversock, listen_num) < 0)
		{
			close(serversock);
			return sendError(errno, "listen", NULL);
		}
		return serversock;
	}
	return sendError(status, "listenSocket", NULL);
}

bool	allDigits(std::string s)
{
	for (size_t i = 0; i < s.size(); i++)
	{
		if (!std::isdigit(s[i]))
			return false;
	}
	return true;
}

int matchDirective(std::string &s)
{
	const char	*d[] = {"root", "alias", "autoindex", "max_body_size", "limit_except", "upload_store", "return", "timeout", "cgi_timeout"};

	for (size_t i = 0; i < 9; i++)
	{
		if (s == d[i])
			return 1 << i;
	}
	return 0;
}

/**
 * @brief Get the file extension without the dot
 */
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

/**
 * @brief Checks if file exists or if user has enough permissions
 */
int	checkFileStatus(int flags, std::string &file)
{
	struct stat	st;

	if (stat(file.c_str(), &st) < 0)
		return NOT_FOUND;

	if (access(file.c_str(), flags) < 0)
		return FORBIDDEN;

	return OK;
}

/**
 * @brief Returns location index if it exist, it not, returns -1
 */
int	getLocationIndex(std::vector<LocationConfig> &locations, std::string path)
{
	size_t		max_len = 0;
	size_t		location_pos = -1;

	for (size_t i = 0; i < locations.size(); i++)
	{
		std::string	location_name = locations[i].name[0] == '/' ? locations[i].name : "/" + locations[i].name;
		size_t	len = location_name.length();
	
		if (!path.find(location_name))
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

/**
 * @brief Appends path with file, manages primarily backslashes. Modifies file
 */
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

/**
 * @brief Takes a file stream and dump it all into a string
 */
std::string	dumptoString(std::fstream &f)
{
	if (isFileEmpty(f))
		return "";

	std::stringstream ss;
	ss << f.rdbuf();
	
	return ss.str();
}

void	toupperString(std::string &s)
{
	for (size_t i = 0; i < s.length(); i++)
		s[i] = std::toupper(s[i]);
}

int	hexToAscii(std::string s)
{
	int result = 0;
	int base = 1;

	for (std::string::reverse_iterator it = s.rbegin(); it != s.rend(); ++it)
	{
		char c = std::toupper(static_cast<unsigned char>(*it));
		int val = 0;

		if (c >= 'A' && c <= 'F')
			val = c - 'A' + 10;
		else if (c >= '0' && c <= '9')
			val = c - '0';
		else
			return -1;
		result += val * base;
		base *= 16;
	}
	return result;
}

/**
 * @brief When received "hola%21" -> "hola!"
 */
void	urlDecode(std::string &s)
{
	std::string result;

	for (std::string::size_type i = 0; i < s.size(); ++i)
	{
		if (s[i] == '%' && i + 2 < s.size())
		{
			int value = hexToAscii(s.substr(i + 1, 2));

			if (value >= 0)
			{
				result += static_cast<char>(value);
				i += 2;
			}
			else
				result += s[i];
		}
		else if (s[i] == '+')
			result += ' ';
		else
			result += s[i];
	}

	s = result;
}

/**
 * @brief Convert IPv4 and IPv6 addresses from binary to text form
 * */
std::string	inet_ntop(int af, const void *src)
{
	std::stringstream ss;
	std::string s;

	if (!src)
		return "";

	if (af == AF_INET)
	{
		const struct in_addr	*addr4 = static_cast<const struct in_addr *>(src);
		uint32_t				addr = ntohl(addr4->s_addr);

		ss << ((addr >> 24) & 255 ) << "." << ((addr >> 16) & 255 ) << "." << ((addr >> 8) & 255 ) << "." << (addr & 255 );
	}
	else if (af == AF_INET6)
	{
		const struct in6_addr *addr6 = static_cast<const struct in6_addr *>(src);
	
		for (size_t i = 0; i < 16; i+=2)
		{
			uint16_t group = (addr6->s6_addr[i] << 8) | addr6->s6_addr[i + 1];
			ss << std::hex << group;
			if (i != 14)
				ss << ":";
		}
	}
	else
		return "";
	s = ss.str();

	return s;
}

long	strtoInt(std::string s)
{
	std::stringstream	ss(s.c_str());
	long				n;

	ss >> n;
	return n;
}

bool	incompleteSemicolons(std::string &s)
{
	std::stringstream	ss(s);
	std::string			buff;

	while (std::getline(ss, buff))
	{
		if (buff.find_first_not_of(" \t\r\n") == std::string::npos)
			continue;

		if (buff.find('{') != std::string::npos || buff.find('}') != std::string::npos)
			continue;

		if (buff.find("server") != std::string::npos || buff.find("location") != std::string::npos)
			continue;
		size_t last_char = buff.find_last_not_of(" \t\r\n");
		if (last_char == std::string::npos || buff[last_char] != ';')
			return true;
	}
	return false;
}

bool	unmatchedBracket(std::string &s)
{
	int		par = 0;
	bool	in_comment = false;

	for (size_t i = 0; i < s.size(); i++)
	{
		if (s[i] == '#')
		{
			in_comment = true;
			continue;
		}
		if (in_comment)
		{
			if (s[i] == '\n')
				in_comment = false;
			continue;
		}
		if (s[i] == '{')
			par++;
		else if (s[i] == '}')
		{
			par--;
			if (par < 0)
				return true;
		}
	}

	if (par)
		return true;
	return false;
}
