#include "WebServer.hpp"

size_t	WebServer::getClientIndex(int clientFd)	const
{
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].getFd() == clientFd || clients[i].getCgiFd() == clientFd)
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
		if (clients[i].getFd() == fileDes || clients[i].getCgiFd() == fileDes || clients[i].getFileFd() == fileDes)
		{
			return (&clients[i]);
		}
	}
	if (isServerSocket(fileDes) == true)
		return (nullptr);
	throw std::runtime_error("Client not found");
}

void	WebServer::closeConnection(int fd)
{
	int clientIndex = getClientIndex(fd);
	int pollIndex = getPollfdIndex(fd);
	int cgiPollIndex = getPollfdIndex(clients[clientIndex].getCgiFd());

	// send response if unfinished business
	removeClient(clientIndex);
	pollDescriptors.erase(pollDescriptors.begin() + pollIndex);
	if (cgiPollIndex != -1)
		pollDescriptors.erase(pollDescriptors.begin() + cgiPollIndex);
	std::cout << "Closed connection" << std::endl;
}

void	WebServer::parseCgiOutput(Client& client)
{
	ssize_t			readBytes;
	std::string		buffer;
	// HttpRequest&	request = client.getRequest();

	buffer.resize(BUFFERSIZE);
	readBytes = read(client.getCgiFd(), buffer.data(), BUFFERSIZE);
	if (readBytes == -1)
	{
		throw std::runtime_error("Error reading from client_fd parsing CGI output");
	}
	client.getResponse().buffer += buffer;
}
