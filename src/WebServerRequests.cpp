#include "WebServer.hpp"

#define BUFFERSIZE 8 * 1024

void	WebServer::handleClientWrite(int clientFd)
{
	(void)clientFd;
}

void	WebServer::closeConnection(int fd)
{
	std::vector<pollfd>::iterator it;
	
	close(fd);
	for (it = pollDescriptors.begin(); it != pollDescriptors.end(); it++)
	{
		if (it->fd == fd)
		{
			pollDescriptors.erase(it);
			break ;
		}
	}
}
// static bool	finishedReading(int clientFd)
// {
// 	struct timeval timeout = {0, 0};
// 	fd_set	readfds;
	
// 	FD_ZERO(&readfds);
// 	FD_SET(clientFd, &readfds);

// 	if (select(clientFd + 1, &readfds, NULL, NULL, &timeout) == 0)
// 		return (true);
// 	return (false);
// }

void	WebServer::handleClientRead(int clientFd)
{
	ssize_t		readBytes;
	std::string	buffer(BUFFERSIZE + 1, '\0');

	readBytes = read(clientFd, &buffer[0], BUFFERSIZE);
	if (readBytes == -1)
	{
		if (errno != EWOULDBLOCK)
		{
			std::cerr << "Client_fd read error" << std::endl;
			// handleError(strerror(errno));
		}
		else
		{
			std::cerr << "Error number: " << errno << " Error msg: " << strerror(errno) << std::endl;
			errorExit("Read failed", -1);
		}
	}
	else if (readBytes == 0)
	{
		closeConnection(clientFd);
		return ;
	}
	requests[clientFd].lastRead = getTime();
	requests[clientFd].req += buffer;
	std::cout << "Amount read: " << readBytes << " , request: " << buffer << std::endl;
}
