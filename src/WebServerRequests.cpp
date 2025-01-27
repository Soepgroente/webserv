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
		if (clients[i].getFd() == fileDes || clients[i].getCgiFd() == fileDes || clients[i].getFileFd() == fileDes)
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

bool	WebServer::getContentType(Client& client, HttpRequest& request, size_t i)
{
	(void)client;
	request.contentType = request.splitRequest[i].substr(14);
	return (true);
}

bool	WebServer::getContentLength(Client& client, HttpRequest& request, size_t i)
{
	(void)client;
	request.contentLength = std::stoi(request.splitRequest[i].substr(16));
	if (static_cast<int32_t>(request.contentLength) > client.getServer().maxBodySize)
		request.status = requestIsInvalid;
	return (true);
}

bool	WebServer::getHost(Client& client, HttpRequest& request, size_t i)
{
	(void)client;
	size_t	splitter = request.splitRequest[i].find_last_of(':');

	request.host = request.splitRequest[i].substr(6, splitter - 6);
	request.port = request.splitRequest[i].substr(splitter + 1);
	return (true);
}

bool	WebServer::getMethods(Client& client, HttpRequest& request, size_t i)
{
    std::stringstream	stream;

	stream.str(request.splitRequest[i]);
	stream >> request.method >> request.path >> request.protocol;

	const std::filesystem::path path = request.path;
	if (std::filesystem::exists(path) == false)
	{
		request.status = requestNotFound;
		return (false);
	}
	if (std::filesystem::is_regular_file(path))
	{
		openFile(path.c_str());
		client.setClientStatus(readingFromFile);
	}
    else if (std::filesystem::is_directory(path))
	{
		const std::map<std::string, Location>& locations = client.getServer().locations;

		if (locations.find(request.path) != locations.end())
		{
			if (locations.at(request.path).directoryListing == true)
        		client.setClientStatus(showDirectory);
		}
		else
		{
			request.status = requestIsInvalid;
			return (false);
		}
	}
	return (true);
}

static bool	requestIsFinished(Client& client, const HttpRequest& request)
{
	if (request.body.size() == request.contentLength) // alex is a scaredy cat and wants to confirm this is ok
	{
		client.setClientStatus(clientShouldRespond);
	}
	if (client.getClientStatus() == clientShouldRespond)
	{
		return (true);
	}
	return (false);
}

bool	WebServer::getConnectionType(Client& client, HttpRequest& request, size_t i)
{
	(void)client;
	request.connectionType = request.splitRequest[i].substr(12);
	return (true);
}

bool	WebServer::getKeepAlive(Client& client, HttpRequest& request, size_t i)
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

static void	setDefaultResponse(Client& client, HttpResponse& response)
{
	response.buffer = HttpResponse::defaultResponses.at(client.getClientStatus());
	client.setClientStatus(clientShouldRespond);
}

bool	WebServer::parseHeaders(Client& client, HttpRequest& request)
{
	if (request.status == headerIsParsed)
		return (true);
	if (request.rawRequest.find("\r\n\r\n") == std::string::npos)
		return (false);

	const std::map<std::string, std::function<bool(WebServer*, Client&, HttpRequest&, size_t)>> parseFunctions = 
	{
		{"GET", &WebServer::getMethods},
		{"POST", &WebServer::getMethods},
		{"DELETE", &WebServer::getMethods},
		{"Connection:", &WebServer::getConnectionType},
		{"Keep-Alive:", &WebServer::getKeepAlive},
		{"Host:", &WebServer::getHost},
		{"Content-Type:", &WebServer::getContentType},
		{"Content-Length:", &WebServer::getContentLength}
	};
	request.splitRequest = stringSplit(request.rawRequest);
	for (size_t i = 0; i < request.splitRequest.size(); i++)
	{
		std::string firstWord = request.splitRequest[i].substr(0, request.splitRequest[i].find(' '));

		if (parseFunctions.find(firstWord) != parseFunctions.end())
		{
			if (parseFunctions.at(firstWord)(this, client, request, i) == false)
			{
				setDefaultResponse(client, client.getResponse());
				return (true);
			}
		}
	}
	request.rawRequest.erase(request.rawRequest.begin(), request.rawRequest.begin() + request.rawRequest.find("\r\n\r\n") + 4);
	request.status = headerIsParsed;
	return (true);
}

static void	parseBody(HttpRequest& request)
{
	if (request.contentLength == 0 && request.rawRequest.size() > 0)
		request.status = requestIsInvalid;
	else if (request.contentLength == request.rawRequest.size())
	{
		request.body = request.rawRequest; // delete the body variable later to not make huge copies unnecessarily
		request.status = bodyIsParsed;
	}
}

void	WebServer::interpretRequest(Client& client, HttpRequest& request, int clientFd)
{
	if (parseHeaders(client, request) == false)
		return ;
	if (requestIsFinished(client, request) == false)
		parseBody(request);
	else
	{
		assert(getPollfdIndex(clientFd) != -1);
		client.receivedRequest();
		pollDescriptors[getPollfdIndex(clientFd)].events = POLLOUT;
		if (request.method != "GET")
		{
			client.setClientStatus(clientShouldRespond);
		}
		else
		{
			client.setClientStatus(readingFromFile);
		}
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
		puts("Readbytes == 0");
		closeConnection(clientFd);
		return (false);
	}
	buffer.resize(readBytes);
	request.rawRequest += buffer;
	interpretRequest(client, request, clientFd);
	if (request.status == requestIsInvalid)
	{
		client.setClientStatus(clientShouldRespond);
	}
	if (request.fileType == ".cgi")
	{
		client.setClientStatus(launchCgi);
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
		throw std::runtime_error("Error reading from client_fd parsing CGI output");
	}
	client.getResponse().buffer += buffer;
}
