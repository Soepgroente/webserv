#pragma once

#include <assert.h>
#include <chrono>
#include <cstring>
#include <errno.h>
#include <exception>
#include <fcntl.h>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <netdb.h>
#include <poll.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "Client.hpp"
#include "Server.hpp"
#include "Utils.hpp"

#define FOREVER 1
#define MAXBODYSIZE 1000000
#define ERRORPAGE "404"

struct	Server;
class	Client;

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
	std::vector<Client>					clients;
	std::vector<struct pollfd>			pollDescriptors;
	bool								serverShouldRun;
	
	/*	Private functions	*/

	void	initialize();
	void	loopadydoopady();
	bool	isServerSocket(int socket)	const;
	void	acceptConnection(int serverSocket);
	void	closeConnection(int fd);
	time_t	getTime()	const;
	bool	timeout(time_t lastPinged)	const;

	void	interpretRequest(HttpRequest& request, int clientFd);
	bool	handleClientRead(Client* client, int clientFd);
	bool	handleClientWrite(int clientFd);
	void	parseCgiOutput(Client& client);

	std::string					showErrorPage(std::string error);
	std::vector<struct pollfd>	createPollArray();
	size_t						getPollfdIndex(int fdToFind);
	void						set_signals();

	void	launchCGI(Client& client);
	Client*	getClient(int clientFd);
	size_t	getClientIndex(int clientFd) const;
	const Server&	getServer(int serverSocket);
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
std::ostream&				operator<<(std::ostream& out, const std::vector<struct pollfd>& p);