#include "Client.hpp"

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

void	Client::setClientStatus(clientStatus newStatus)
{
	status = newStatus;
}

std::ostream&	operator<<(std::ostream& out, const Client& p)
{
	out << "Client fd: " << p.getFd() << std::endl;
	out << "Client server fd: " << p.getServer().socket << std::endl;
	out << "Client latest ping: " << p.getLatestPing() << std::endl;
	out << "Client timeout: " << p.getTimeout() << std::endl;
	out << "Client remaining requests: " << p.getRemainingRequests() << std::endl;
	out << "Client status: " << p.getClientStatus() << std::endl;
	return (out);
}