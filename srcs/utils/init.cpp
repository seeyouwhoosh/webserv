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

