#include "Client.hpp"

Client::Client(int serverSocket, const Server& in) : cgiFd(-1), cgiStatus(cgiIsFalse), request(HttpRequest()), server(in)	
{
	initializeSocket(serverSocket);
}

Client::~Client()
{
}

Client::Client(const Client& other) : fd(other.fd), cgiFd(other.cgiFd), \
	cgiStatus(other.cgiStatus), request(other.request), server(other.server)
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

	this->fd = accept(serverSocket, reinterpret_cast<sockaddr*> (&clientAddress), &clientAddressLength);
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

std::ostream&	operator<<(std::ostream& out, const Client& p)
{
	out << "Client fd: " << p.getFd() << std::endl;
	out << "Client cgiFd: " << p.getCgiFd() << std::endl;
	out << "Client cgiStatus: " << p.getCgiStatus() << std::endl;
	out << "Client server fd: " << p.getServer().socket << std::endl;
	return (out);
}