#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <cstdint>
#include "HttpRequest.hpp"

#define MAXBODYSIZE 50000000

struct Location
{
	bool		directoryListing;

	std::vector<std::string>	methods;
	std::vector<std::string>	cgiExtensions;

	std::map<std::string, std::string>	dirs = 
	{
		{"root", ""},
		{"directory_listing", ""},
		{"index", ""},
		{"upload_dir", ""},
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