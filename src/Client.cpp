#include "Client.hpp"

Client::Client(const Server& in) : 
	latestPing(INT64_MAX), timeout(DEFAULT_TIMEOUT), remainingRequests(INT_MAX), status(clientIsActive), \
	fd(-1), fileFd(-1),	cgiFd(-1), request(HttpRequest()), \
	response(HttpResponse()), server(in)
{
}

Client::~Client()
{
}

Client::Client(const Client& other) : 
	latestPing(other.latestPing), timeout(other.timeout), \
	remainingRequests(other.remainingRequests), status(other.status), \
	fd(other.fd), fileFd(other.fileFd), cgiFd(other.cgiFd), \
	request(other.request), response(other.response), server(other.server)
{
}

Client&	Client::operator=(const Client& other)
{
	Client* tmp = new Client(other);

	return (*tmp);
}

int64_t	Client::getLatestPing() const
{
	return (latestPing);
}

void	Client::setPingTime()
{
	latestPing = WebServer::getTime();
}

int64_t	Client::getTimeout() const
{
	return (timeout);
}

int	Client::getFd() const
{
	return (fd);
}

int	Client::getFileFd() const
{
	return (fileFd);
}

void	Client::setFileFd(int newFd)
{
	fileFd = newFd;
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

const Server&	Client::getServer() const
{
	return (server);
}

void	Client::setTimeout(int64_t newTimeout)
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

void	Client::setClientStatus(int newStatus)
{
	status = newStatus;
}

std::ostream&	operator<<(std::ostream& out, const Client& p)
{
	out << "Client fd: " << p.getFd() << std::endl;
	out << "Client cgiFd: " << p.getCgiFd() << std::endl;
	out << "Client server fd: " << p.getServer().socket << std::endl;
	out << "Client latest ping: " << p.getLatestPing() << std::endl;
	out << "Client timeout: " << p.getTimeout() << std::endl;
	out << "Client remaining requests: " << p.getRemainingRequests() << std::endl;
	out << "Client status: " << p.getClientStatus() << std::endl;
	return (out);
}