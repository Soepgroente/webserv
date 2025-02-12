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
		printToLog("Error writing to client fd: " + std::string(strerror(errno)));
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

void	Client::writeToFile()
{
	ssize_t	writtenBytes;

	writtenBytes = write(fileFd, &request.body[writePos], \
		std::min(request.body.size() - writePos, (size_t)BUFFERSIZE));
	if (writtenBytes == -1)
	{
		printToLog("Error writing to file fd: " + std::string(strerror(errno)));
		closeAndResetFd(Client::fileAndCgiDescriptors, fileFd);
		writePos = 0;
		status = CLOSING;
		return ;
	}
	if (writtenBytes < BUFFERSIZE)
	{
		status = RESPONDING;
		writePos = 0;
		response.reply = HttpResponse::defaultResponses[200];
		closeAndResetFd(Client::fileAndCgiDescriptors, fileFd);
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
		{writingToFile, &Client::writeToFile},
		{launchCgi, &Client::launchCGI},
		{parseCgi, &Client::readFromFile},
		{showDirectory, &Client::parseDirectory},
		{readingFromFile, &Client::readFromFile},
	};
	outgoingMethods[status](this);
}
