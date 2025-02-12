#include "Client.hpp"

/*	Writes whichever amount is smaller between BUFFERSIZE and remaining response size to client.
	Clears the request & response structs once everything has been sent	*/

void	Client::writeToClient()
{
	ssize_t	writtenBytes;

	writtenBytes = write(fd, &response.reply[writePos], \
		std::min(response.reply.size() - writePos, (size_t)BUFFERSIZE));
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
		response.clear();
		request.clear();
	}
	else
		writePos += BUFFERSIZE;
}

void	Client::parseDirectory()
{

	status = RESPONDING;
}

/*	Executes a different method based on the status of the Client	*/

void	Client::handleOutgoingState()
{
	std::map<clientStatus, std::function<void (Client*)>> outgoingMethods =
	{
		{RESPONDING, &Client::writeToClient},
		{launchCgi, &Client::launchCGI},
		{parseCgi, &Client::readFromFile},
		{showDirectory, &Client::parseDirectory},
		{readingFromFile, &Client::readFromFile},
	};
	outgoingMethods[status](this);
}
