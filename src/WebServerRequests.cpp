#include "WebServer.hpp"

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
	// send response if unfinished business
	removeClient(clientIndex);
	pollDescriptors.erase(pollDescriptors.begin() + pollIndex);
	// if (clients[clientIndex].getFileFd() != -1)
	// 	Client::fileAndCgiDescriptors.erase(Client::fileAndCgiDescriptors.begin()); // need to get correct index
	std::cout << "Closed connection" << std::endl;
}
