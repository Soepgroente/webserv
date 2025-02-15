#include "WebServer.hpp"

size_t	WebServer::getFileCgiIndex(int fileFd)	const
{
	for (size_t i = 0; i < Client::fileAndCgiDescriptors.size(); i++)
	{
		if (Client::fileAndCgiDescriptors[i].fd == fileFd)
			return (i);
	}
	throw std::runtime_error("File descriptor not found");
}

size_t	WebServer::getClientIndex(int clientFd)	const
{
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].getFd() == clientFd)
			return (i);
	}
	throw std::runtime_error("Client not indexed");
}

Client*	WebServer::getClient(int fileDes)
{
	if (this->clients.empty() == true)
		return (nullptr);
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].getFd() == fileDes || clients[i].getFileFd() == fileDes)
		{
			return (&clients[i]);
		}
	}
	if (isServerSocket(fileDes) == true)
		return (nullptr);
	throw std::runtime_error("Client not found");
}

void	WebServer::closeConnection(int pollIndex, int clientIndex)
{
	int tmpFd = clients[clientIndex].getFileFd();
	if (tmpFd != -1)
	{
		Client::fileAndCgiDescriptors.erase(Client::fileAndCgiDescriptors.begin() + getFileCgiIndex(tmpFd));
		close(tmpFd);
	}
	removeClient(clientIndex);
	pollDescriptors.erase(pollDescriptors.begin() + pollIndex);
	printToLog("Closed connection");
	newConnectionTotal--;
	// std::cout << newConnectionTotal << " connections" << std::endl;
}
