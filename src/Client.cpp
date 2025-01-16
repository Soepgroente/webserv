#include "Client.hpp"

Client::Client(int serverSocket, const Server& in) : fd(-1), cgiFd(-1), cgiStatus(cgiIsFalse), server(in)
{
	initializeSocket(serverSocket);
}

Client::~Client()
{
	close(fd);
	if (cgiFd != -1)
		close(cgiFd);
}

Client::Client(const Client& other) : fd(other.fd), cgiFd(other.cgiFd), request(other.request), cgiStatus(other.cgiStatus), server(other.server)
{
}

Client&	Client::operator=(const Client& other)
{
	Client* tmp = new Client(other);
	return (*tmp);
}

int	Client::getFd() const
{
	return (fd);
}

void	Client::initializeSocket(int serverSocket)
{
	struct sockaddr_in	clientAddress;
	socklen_t			clientAddressLength = sizeof(sockaddr_in);

	this->fd = accept(serverSocket, reinterpret_cast<sockaddr*> (&clientAddress), \
		&clientAddressLength);
	if (this->fd == -1)
		throw std::runtime_error("Failed to accept client");
	if (fcntl(this->fd, F_SETFL, fcntl(this->fd, F_GETFL, 0) | O_NONBLOCK) == -1)
		throw std::runtime_error("Failed to set client socket to non-blocking");
}

int	Client::getCgiFd() const
{
	return (cgiFd);
}

void	Client::setCgiFd(int newFd)
{
	cgiFd = newFd;
}

HttpRequest&	Client::getRequest()
{
	return (request);
}

void	Client::setRequest(const struct HttpRequest& newRequest)
{
	request = newRequest;
}

void	Client::setCgiStatus(int status)
{
	cgiStatus = status;
}

int	Client::getCgiStatus() const
{
	return (cgiStatus);
}

const Server&	Client::getServer() const
{
	return (server);
}