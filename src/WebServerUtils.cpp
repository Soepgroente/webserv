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

void	WebServer::closeAndResetFd(int& fd)
{
	int pollFdIndex = getPollfdIndex(fd);

	close(fd);
	fd = -1;
	pollDescriptors.erase(pollDescriptors.begin() + pollFdIndex);
}

int	WebServer::openFile(const char* path)
{
	int fd = open(path, O_RDONLY);
	if (fd == -1)
	{
		throw std::runtime_error("Failed to open file even though it exists");
	}
	if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) == -1)
	{
		close(fd);
		throw std::runtime_error("Failed to set file fd to non-blocking");
	}
	pollDescriptors.push_back({fd, POLLIN, 0});
	return (fd);
}