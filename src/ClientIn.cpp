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
		std::cerr << "Error reading from client_fd: " << strerror(errno) << std::endl;
		status = CLOSING;
		return ;
	}
	request.buffer += tmp;
	interpretRequest();
}

void	Client::setupErrorPage(int error)
{
	std::string	path = "." + server.errorLocation + std::to_string(error) + ".jpg";

	std::cout << path << std::endl;
	fileFd = openFile(path.c_str(), Client::fileAndCgiDescriptors);
	response.reply = HttpResponse::defaultResponses[error];
	status = readingFromFile;
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
		if (status == parseCgi && response.buffer.substr(0, 6) == "Error:")
		{
			assert(response.buffer.size() == 10);
			setupErrorPage(std::stoi(response.buffer.substr(7, 3)));
		}
		else
		{
			response.buffer.resize(response.buffer.size() - (BUFFERSIZE - readBytes));
			if (response.reply.empty() == true)
				response.reply = HttpResponse::defaultResponses[200];
			response.reply += std::to_string(response.buffer.size()) + "\r\n\r\n" + response.buffer;
			status = RESPONDING;
		}
	}
	else
		readPos += BUFFERSIZE;
}
