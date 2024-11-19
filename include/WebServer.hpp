#pragma once

#include "Server.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <map>
#include <functional>

struct	Server;

class	WebServer
{
	public:

	WebServer() = default;
	~WebServer() = default;
	WebServer(const WebServer& original) = delete;
	void operator=(const WebServer& original) = delete;

	void	parseConfigurations(const std::string& fileLocation);

	private:

	std::vector<Server>	servers;
	
	void	printServerStruct(const Server& toPrint)	const;
};