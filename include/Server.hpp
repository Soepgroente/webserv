#pragma once

#include <iostream>
#include <vector>
#include <map>

struct Location
{
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
	int16_t		socket;
	int32_t		bodySize;

	std::string	serverName;
	std::string	errorLocation;
	std::string	host;

	std::map<std::string, struct Location>	locations;
};