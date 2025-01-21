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
	throw std::runtime_error("Client not found");
}

void	WebServer::closeConnection(int fd)
{
	int clientIndex = getClientIndex(fd);
	int pollIndex = getPollfdIndex(fd);
	int cgiPollIndex = getPollfdIndex(clients[clientIndex].getCgiFd());

	// send response if unfinished business
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

static bool	getConnectionType(HttpRequest& req, size_t i)
{
	if (req.splitRequest[i].size() <= 12)
		return (false);
	if (req.splitRequest[i].substr(0, 12) != "Connection: ")
		return (false);
	req.connectionType = req.splitRequest[i].substr(12);
	return (true);
}

static bool	parseHeaders(HttpRequest& request)
{
	if (request.rawRequest.find("\r\n\r\n") == std::string::npos || request.host.size() != 0)
		return (false); // put this here, otherwise it gets tricky handling strings that are half-finished etc

	std::map<std::string, std::function<bool(HttpRequest&, size_t)>> parseFunctions = 
	{
		{"GET", &getMethods},
		{"POST", &getMethods},
		{"DELETE", &getMethods},
		{"Connection:", &getConnectionType},
		{"Host:", &getHost},
		{"Content-Type:", &getContentType},
		{"Content-Length:", &getContentLength}
	};
	request.splitRequest = stringSplit(request.rawRequest);
	for (size_t i = 0; i < request.splitRequest.size(); i++)
	{
		std::string firstWord = request.splitRequest[i].substr(0, request.splitRequest[i].find(' '));

		if (parseFunctions.find(firstWord) != parseFunctions.end())
		{
			if (parseFunctions[firstWord](request, i) == false)
			{
				// set isValidRequest to false & respond with error message
				return (false);
			}
		}
	}
	request.rawRequest.erase(request.rawRequest.begin(), request.rawRequest.begin() + request.rawRequest.find("\r\n\r\n") + 4);
	return (true);
}

static void	parseBody(HttpRequest& request)
{
	if (request.contentLength == request.splitRequest.back().size() + request.body.size())
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
		size_t index = request.path.find_last_of('.');
		if (index != std::string::npos)
		{
			request.fileType = request.path.substr(index);
		}
	}
}

bool	WebServer::handleRequest(Client* client, int clientFd)
{
	ssize_t			readBytes;
	std::string		buffer;

	assert(client != nullptr);
	HttpRequest&	request = client->getRequest();

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
	client->setPingTime();
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
