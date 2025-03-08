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
	if (request.buffer.size() > MAXBODYSIZE)
	{
		setupErrorPage(payloadTooLarge);
		return ;
	}
	interpretRequest();
}

void	Client::readFromFile()
{
	int pollIndex = getPollfdIndex(Client::fileAndCgiDescriptors, fileFd);
	if (Client::fileAndCgiDescriptors[pollIndex].revents == 0)
	{
		return ;
	}
	response.buffer.resize(response.buffer.size() + BUFFERSIZE);
	ssize_t readBytes = read(fileFd, &response.buffer[readPos], BUFFERSIZE);
	if (readBytes < BUFFERSIZE && readBytes >= 0)
		response.buffer.resize(response.buffer.size() - (BUFFERSIZE - readBytes));
	if (response.status == defaultStatus && status == parseCgi)
	{
		size_t index = response.buffer.find("\r\n\r\n");
		if (index != std::string::npos)
		{
			index += 4;
			size_t contentLengthIndex = response.buffer.find("Content-Length: ");

			if (contentLengthIndex == std::string::npos)
			{
				setupErrorPage(internalServerError);
				return ;
			}
			contentLengthIndex += 16;
			response.status = headerIsParsed;
			response.cgiContentLength = std::stoi(response.buffer.substr(contentLengthIndex, response.buffer.find("\r\n", contentLengthIndex)));
			response.cgiLength = response.cgiContentLength + index;
		}
	}
	if (readBytes < BUFFERSIZE)
	{
		if (readBytes == -1)
		{
			readPos = 0;
			closeAndResetFd(Client::fileAndCgiDescriptors, fileFd);
			printToLog("Error reading from file: " + std::string(strerror(errno)));
			setupErrorPage(internalServerError);
			return ;
		}
		if (status == parseCgi)
		{
			if (response.buffer.substr(0, 7) == "Error: ")
			{
				try
				{
					setupErrorPage(std::stoi(response.buffer.substr(7, 3)));
				}
				catch (std::exception& e)
				{
					setupErrorPage(internalServerError);
				}
			}
			else if (response.status == headerIsParsed && response.buffer.size() == response.cgiLength)
			{
				response.reply = response.buffer;
				// std::cout << response.reply.substr(0, 100) << std::endl;
				status = RESPONDING;
				Client::cgiCounter--;
			}
			else
				readPos += readBytes;
		}
		else
		{
			response.constructResponse(request.status, request.fileType, response.buffer.size());
			status = RESPONDING;
		}
		if (status == RESPONDING)
		{
			readPos = 0;
			closeAndResetFd(Client::fileAndCgiDescriptors, fileFd);
		}
	}
	else
		readPos += readBytes;
}
