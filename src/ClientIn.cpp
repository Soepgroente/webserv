#include "Client.hpp"

void	Client::readIncomingRequest()
{
	ssize_t		readBytes;
	std::string	tmp;

	request.status = defaultStatus;
	tmp.resize(BUFFERSIZE);
	readBytes = read(fd, &tmp[0], BUFFERSIZE);
	if (readBytes == -1)
	{
		printToLog("Error reading from client_fd: " + std::string(strerror(errno)));
		status = CLOSING;
		return ;
	}
	request.buffer += tmp.substr(0, readBytes);
	interpretRequest();
}

void	Client::readFromFile()
{
	int pollIndex = getPollfdIndex(Client::fileAndCgiDescriptors, fileFd);
	if (Client::fileAndCgiDescriptors[pollIndex].revents == 0)
		return ;
	response.buffer.resize(response.buffer.size() + BUFFERSIZE);
	ssize_t readBytes = read(fileFd, &response.buffer[readPos], BUFFERSIZE);
	if (readBytes == -1)
	{
		printToLog("Error reading from file: " + std::string(strerror(errno)));
		setupErrorPage(500);
		readPos = 0;
	}
	else if (readBytes < BUFFERSIZE)
	{
		closeAndResetFd(Client::fileAndCgiDescriptors, fileFd);
		readPos = 0;
		if (status == parseCgi)
		{
			if (response.buffer.substr(0, 6) == "Error:")
				setupErrorPage(std::stoi(response.buffer.substr(7, 3)));
			else
			{
				int fd = open("out.txt", O_RDWR | O_CREAT, 0644);
				write(fd, response.buffer.c_str(), response.buffer.size());
				close(fd);
				response.reply = response.buffer;
				status = RESPONDING;
			}
		}
		else
		{
			response.buffer.resize(response.buffer.size() - (BUFFERSIZE - readBytes));
			if (response.reply.empty() == true)
				response.reply = HttpResponse::defaultResponses[200];
			response.reply += std::to_string(response.buffer.size()) + EMPTY_LINE + response.buffer;
			status = RESPONDING;
		}
	}
	else
		readPos += BUFFERSIZE;
}
