#include "WebServer.hpp"

#define BUFFERSIZE 8 * 1024

size_t	WebServer::getClientIndex(int clientFd)	const
{
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].getFd() == clientFd || clients[i].getCgiFd() == clientFd)
			return (i);
	}
	throw std::runtime_error("Client not indexed");
}

Client*	WebServer::getClient(int fileDes)
{
	if (this->clients.empty() == true)
		return (nullptr);
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].getFd() == fileDes || clients[i].getCgiFd() == fileDes)
		{
			return (&clients[i]);
		}
	}
	if (isServerSocket(fileDes) == true)
		return (nullptr);
	// std::cout << "Client_fd: " << clientFd << std::endl;
	throw std::runtime_error("Client not found");
}

void	WebServer::closeConnection(int fd)
{
	int clientIndex = getClientIndex(fd);
	int pollIndex = getPollfdIndex(fd);
	int cgiPollIndex = getPollfdIndex(clients[clientIndex].getCgiFd());

	removeClient(clientIndex);
	pollDescriptors.erase(pollDescriptors.begin() + pollIndex);
	if (cgiPollIndex != -1)
		pollDescriptors.erase(pollDescriptors.begin() + cgiPollIndex);
	std::cout << "Closed connection" << std::endl;
}

static bool	getContentType(HttpRequest& req, size_t i)
{
	if (req.splitRequest[i].size() <= 14)
		return (false);
	if (req.splitRequest[i].substr(0, 14) != "Content-Type: ")
		return (false);
	req.contentType = req.splitRequest[i].substr(14);
	return (true);
}

static bool	getContentLength(HttpRequest& req, size_t i)
{
	if (req.splitRequest[i].size() <= 16)
		return (false);
	if (req.splitRequest[i].substr(0, 16) != "Content-Length: ")
	{
		std::cout << req.splitRequest[i].substr(0, 16) << std::endl;
		return (false);
	}
	req.contentLength = std::stoi(req.splitRequest[i].substr(16));
	// if (req.contentLength > MAXBODYSIZE)
		//do a thing
	return (true);
}

static bool	getHost(HttpRequest& req, size_t i)
{
	size_t	splitter = req.splitRequest[i].find_last_of(':');

	req.host = req.splitRequest[i].substr(6, splitter - 6);
	req.port = req.splitRequest[i].substr(splitter + 1);
	return (true);
}

static bool	getMethods(HttpRequest& req, size_t i)
{
    std::stringstream	stream;

	stream.str(req.splitRequest[i]);
	stream >> req.method >> req.path >> req.protocol;
	return (true);
}

static bool	requestIsFinished(const HttpRequest& request)
{
	if (request.contentLength != 0)
	{
		if (request.body.size() == request.contentLength)
			return (true);
	}
	else if (request.splitRequest.back().empty() == true)
	{
		return (true);
	}
	return (false);
}

static bool	parseHeaders(HttpRequest& request)
{
	Server		server;
	int			count = 0;
	bool (*func[4])(HttpRequest&, size_t) = {&getMethods, &getHost, &getContentType, &getContentLength};
	
	if (request.rawRequest.find("\r\n\r\n") == std::string::npos || request.host.size() != 0)
		return (false);
	request.splitRequest = stringSplit(request.rawRequest);
	for (size_t i = 0; i < request.splitRequest.size() && count < 4; i++)
	{
		count += func[count](request, i);
	}
	request.rawRequest.erase(request.rawRequest.begin(), request.rawRequest.begin() + request.rawRequest.find("\r\n\r\n") + 4);
	return (true);
}

static void	parseBody(HttpRequest& request)
{
	if (request.contentLength == request.splitRequest.back().size())
		request.body += request.splitRequest.back();
}

void	WebServer::interpretRequest(HttpRequest& request, int clientFd)
{
	if (parseHeaders(request) == false)
		return ;
	parseBody(request);
	if (requestIsFinished(request) == true)
	{
		assert(getPollfdIndex(clientFd) != -1);
		pollDescriptors[getPollfdIndex(clientFd)].events = POLLOUT;
		try
		{
			request.fileType = request.path.substr(request.path.find_last_of('.'));
		}
		catch (std::out_of_range& e){}
	}
}

bool	WebServer::handleClientRead(Client* client, int clientFd)
{
	ssize_t			readBytes;
	std::string		buffer;

	assert(client != nullptr);
	HttpRequest&	request = client->getRequest();

	if (timeout(request.lastRead) == true)
	{
		closeConnection(clientFd);
		return (false);
	}
	buffer.resize(BUFFERSIZE);
	readBytes = read(clientFd, buffer.data(), BUFFERSIZE);
	if (readBytes == -1)
	{
		throw std::runtime_error("Error reading from client_fd");
	}
	if (readBytes == 0)
	{
		closeConnection(clientFd);
		return (false);
	}
	buffer.resize(readBytes);
	request.lastRead = getTime();
	request.rawRequest += buffer;
	interpretRequest(request, clientFd);
	if (request.fileType == ".out")
	{
		client->setCgiStatus(launchCgi);
	}
	return (true);
}

void	WebServer::parseCgiOutput(Client& client)
{
	ssize_t			readBytes;
	std::string		buffer;
	// HttpRequest&	request = client.getRequest();

	buffer.resize(BUFFERSIZE);
	readBytes = read(client.getCgiFd(), buffer.data(), BUFFERSIZE);
	if (readBytes == -1)
	{
		throw std::runtime_error("Error reading from client_fd");
	}
	std::cout << buffer << std::endl;
	client.setCgiStatus(cgiIsFalse);
}
