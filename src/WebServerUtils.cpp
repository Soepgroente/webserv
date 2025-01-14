#include "WebServer.hpp"

bool*	ptr;

void	WebServer::printServerStruct(const Server& toPrint)	const
{
	std::cout << "Host: " << toPrint.host << std::endl;
	std::cout << "Port: " << toPrint.port << std::endl;
	std::cout << "Socket: " << toPrint.socket << std::endl;
	std::cout << "Name: " << toPrint.serverName << std::endl;
	std::cout << "Body size: " << toPrint.bodySize << std::endl;
	std::cout << "Error location: " << toPrint.errorLocation << "\n" << std::endl;
	for (const std::pair<std::string, Location> pair : toPrint.locations)
	{
		std::cout << "Location: " << pair.first << std::endl;
		std::cout << "Methods: ";
		for (const std::string& iter : pair.second.methods)
		{
			std::cout << iter << ", ";
		}
		std::cout << std::endl;
		std::cout << "CGI extensions: ";
		for (const std::string& iter : pair.second.cgiExtensions)
		{
			std::cout << iter << ", ";
		}
		std::cout << std::endl;
		for (const std::pair<std::string, std::string> despair : pair.second.dirs)
		{
			std::cout << despair.first << ": " << despair.second << std::endl;
		}
		std::cout << "--------------------------------------------" << std::endl;
	}
	std::cout << "==============================================" << std::endl;
}

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

bool	WebServer::timeout(time_t lastPinged)	const
{
	if (lastPinged == 0)
		return (false);
	if (this->getTime() - lastPinged > 5)
		return (true);
	return (false);
}

size_t	WebServer::getPollfdIndex(int fdToFind)
{
	for (size_t i = 0; i < pollDescriptors.size(); i++)
	{
		if (pollDescriptors[i].fd == fdToFind)
			return (i);
	}
	return (SIZE_MAX);
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