#include "Client.hpp"

void	Client::readIncomingRequest()
{
	ssize_t		readBytes;
	std::string	readBuffer(BUFFERSIZE, 0);

	readBytes = read(fd, &readBuffer[0], BUFFERSIZE);
	if (readBytes == -1)
	{
		printToLog("Error reading from client_fd: " + std::string(strerror(errno)));
		status = CLOSING;
		return ;
	}
	request.buffer += readBuffer.substr(0, readBytes);
	interpretRequest();
}

void	Client::readFromFile()
{
	int pollIndex = getPollfdIndex(Client::fileAndCgiDescriptors, fileFd);
	if (Client::fileAndCgiDescriptors[pollIndex].revents == 0)
		return ;
	response.buffer.resize(response.buffer.size() + BUFFERSIZE);
	ssize_t readBytes = read(fileFd, &response.buffer[readPos], BUFFERSIZE);
	if (readBytes < BUFFERSIZE)
	{
		readPos = 0;
		closeAndResetFd(Client::fileAndCgiDescriptors, fileFd);
		if (readBytes == -1)
		{
			printToLog("Error reading from file: " + std::string(strerror(errno)));
			setupErrorPage(internalServerError);
			return ;
		}
		if (status == parseCgi)
		{
			if (response.buffer.substr(0, 6) == "Error:")
				setupErrorPage(std::stoi(response.buffer.substr(7, 3)));
			else
			{
				response.reply = response.buffer;
				status = RESPONDING;
			}
		}
		else
		{
			response.buffer.resize(response.buffer.size() - (BUFFERSIZE - readBytes));
			response.constructResponse(request.status, request.fileType, response.buffer.size());
			status = RESPONDING;
		}
	}
	else
		readPos += readBytes;
}
