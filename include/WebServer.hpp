#pragma once

#include <assert.h>
#include <chrono>
#include <cstring>
#include <errno.h>
#include <exception>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <netdb.h>
#include <poll.h>
#include <signal.h>
#include <sstream>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "Client.hpp"
#include "Server.hpp"
#include "utils.hpp"

#define FOREVER 1
#define BUFFERSIZE 8 * 1024
#define CHUNKED_EOF "0\r\n\r\n"

struct	Server;
struct	pollfd;
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
	
	static int64_t	getTime();

	private:

	/* Private variables	*/

	std::vector<Server>		servers;
	std::vector<Client>		clients;
	std::vector<pollfd>		pollDescriptors;
	bool					serverShouldRun;
	
	/*	Private functions	*/
	

	void	initialize();
	void	loopadydoopady();
	bool	isServerSocket(size_t position)	const;
	void	acceptConnection(int serverSocket);
	void	closeConnection(int pollIndex, int clientIndex);
	bool	timeout(int64_t lastPinged, int64_t timeout)	const;

	//void	handleResponse(Client& client);

	//std::string				showErrorPage(std::string error);
	std::vector<pollfd>		createPollArray();
	void					set_signals();
	Client*					getClient(int clientFd);
	size_t					getClientIndex(int clientFd) const;
	const Server&			getServer(int serverSocket);

	void	addClient(int serverSocket);
	void	removeClient(int fd);
	void	removeInactiveConnections();

/* 	bool	handleGet(Client& client, std::string& buffer);
	bool	handlePost(Client& client, std::string& buffer);
	bool	handleDelete(Client& client, std::string& buffer); */

	size_t	getFileCgiIndex(int fileFd)	const;
};
