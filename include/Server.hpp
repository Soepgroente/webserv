#pragma once

#include <cstdint>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <map>

#define MAXBODYSIZE 50000000
#define MAXCONFIGSIZE 1000000

struct Location
{
	std::vector<std::string>	methods;
	std::vector<std::string>	cgiExtensions;

	std::map<std::string, std::string>	dirs = 
	{
		{"root", ""},
		{"directory_listing", ""},
		{"index", ""},
		{"redirection", ""},
	};
};

struct	Server
{
	uint16_t	port;
	int			socket = -1;
	int32_t		maxBodySize;

	std::string	serverName;
	std::string	errorLocation;
	std::string	host;
	std::string	cgiPath;

	std::map<std::string, Location>	locations;
};

std::ostream&	operator<<(std::ostream& out, const struct Server& p);
std::ostream&	operator<<(std::ostream& out, const Location& location);