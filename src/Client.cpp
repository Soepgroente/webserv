#include "Client.hpp"

Client::Client(const Server& in) : 
	timeout(DEFAULT_TIMEOUT), remainingRequests(INT_MAX), status(clientIsActive), \
	cgiFd(-1), cgiStatus(cgiIsFalse), request(HttpRequest()), \
	response(HttpResponse()), server(in)
{
}

Client::~Client()
{
}

Client::Client(const Client& other) : 
	latestPing(other.latestPing), timeout(other.timeout), \
	remainingRequests(other.remainingRequests), status(other.status), \
	fd(other.fd), cgiFd(other.cgiFd), cgiStatus(other.cgiStatus), \
	request(other.request), response(other.response), server(other.server)
{
}

Client&	Client::operator=(const Client& other)
{
	Client* tmp = new Client(other);

	return (*tmp);
}

time_t	Client::getLatestPing() const
{
	return (latestPing);
}

time_t	Client::getTime() const
{
	return (time(nullptr));
}

void	Client::setPingTime()
{
	latestPing = getTime();
}

time_t	Client::getTimeout() const
{
	return (timeout);
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

HttpResponse&	Client::getResponse()
{
	return (response);
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

void	Client::setTimeout(time_t newTimeout)
{
	if (newTimeout != 0)
		timeout = newTimeout;
}

void	Client::receivedRequest()
{
	remainingRequests--;
	if (remainingRequests == 0)
		status = clientShouldClose;
}

void	Client::setRemainingRequests(int input)
{
	remainingRequests = input;
}

int	Client::getRemainingRequests() const
{
	return (remainingRequests);
}

int	Client::getClientStatus() const
{
	return (status);
}

std::ostream&	operator<<(std::ostream& out, const Client& p)
{
	out << "Client fd: " << p.getFd() << std::endl;
	out << "Client cgiFd: " << p.getCgiFd() << std::endl;
	out << "Client cgiStatus: " << p.getCgiStatus() << std::endl;
	out << "Client server fd: " << p.getServer().socket << std::endl;
	out << "Client latest ping: " << p.getLatestPing() << std::endl;
	out << "Client timeout: " << p.getTimeout() << std::endl;
	out << "Client remaining requests: " << p.getRemainingRequests() << std::endl;
	out << "Client status: " << p.getClientStatus() << std::endl;
	return (out);
}