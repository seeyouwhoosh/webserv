#pragma once

#include <netdb.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cerrno>
#include <vector>

#ifndef SHOW_LOGS
 # define SHOW_LOGS 1
#endif

class LocationConfig;

template <typename T>
std::string toString(const T val)
{
	std::stringstream stream;
	stream << val;
	return stream.str();
}

/* General */

int			sendError(int err_code, const char *func, const char *msg);

std::string	getDate();

int			pipePipes(int pipe_in[2], int pipe_out[2]);

void		closePipes(int pipe_in[2], int pipe_out[2]);

int			setNonBlocking(int fd);

int			listenSocket(struct addrinfo *addr, int listen_num);

bool		allDigits(std::string s);

int			matchDirective(std::string &s);

/* Files */

std::string	getFileExtension(std::string file);

bool		isFileEmpty(std::fstream &file);

int			checkFileStatus(int flags, std::string &file);

std::string	getMIMEtype(std::string ext);

int			getLocationIndex(std::vector<LocationConfig> &locations, std::string path);

std::string	dumptoString(std::fstream &f);

void		refactorFile(std::string &path, std::string &file);

/* Conversion utils */

std::string	inet_ntop(int af, const void *src);

void		toupperString(std::string &s);

int			hexToAscii(std::string s);

long		strtoInt(std::string s);

void		urlDecode(std::string &s);

/* INITs */

void		init_responses();

void		init_MIMEtypes();

/* Checkers */

bool		unmatchedBracket(std::string &s);

bool		incompleteSemicolons(std::string &s);
