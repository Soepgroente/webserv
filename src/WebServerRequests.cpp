#include "WebServer.hpp"

#define BUFFERSIZE 8 * 1024

void	WebServer::closeConnection(int fd)
{
	close(fd);
	requests.erase(fd);
	pollDescriptors.erase(pollDescriptors.begin() + getPollfdIndex(fd)); // this can go out of bounds if we don't have this fd stored
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
	parseHeaders(request);
	parseBody(request);
	if (requestIsFinished(request) == true)
	{
		pollDescriptors[getPollfdIndex(clientFd)].events = POLLOUT;
	}
}

void	WebServer::handleClientRead(int clientFd)
{
	ssize_t		readBytes;
	std::string	buffer;

	if (timeout(requests[clientFd].lastRead) == true)
	{
		closeConnection(clientFd);
		return ;
	}
	buffer.resize(BUFFERSIZE);
	readBytes = read(clientFd, &buffer[0], BUFFERSIZE);
	if (readBytes == -1)
	{
		if (errno != EWOULDBLOCK)
		{
			std::cerr << "Client_fd read error" << std::endl;
			// handleError(strerror(errno));
		}
		else
		{
			std::cerr << "Error number: " << errno << " Error msg: " << strerror(errno) << std::endl;
			errorExit("Read failed", -1);
		}
	}
	else if (readBytes == 0)
	{
		closeConnection(clientFd);
		return ;
	}
	buffer.resize(readBytes);
	requests[clientFd].lastRead = getTime();
	requests[clientFd].rawRequest += buffer;
	interpretRequest(requests[clientFd], clientFd);
}
