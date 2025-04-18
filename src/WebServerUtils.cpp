#include "WebServer.hpp"

void	errorExit(std::string errorMessage, int errorLocation)
{
	if (errorLocation >= 0)
	{
		printToLog(errorMessage + " on line " + std::to_string(errorLocation));
	}
	else
	{
		printToLog(errorMessage);
	}
	throw std::runtime_error("Error: shutting down server");
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

static void	exitGracefullyOnSignal(int signal)
{
	std::string message = "Shutting down after signal " + std::to_string(signal) + " was received...";
	throw std::runtime_error(message);
}

void	WebServer::set_signals()
{
	signal(SIGINT, &exitGracefullyOnSignal);
	signal(SIGTERM, &exitGracefullyOnSignal);
	signal(SIGQUIT, SIG_IGN);
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
	clients.push_back(newClient);
	if (clients.size() >= MAXCLIENTS)
	{
		newClient.setupErrorPage(tooManyRequests);
		pollDescriptors.push_back({clients.back().getFd(), POLLOUT, 0});
	}
	else
	{
		pollDescriptors.push_back({clients.back().getFd(), POLLIN, 0});
	}
}

void	WebServer::removeClient(int clientIndex)
{
	int tmpFd = clients[clientIndex].getFileFd();

	if (clients[clientIndex].getCgiPid() != -1)
	{
		if (kill(clients[clientIndex].getCgiPid(), SIGTERM) == -1)
		{
			printToLog("Failed to kill cgi");
		}
		Client::cgiCounter--;
	}
	if (tmpFd != -1)
	{
		close(tmpFd);
		Client::fileAndCgiDescriptors.erase(Client::fileAndCgiDescriptors.begin() + getFileCgiIndex(tmpFd));
	}
	close(clients[clientIndex].getFd());
	clients.erase(clients.begin() + clientIndex);
}

std::ostream&	operator<<(std::ostream& out, const std::vector<pollfd>& p)
{
	for (size_t i = 0; i < p.size(); i++)
	{
		out << "Index: " << i << std::endl;
		out << "Pollfd: " << p[i].fd << std::endl;
		out << "Events: " << p[i].events << std::endl;
		out << "Revents: " << p[i].revents << std::endl;
		out << std::endl;
	}
	return (out);
}
