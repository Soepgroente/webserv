#include "WebServer.hpp"

bool*	ptr;

void	errorExit(std::string errorMessage, int errorLocation)
{
	std::cerr << errorMessage;
	if (errorLocation >= 0)
		std::cerr << " on line " << errorLocation;
	std::cerr << std::endl;
	std::exit(EXIT_FAILURE);
}

bool	WebServer::isServerSocket(size_t position)	const
{
	return (position < servers.size());
}

std::vector<struct pollfd>	WebServer::createPollArray()
{
	std::vector<struct pollfd>	fileDescriptors;

	for (Server it : servers)
	{
		fileDescriptors.push_back({it.socket, POLLIN, 0});
	}
	return (fileDescriptors);
}

int64_t	WebServer::getTime()
{
	return (std::chrono::duration_cast<std::chrono::milliseconds>\
		(std::chrono::system_clock::now().time_since_epoch()).count());
}

bool	WebServer::timeout(int64_t lastPinged, int64_t timeout)	const
{
	if (WebServer::getTime() - lastPinged > timeout)
	{
		printToLog("Connection timed out");
		return (true);
	}
	return (false);
}


static void	exitGracefullyOnSignal(int signal)
{
	std::cerr << "Shutting down after signal " << signal << " was received..." << std::endl;
	*ptr = false;
}

void	WebServer::set_signals()
{
	ptr = &serverShouldRun;
	signal(SIGINT, &exitGracefullyOnSignal);
	signal(SIGTERM, &exitGracefullyOnSignal);
	signal(SIGQUIT, SIG_IGN);
}

static bool	duplicateClient(const std::vector<Client>& clients, const Client& client)
{
	for (const Client& tmp : clients)
	{
		if (tmp.getFd() == client.getFd())
			return (true);
	}
	return (false);
}

void	WebServer::addClient(int serverSocket)
{
	if (clients.size() >= MAXCLIENTS + MAXDUMMYCLIENTS)
	{
		printToLog("Too many clients, rejecting connection");
		return ;
	}
	Client	newClient(getServer(serverSocket));

	newClient.initializeSocket(serverSocket);
	assert(duplicateClient(clients, newClient) == false);
	clients.push_back(newClient);
	if (clients.size() >= MAXCLIENTS)
	{
		newClient.setupErrorPage(tooManyRequests);
		pollDescriptors.push_back({clients.back().getFd(), POLLOUT, 0});
	}
	else
		pollDescriptors.push_back({clients.back().getFd(), POLLIN, 0});
}

void	WebServer::removeClient(int clientIndex)
{
	int tmpFd = clients[clientIndex].getFileFd();

	if (tmpFd != -1)
	{
		close(tmpFd);
		Client::fileAndCgiDescriptors.erase(Client::fileAndCgiDescriptors.begin() + getFileCgiIndex(tmpFd));
	}
	close(clients[clientIndex].getFd());
	clients.erase(clients.begin() + clientIndex);
}
