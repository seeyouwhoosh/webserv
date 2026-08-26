#include "HTTPmessage.hpp"

stringmap					mime_types;
std::map<int, const char *>	responses;

std::string	getMIMEtype(std::string ext)
{
	stringmap::iterator it = mime_types.find(ext);

	if (it == mime_types.end())
		return "application/octet-stream";

	return it->second;
}

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
	responses[BAD_REQUEST]				= "Bad Request";
	responses[FORBIDDEN]				= "Forbidden";
	responses[NOT_FOUND]				= "Not Found";
	responses[METHOD_NOT_ALLOWED]		= "Method Not Allowed";
	responses[UNSUPPORTED_MEDIA_TYPE]	= "Unsupported Media Type";
	responses[INTERNAL_SERVER_ERROR]	= "Internal Server Error";
	responses[NOT_IMPLEMENTED]			= "Not Implemented";
}

int	HTTPmessage::matchMethod(std::string &method)
{
	int	e_methods[] = {
		GET,
		POST,
		DELETE,
		HEAD,
		TRACE,
		BAD_METHOD,
	};

	for (size_t i = 0; i < allowedMethods.size(); i++)
	{
		if (method == allowedMethods[i])
			return e_methods[i];
	}
	return BAD_METHOD;
}

int	HTTPmessage::fillCGIenv()
{
	/* ALL REQUESTS */
	cgi_env.push_back(std::string("SERVER_SOFTWARE=") + "Custom/1.0");
	cgi_env.push_back(std::string("GATEWAY_INTERFACE=") + "cgi/1.1");
	
	cgi_env.push_back("SERVER_PROTOCOL=" + rq_startline.version);
	cgi_env.push_back("SERVER_PORT=" + server_port);
	cgi_env.push_back("SERVER_NAME=" + serverconf->server_name);
	cgi_env.push_back("REQUEST_METHOD=" + rq_startline.method);
	cgi_env.push_back("QUERY_STRING=" + rq_startline.query);

	cgi_env.push_back("CONTENT_LENGTH=" + rq_content_length);
	cgi_env.push_back("CONTENT_TYPE=" + rq_content_type);
	cgi_env.push_back("PATH_INFO=" + parseURL());
	cgi_env.push_back("PATH_TRANSLATED=" + full_path);

	cgi_env.push_back("AUTH_TYPE=");
	cgi_env.push_back("REMOTE_ADDR=");
	cgi_env.push_back("REMOTE_HOST=");
	cgi_env.push_back("REMOTE_IDENT=");
	cgi_env.push_back("REMOTE_USER=");

	cgi_env.push_back("SCRIPT_NAME=" + rq_startline.path);
	cgi_env.push_back(NULL);
}