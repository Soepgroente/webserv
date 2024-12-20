#include "WebServer.hpp"

#define BUFFERSIZE 8 * 1024

void	WebServer::closeConnection(int fd)
{
	close(fd);
	requests.erase(fd);
	for (size_t i = 0; i < pollDescriptors.size(); i++)
	{
		if (pollDescriptors[i].fd == fd)
		{
			pollDescriptors.erase(pollDescriptors.begin() + i);
			break ;
		}
	}
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

static void	parseHeaders(HttpRequest& request)
{
	Server		server;
	int			count = 0;
	bool (*func[4])(HttpRequest&, size_t) = {&getMethods, &getHost, &getContentType, &getContentLength};
	
	if (request.rawRequest.find("\r\n\r\n") == std::string::npos || request.host.size() != 0)
		return ;
	request.splitRequest = stringSplit(request.rawRequest);
	for (size_t i = 0; i < request.splitRequest.size() && count < 4; i++)
	{
		count += func[count](request, i);
	}
	request.rawRequest.erase(request.rawRequest.begin(), request.rawRequest.begin() + request.rawRequest.find("\r\n\r\n") + 4);
	request.headerIsParsed = true;
}

static void	parseBody(HttpRequest& request)
{
	if (request.contentLength == request.splitRequest.back().size())
		request.body += request.splitRequest.back();
	// request.rawRequest.erase(request.rawRequest.begin(), request.rawRequest.end());
}

void	WebServer::interpretRequest(HttpRequest& request)
{
	parseHeaders(request);
	if (request.headerIsParsed == false)
		return ;
	parseBody(request);
	if (requestIsFinished(request) == false)
		return ;
	
	// std::cout << request;
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
	interpretRequest(requests[clientFd]);
}

// POST /Server_1_uploads/new_upload HTTP/1.1
// Host: 127.0.0.1:8080
// User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:132.0) Gecko/20100101 Firefox/132.0
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
// Accept-Language: en-US,en;q=0.5
// Accept-Encoding: gzip, deflate, br, zstd
// Content-Type: application/x-www-form-urlencoded
// Content-Length: 250
// Origin: http://127.0.0.1:8080
// DNT: 1
// Connection: keep-alive
// Referer: http://127.0.0.1:8080/postMethod.html
// Upgrade-Insecure-Requests: 1
// Sec-Fetch-Dest: document
// Sec-Fetch-Mode: navigate
// Sec-Fetch-Site: same-origin
// Sec-Fetch-User: ?1
// Priority: u=0, i


// POST /Server_1_uploads/new_upload HTTP/1.1
// Host: 127.0.0.1:8080
// User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:132.0) Gecko/20100101 Firefox/132.0
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
// Accept-Language: en-US,en;q=0.5
// Accept-Encoding: gzip, deflate, br, zstd
// Content-Type: application/x-www-form-urlencoded
// Content-Length: 11
// Origin: http://127.0.0.1:8080
// DNT: 1
// Connection: keep-alive
// Referer: http://127.0.0.1:8080/postMethod.html
// Upgrade-Insecure-Requests: 1
// Sec-Fetch-Dest: document
// Sec-Fetch-Mode: navigate
// Sec-Fetch-Site: same-origin
// Sec-Fetch-User: ?1
// Priority: u=0, i