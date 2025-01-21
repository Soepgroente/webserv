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

bool	WebServer::isServerSocket(int socket)	const
{
	for (const Server& it : servers)
	{
		if (it.socket == socket)
			return (true);
	}
	return (false);
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

time_t	WebServer::getTime()	const
{
	return (std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
}

bool	WebServer::timeout(time_t lastPinged, time_t timeout)	const
{
	if (lastPinged == 0)
		return (false);
	if (getTime() - lastPinged > timeout)
		return (true);
	return (false);
}

int	WebServer::getPollfdIndex(int fdToFind)
{
	if (fdToFind == -1)
		return (-1);
	for (size_t i = 0; i < pollDescriptors.size(); i++)
	{
		if (pollDescriptors[i].fd == fdToFind)
			return (static_cast<int>(i));
	}
	throw std::runtime_error("Pollfd not found");
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
	Client	newClient(getServer(serverSocket));

	newClient.initializeSocket(serverSocket);
	assert(duplicateClient(clients, newClient) == false);
	clients.push_back(newClient);
}

void	WebServer::removeClient(int clientIndex)
{
	if (clients[clientIndex].getCgiFd() != -1)
		close(clients[clientIndex].getCgiFd());
	close(clients[clientIndex].getFd());
	clients.erase(clients.begin() + clientIndex);
}