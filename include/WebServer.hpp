#pragma once

#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <netdb.h>
#include "Server.hpp"
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>

struct	Server;

class	WebServer
{
	public:

	WebServer() = default;
	~WebServer() = default;
	WebServer(const WebServer& original) = delete;
	void operator=(const WebServer& original) = delete;

	void	parseConfigurations(const std::string& fileLocation);
	void	startTheThing();

	private:

	std::vector<Server>	servers;
	
	void	printServerStruct(const Server& toPrint)	const;
};