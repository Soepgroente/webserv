#include "Client.hpp"

void	Client::readIncomingRequest()
{
	ssize_t		readBytes;
	std::string	tmp;

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

void	Client::readFromFile()
{
	ssize_t		readBytes;
	std::string	buffer;

	status = readingFromFile;
	return ;
	buffer.resize(BUFFERSIZE);
	readBytes = read(fileFd, &buffer[0], BUFFERSIZE);
	if (readBytes == -1)
	{
		std::cerr << "Error reading from fileFd: " << strerror(errno) << std::endl;
		remainingRequests = 0;
		response.buffer = HttpResponse::defaultResponses[404];
		status = RESPONDING;
		return ;
	}
	if (readBytes == 0)
	{
		close(fileFd);
		fileFd = -1;
		status = RESPONDING;
	}
	buffer.resize(readBytes);
	response.buffer += buffer;
}
