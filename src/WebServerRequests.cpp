#include "WebServer.hpp"

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

static bool	getContentType(Client& client, HttpRequest& request, size_t i)
{
	(void)client;
	request.contentType = request.splitRequest[i].substr(14);
	return (true);
}

static bool	getContentLength(Client& client, HttpRequest& request, size_t i)
{
	(void)client;
	request.contentLength = std::stoi(request.splitRequest[i].substr(16));
	// if (request.contentLength > MAXBODYSIZE)
		//do a thing
	return (true);
}

static bool	getHost(Client& client, HttpRequest& request, size_t i)
{
	(void)client;
	size_t	splitter = request.splitRequest[i].find_last_of(':');

	request.host = request.splitRequest[i].substr(6, splitter - 6);
	request.port = request.splitRequest[i].substr(splitter + 1);
	return (true);
}

static bool	getMethods(Client& client, HttpRequest& request, size_t i)
{
	(void)client;
    std::stringstream	stream;

	stream.str(request.splitRequest[i]);
	stream >> request.method >> request.path >> request.protocol;
	return (true);
}

static bool	requestIsFinished(const HttpRequest& request)
{
	if (request.contentLength == 0 || request.body.size() == request.contentLength)
	{
		return (true);
	}
	return (false);
}

static bool	getConnectionType(Client& client, HttpRequest& request, size_t i)
{
	(void)client;
	request.connectionType = request.splitRequest[i].substr(12);
	return (true);
}

static bool	getKeepAlive(Client& client, HttpRequest& request, size_t i)
{
	if (request.connectionType != "Keep-Alive")
	{
		request.status = requestIsInvalid;
		return (false);
	}
	std::string&	req = request.splitRequest[i];
	client.setTimeout(std::stoi(req.substr(req.find_first_of("timeout=") + 8, req.find_first_of(','))));
	client.setRemainingRequests(std::stoi(req.substr(req.find_first_of("max=") + 4)));
	return (true);
}

static bool	parseHeaders(Client& client, HttpRequest& request)
{
	if (request.status == headerIsParsed)
		return (true);
	if (request.rawRequest.find("\r\n\r\n") == std::string::npos)
		return (false);

	std::map<std::string, std::function<bool(Client&, HttpRequest&, size_t)>> parseFunctions = 
	{
		{"GET", &getMethods},
		{"POST", &getMethods},
		{"DELETE", &getMethods},
		{"Connection:", &getConnectionType},
		{"Keep-Alive:", &getKeepAlive},
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
			if (parseFunctions[firstWord](client, request, i) == false)
			{
				// set isValidRequest to false & respond with error message
				return (false);
			}
		}
	}
	request.rawRequest.erase(request.rawRequest.begin(), request.rawRequest.begin() + request.rawRequest.find("\r\n\r\n") + 4);
	request.status = headerIsParsed;
	return (true);
}

static void	parseBody(HttpRequest& request)
{
	if (request.contentLength == request.rawRequest.size())
	{
		request.status = bodyIsParsed;
		request.body = request.rawRequest; // delete the body variable later to not make huge copies unnecessarily
	}
}

void	WebServer::interpretRequest(Client& client, HttpRequest& request, int clientFd)
{
	if (parseHeaders(client, request) == false)
		return ;
	if (requestIsFinished(request) == false)
		parseBody(request);
	else
	{
		assert(getPollfdIndex(clientFd) != -1);
		client.receivedRequest();
		pollDescriptors[getPollfdIndex(clientFd)].events = POLLOUT;
		size_t index = request.path.find_last_of('.');
		if (index != std::string::npos)
		{
			request.fileType = request.path.substr(index);
		}
	}
}

bool	WebServer::handleRequest(Client& client, int clientFd)
{
	ssize_t			readBytes;
	std::string		buffer;
	HttpRequest&	request = client.getRequest();

	buffer.resize(BUFFERSIZE);
	readBytes = read(clientFd, buffer.data(), BUFFERSIZE);
	if (readBytes == -1)
	{
		throw std::runtime_error("Error reading from client_fd");
	}
	if (readBytes == 0) // does poll do this to us?
	{
		closeConnection(clientFd);
		return (false);
	}
	buffer.resize(readBytes);
	client.setPingTime();
	request.rawRequest += buffer;
	interpretRequest(client, request, clientFd);
	if (request.fileType == ".out")
	{
		client.setCgiStatus(launchCgi);
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
