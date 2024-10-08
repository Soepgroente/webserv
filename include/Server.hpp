#pragma once

#include "Project.hpp"

struct	Server
{
	long	host;
	int16_t	port;
	int		fd;

	std::string	serverName;
};