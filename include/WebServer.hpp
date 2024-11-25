#pragma once

#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <netdb.h>
#include <poll.h>
#include "Server.hpp"
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#define FOREVER 1

struct	Server;

class	WebServer
{
	public:

	WebServer() = default;
	~WebServer();
	WebServer(const WebServer& original) = delete;
	void operator=(const WebServer& original) = delete;

	void	parseConfigurations(const std::string& fileLocation);
	void	startTheThing();

	private:

	std::vector<Server>			servers;
	std::vector<struct pollfd>	pollDescriptors;
	
	void	initialize();
	void	loopadydoopady();
	void	printServerStruct(const Server& toPrint)	const;
	bool	isServerSocket(int socket);
	void	acceptConnection(int serverSocket);

	std::vector<struct pollfd>	createPollArray();
};

void	errorExit(std::string errorMessage, int errorLocation);