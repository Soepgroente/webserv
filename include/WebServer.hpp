#pragma once

#include <chrono>
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
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#define FOREVER 1

#define ERRORPAGE "404"

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

	/* Private variables	*/

	std::vector<Server>					servers;
	std::vector<struct pollfd>			pollDescriptors;
	std::map<int, struct HttpRequest>	requests;
	
	/*	Private functions	*/

	void	initialize();
	void	loopadydoopady();
	void	printServerStruct(const Server& toPrint)	const;
	bool	isServerSocket(int socket)	const;
	void	acceptConnection(int serverSocket);
	void	closeConnection(int fd);
	time_t	getTime()	const;
	bool	timeout(time_t lastPinged)	const;

	void	interpretRequest(HttpRequest& request);

	void	handleClientRead(int clientFd);
	void	handleClientWrite(int clientFd);

	std::string	showErrorPage(std::string error);

	std::vector<struct pollfd>	createPollArray();
};

/*	Template functions	*/

template <typename T>
void	printVector(std::vector<T>& toPrint)
{
	for (T& it : toPrint)
	{
		std::cout << it << std::endl;
	}
}

/*	Utility functions	*/

void						errorExit(std::string errorMessage, int errorLocation);
std::vector<std::string>	stringSplit(std::string toSplit);