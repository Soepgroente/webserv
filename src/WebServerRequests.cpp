#include "WebServer.hpp"

#define BUFFERSIZE 684

void	WebServer::handleClientWrite(int clientFd)
{
	(void)clientFd;
}

static bool	finishedReading(int clientFd)
{
	struct timeval timeout = {0, 0};
	fd_set	readfds;
	
	FD_ZERO(&readfds);
	FD_SET(clientFd, &readfds);

	if (select(clientFd + 1, &readfds, NULL, NULL, &timeout) == 0)
		return (true);
	return (false);
}

void	WebServer::handleClientRead(int clientFd)
{
	ssize_t		readBytes = BUFFERSIZE;
	std::string	buffer(BUFFERSIZE, '\0');
	std::string	req;

	std::cout << clientFd << std::endl;
	while (readBytes > 0)
	{
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
		std::cout << "Amount read: " << readBytes << " , request: " << buffer << std::endl;
		req += buffer;
		if (finishedReading(clientFd) == true)
			break ;
	}
	close(clientFd);
	std::cout << "Closed connection to client" << std::endl;
}

/* void	WebServer::handleClientRead(int clientFd)
{
	std::string	buffer(BUFFERSIZE, '\0');
	int			bytes_read;
	std::string	response;
	
	bytes_read = read(clientFd, &buffer[0], BUFFERSIZE);
	if (bytes_read < 0)
	{
		if (errno != EWOULDBLOCK)
		{
			std::cerr << "Client_fd read error" << std::endl;
			// handleError(strerror(errno));
		}
	}
	else if (bytes_read == 0)
	{
		close(clientFd);
		// client_pollfd.fd = -1;
		std::cout << "Client -> Connection closed" << std::endl;
	}
	else
	{
		buffer.resize(bytes_read);
		std::cout << "Received: " << buffer << std::endl;
		// response = handleRequest(buffer, servers);
		// client_responses[client_pollfd.fd] = response;
		// client_pollfd.events = POLLOUT;
		// response = 	"HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!";
	}
} */