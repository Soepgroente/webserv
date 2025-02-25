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
	{
		return ;
	}
	response.buffer.resize(response.buffer.size() + BUFFERSIZE);
	ssize_t readBytes = read(fileFd, &response.buffer[readPos], BUFFERSIZE);
	if (readBytes < BUFFERSIZE && readBytes >= 0)
		response.buffer.resize(response.buffer.size() - (BUFFERSIZE - readBytes));
	// std::cout << response.
	if (response.status == defaultStatus && status == parseCgi)
	{
		size_t index = response.buffer.find("\r\n\r\n") + 4;
		if (index != std::string::npos)
		{
			size_t contentLengthIndex = response.buffer.find("Content-Length: ") + 16;

			if (contentLengthIndex == std::string::npos)
			{
				setupErrorPage(internalServerError);
				return ;
			}
			response.status = headerIsParsed;
			response.cgiContentLength = std::stoi(response.buffer.substr(contentLengthIndex, response.buffer.find("\r\n", contentLengthIndex)));
			response.cgiLength = response.cgiContentLength + index;
			response.reply = response.buffer;
			// response.buffer.clear();
			std::cout << "READ FROM FILE PARSE CGI HEADERS, response.reply.size(): " << response.reply.size() << std::endl;
			
			// response.reply = response.buffer.substr(0, index);
			// response.buffer.erase(0, index);
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
			if (response.buffer.substr(0, 6) == "Error:")
			{
				setupErrorPage(std::stoi(response.buffer.substr(7, 3)));
			}
			else if (response.status == headerIsParsed && response.buffer.size() == response.cgiLength)
			{
				response.reply = response.buffer;
				std::cout << "READ FROM FILE, response.reply.size(): " << response.reply.size() << std::endl;
				status = RESPONDING;
			}
		}
		else
		{
			// response.buffer.resize(response.buffer.size() - (BUFFERSIZE - readBytes));
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
