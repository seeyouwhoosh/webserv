#include "CGIHandler.hpp"
#include "HTTPmessage.hpp"
#include "ServerConfig.hpp"
#include "utils.hpp"

/* ----------------------- Orthodox Canonical Form ----------------------- */

CGIHandler::CGIHandler(): pid(-1), isrunning(false), iscgi(false)
{
	pipe_in[0] = -1;
	pipe_in[1] = -1;
	pipe_out[0] = -1;
	pipe_out[1] = -1;
}

CGIHandler::CGIHandler(CGIHandler &other)
{
	*this = other;
}

CGIHandler	&CGIHandler::operator=(CGIHandler &other)
{
	cgi_env = other.cgi_env;
	argv = other.argv;
	envp = other.envp;
	cgi_path = other.cgi_path;
	script_path = other.script_path;
	filename = other.filename;
	path_info = other.path_info;
	query = other.query;
	iscgi = other.iscgi;
	input = other.input;
	output = other.output;
	return *this;
}

CGIHandler::~CGIHandler()
{
	killScript();
}

/* ---------------------- Class private functions ---------------------- */

std::string	CGIHandler::matchExtension(const std::string &ext,
										const LocationConfig *location) const
{
	if (location)
	{
		cgi_handler_type::const_iterator	it = location->cgi_handlers.find(ext);
		if (it != location->cgi_handlers.end())
			return it->second;
	}
	return "";
}

/**
 * @brief Parse for cgi executable based on their extension
 * 
 * If it's a file that matches with one of the cgi handlers or the extension is
 * ".cgi", then sets path_info accordingly and updates full_path to path_info
 * (else full_path is the same)
 * 
 * If cgi-bin is detected, send it to the parseCGIbinFile function
 */
void	CGIHandler::parseFiletoCGI(std::string &full_path, LocationConfig *location)
{
	size_t cgibin_pos = full_path.find("/cgi-bin/");

	if (cgibin_pos != std::string::npos)
	{
		parseCGIbinFile(cgibin_pos, full_path, location);
		return;
	}

	std::stringstream ss(full_path);
	std::string segment;
	std::string current_path;
	bool found = false;

	while (std::getline(ss, segment, '/'))
	{
		if (segment.empty())
			break;

		if (found)
		{
			path_info += "/" + segment;
			continue;
		}

		current_path += segment;

		struct stat st;

		if (stat(current_path.c_str(), &st) != 0)
			break;

		if ((st.st_mode & S_IFMT) == S_IFDIR)
		{
			current_path += "/";
			continue;
		}

		if ((st.st_mode & S_IFMT) != S_IFREG)
			break;

		size_t pos_ext = segment.find_last_of('.');

		if (pos_ext == std::string::npos)
			break;

		std::string ext_str = segment.substr(pos_ext);
		std::string interpreter = matchExtension(ext_str, location);

		if (interpreter.empty() && ext_str != ".cgi")
			break;

		iscgi = true;
		found = true;

		full_path = current_path;
		script_path = current_path;
		filename = segment;

		if (!interpreter.empty())
			cgi_path = interpreter;
		else
			cgi_path = script_path;
	}
}

/**
 * @brief Cleans the given raw_path from request and sets full_path to a string without queries
 */
void	CGIHandler::separateQuery(std::string &full_path)
{
	size_t		query_pos = full_path.find('?');

	if (query_pos == std::string::npos)
		return;
	
	query = full_path.substr(query_pos + 1);
	full_path = full_path.erase(query_pos);
}

/**
 * @brief Having detected "cgi-bin", searches for the executable path
 * 
 * After the "/cgi-bin/", looks for the next element,
 * if it's a directory, keep searching;
 * else, attach it to the path (for the cgi thingy),
 * the rest of that is attached to path_info
 * 
 * @example The full path is "/html/cgi-bin/bin/binary/some/other/words".
 * Then, first we cut to "bin/binary/some/other/words". We get until "binary"
 *  because there may be a pseudopath for the cgi to use (see PATH_INFO)
 * 
 * @note Updates full_path
 */

void CGIHandler::parseCGIbinFile(size_t cgibin_pos, std::string &full_path, LocationConfig *location)
{
	size_t start_pos = cgibin_pos + std::strlen("/cgi-bin/");
	std::stringstream ss(full_path.substr(start_pos));

	std::string segment;
	std::string current_path = full_path.substr(0, start_pos);
	bool found = false;

	while (std::getline(ss, segment, '/'))
	{
		if (segment.empty())
			break;

		if (found)
		{
			path_info += "/" + segment;
			continue;
		}

		current_path += segment;

		struct stat st;

		if (stat(current_path.c_str(), &st) != 0)
			break;

		if ((st.st_mode & S_IFMT) == S_IFDIR)
		{
			current_path += "/";
			continue;
		}

		if ((st.st_mode & S_IFMT) != S_IFREG)
			break;

		size_t pos_ext = segment.find_last_of('.');

		if (pos_ext == std::string::npos)
			break;

		std::string ext_str = segment.substr(pos_ext);
		std::string interpreter = matchExtension(ext_str, location);

		if (interpreter.empty() && ext_str != ".cgi")
			break;

		iscgi = true;
		found = true;

		full_path = current_path;
		script_path = current_path;
		filename = segment;

		if (!interpreter.empty())
			cgi_path = interpreter;
		else
			cgi_path = script_path;
	}
}

size_t CGIHandler::findHeaderEnd(const std::string &output)
{
	size_t pos = 0;

	while (pos < output.size())
	{
		size_t end = output.find('\n', pos);
		if (end == std::string::npos)
			return std::string::npos;

		size_t line_end = end;
		if (line_end > pos && output[line_end - 1] == '\r')
			--line_end;

		if (line_end == pos)
			return end + 1;

		size_t colon = output.find(':', pos);
		if (colon == std::string::npos || colon >= line_end)
			return std::string::npos;

		if (colon == pos)
			return std::string::npos;

		pos = end + 1;
	}

	return std::string::npos;
}

int CGIHandler::gotoDirectory()
{
	if (script_path.empty())
		return INTERNAL_SERVER_ERROR;

	size_t slash_pos = script_path.find_last_of('/');

	std::string dir_path;

	if (slash_pos == std::string::npos)
		dir_path = ".";
	else if (slash_pos == 0)
		dir_path = "/";
	else
		dir_path = script_path.substr(0, slash_pos);

	struct stat st;

	if (stat(dir_path.c_str(), &st) != 0)
		return FORBIDDEN;

	if ((st.st_mode & S_IFMT) != S_IFDIR)
		return UNPROCESSABLE_CONTENT;

	return chdir(dir_path.c_str());
}

/* ----------------------------------- Fillers ----------------------------------- */

void	CGIHandler::fill_env(const HTTPmessage &request)
{
	cgi_env.reserve(request.rq_headers.size() + 17); // 17 variables from below

	cgi_env.push_back("SERVER_SOFTWARE=Custom/1.0");
	cgi_env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	
	cgi_env.push_back("SERVER_PROTOCOL=" + request.rq_startline.version);
	cgi_env.push_back("SERVER_PORT=" + server_port);
	cgi_env.push_back("SERVER_NAME=" + request.getServerConf()->server_name[0]);
	cgi_env.push_back("REQUEST_METHOD=" + request.rq_startline.method);
	cgi_env.push_back("QUERY_STRING=" + query);

	cgi_env.push_back("CONTENT_LENGTH=" + toString(request.rq_content_length));

	stringmap::const_iterator it = request.rq_headers.find("content-type");
	if (it != request.rq_headers.end())
		cgi_env.push_back("CONTENT_TYPE=" + it->second);
	
	cgi_env.push_back("PATH_INFO=" + path_info);
	cgi_env.push_back("PATH_TRANSLATED=" + request.getServerConf()->root + path_info);

	cgi_env.push_back("AUTH_TYPE=");
	cgi_env.push_back("REMOTE_ADDR=" + client_addr);
	cgi_env.push_back("REMOTE_HOST=" + client_addr);
	cgi_env.push_back("REMOTE_IDENT=");
	cgi_env.push_back("REMOTE_USER=");

	cgi_env.push_back("SCRIPT_NAME=" + filename);

	for (stringmap::const_iterator it = request.rq_headers.begin(); it != request.rq_headers.end(); it++)
	{
		if (it->first == "content-type" || it->first == "content-length")
			continue;
		std::string	var = it->first;
		toupperString(var);
		std::replace(var.begin(), var.end(), '-', '_');
		cgi_env.push_back("HTTP_" + var + "=" + it->second);
	}
}

void CGIHandler::fill_argv()
{
	argv.clear();

	if (cgi_path.empty() || filename.empty())
	{
		argv.push_back(NULL);
		return;
	}

	if (cgi_path == script_path)
		argv.push_back(const_cast<char *>(filename.c_str()));
	else
	{
		argv.push_back(const_cast<char *>(cgi_path.c_str()));
		argv.push_back(const_cast<char *>(filename.c_str()));
	}

	argv.push_back(NULL);
}

void	CGIHandler::fill_envp()
{
	for (size_t i = 0; i < cgi_env.size(); ++i)
		envp.push_back(const_cast<char *>(cgi_env[i].c_str()));
	envp.push_back(NULL);
}

/* --------------------------- Class public functions --------------------------- */

int	CGIHandler::execute(HTTPmessage& request)
{
	int		status;

	if (cgi_path.empty() || script_path.empty())
		return INTERNAL_SERVER_ERROR;

	if ((status = checkFileStatus(F_OK | X_OK, cgi_path)) != OK)
		return status;

	if ((status = checkFileStatus(F_OK | R_OK, script_path)) != OK)
		return status;
	
	if (pipePipes(pipe_in, pipe_out) < 0)
		return INTERNAL_SERVER_ERROR;

	setNonBlocking(pipe_out[0]);
	setNonBlocking(pipe_in[1]);

	fill_env(request);
	fill_argv();
	fill_envp();
	input = request.rq_body;

	pid = fork();
	switch (pid)
	{
		case -1:
		{
			closePipes(pipe_in, pipe_out);
			return INTERNAL_SERVER_ERROR;
		}
		case 0:
		{
			if ((status = gotoDirectory()) != 0)
				exit(status);

			dup2(pipe_in[0], STDIN_FILENO);
			dup2(pipe_out[1], STDOUT_FILENO);

			closePipes(pipe_in, pipe_out);

			execve(argv[0], argv.data(), envp.data());
			exit(INTERNAL_SERVER_ERROR);
		}
		default:
		{
			close(pipe_in[0]);
			close(pipe_out[1]);

			last_activity = std::time(NULL); 
			isrunning = true;
			return OK;
		}
	}
}


void CGIHandler::parseOutput(HTTPmessage &request)
{
	size_t	end_of_header = findHeaderEnd(output);
	if (end_of_header == std::string::npos)
		return;

	std::string			header_part = output.substr(0, end_of_header);
	output.erase(0, end_of_header);

	std::stringstream	ss(header_part);
	std::string			line;

	while (std::getline(ss, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		if (line.empty())
			continue;

		size_t colon_pos = line.find(':');
		if (colon_pos == std::string::npos)
			continue;

		std::string key = line.substr(0, colon_pos);
		std::string value = line.substr(colon_pos + 1);

		size_t first = value.find_first_not_of(" \t");
		value = (first == std::string::npos)
			? ""
			: value.substr(first);

		request.setHeader(key, value);
	}
}

void	CGIHandler::killScript()
{
	int status;

	if (isrunning && pid > 0)
	{
		kill(pid, SIGKILL);
		waitpid(pid, &status, WNOHANG);

		if (pipe_in[1] != -1)
		{
			close(pipe_in[1]);
			pipe_in[1] = -1;
		}
		if (pipe_out[0] != -1)
		{
			close(pipe_out[0]);
			pipe_out[0] = -1;
		}
	}
	clear();
}

void	CGIHandler::clear()
{
	cgi_env.clear();
	argv.clear();
	envp.clear();
	cgi_path.clear();
	script_path.clear();
	filename.clear();
	path_info.clear();
	query.clear();
	pid = -1;
	pipe_in[0] = -1;
	pipe_in[1] = -1;
	pipe_out[0] = -1;
	pipe_out[1] = -1;
	isrunning = false;
	iscgi = false;
	output.clear();
	last_activity = -1;
}

/* ------------------ Getters and setters and booleans ------------------ */

const std::string	&CGIHandler::getQuery() const
{
	return query;
}

int	CGIHandler::getPipeOut() const
{
	return pipe_out[0];
}

void	CGIHandler::setPipeIn(int fd)
{
	pipe_in[1] = fd;
}

int	CGIHandler::getPipeIn() const
{
	return pipe_in[1];
}

pid_t	CGIHandler::getPID() const
{
	return pid;
}

bool	CGIHandler::isRunning()
{
	return isrunning;
}

void	CGIHandler::setRunning(bool b)
{
	isrunning = b;
}

/**
 * @brief Set client address
*/
void	CGIHandler::setClientAddress(std::string client_address)
{
	client_addr = client_address;
}

bool	CGIHandler::isCGI()
{
	return iscgi;
}

/**
 * @brief Set server port
*/
void	CGIHandler::setServerPort(std::string ServerPort)
{
	server_port = ServerPort;
}
