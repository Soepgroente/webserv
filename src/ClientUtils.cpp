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
	return (*server);
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

void	Client::setupErrorPage(int error)
{
	std::string	path = "." + server->errorLocation + std::to_string(error) + ".jpg";

	fileFd = openFile(path.c_str(), O_RDONLY, POLLIN, Client::fileAndCgiDescriptors);
	if (fileFd == -1)
	{
		status = RESPONDING;
		response.reply = HttpResponse::defaultResponses[internalServerError] + "text/plain\r\nContent-Length: 47\r\n\r\n500 Internal server error, no error pages found";
		return ;
	}
	request.fileType = getMimeType(".jpg");
	status = readingFromFile;
}
