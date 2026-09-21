#pragma once

#include <netdb.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cerrno>
#include <vector>

class LocationConfig;

template <typename T>
std::string toString(const T val)
{
	std::stringstream stream;
	stream << val;
	return stream.str();
}

int			error_msg(int err_code, const char *func, const char *msg);

int			listen_socket(struct addrinfo *addr, int listen_num);

std::string	inet_ntop(int af, const void *src);

std::string	getFileExtension(std::string file);

bool		isFileEmpty(std::fstream &file);

int			checkFileStatus(int flags, std::string &file);

/* Conversion utils */

void		toupperString(std::string &s);

int			hexToAscii(std::string s);

std::string	getDate();

int			strtoInt(std::string s);

/* INITs */

void		init_responses();

void		init_MIMEtypes();

void		init_allowedMethods();

std::string	getMIMEtype(std::string ext);

int			getLocationIndex(std::vector<LocationConfig> &locations,
				std::string path);

int			pipePipes(int pipe_in[2], int pipe_out[2]);

void		closePipes(int pipe_in[2], int pipe_out[2]);

void		refactorFile(std::string &path, std::string &file);