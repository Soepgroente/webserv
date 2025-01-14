#include "Client.hpp"

Client::Client() : fd(-1), cgiFd(-1), cgiStatus(cgiIsFalse)
{
}

Client::~Client()
{
	close(fd);
	if (cgiFd != -1)
		close(cgiFd);
}

int	Client::getFd() const
{
	return (fd);
}

void	Client::setFd(int serverSocket)
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

struct HttpRequest&	Client::getRequest()
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