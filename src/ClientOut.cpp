#include "Client.hpp"

void	Client::writeToClient()
{
	ssize_t	writtenBytes;

	writtenBytes = write(fd, &response.buffer[writePos], \
		std::min(response.buffer.size() - writePos, (size_t)BUFFERSIZE));
	if (writtenBytes == -1)
	{
        std::cerr << "Error writing to client_fd: " << strerror(errno) << std::endl;
		status = CLOSING;
		return ;
	}
	if (writtenBytes < BUFFERSIZE)
	{
		status = LISTENING;
		writePos = 0;
	}
	else
		writePos += BUFFERSIZE;
}

void	Client::parseDirectory()
{

	status = RESPONDING;
}

void	Client::handleOutgoingState()
{
	std::map<clientStatus, std::function<void (Client*)>> outgoingMethods =
	{
		{RESPONDING, &Client::writeToClient},
		{launchCgi, &Client::launchCGI},
		{showDirectory, &Client::parseDirectory},
		{readingFromFile, &Client::readFromFile},
	};
	outgoingMethods[status](this);
}