#include "WebServer.hpp"

#define BUFFERSIZE 8 * 1024

void	WebServer::handleClientWrite(int clientFd)
{
	(void)clientFd;
}

void	WebServer::closeConnection(int fd)
{
	std::vector<pollfd>::iterator it;
	
	close(fd);
	for (it = pollDescriptors.begin(); it != pollDescriptors.end(); it++)
	{
		if (it->fd == fd)
		{
			pollDescriptors.erase(it);
			break ;
		}
	}
}

bool	getContentType(HttpRequest& req, size_t i)
{
	if (req.splitRequest[i].size() <= 14)
		return (false);
	if (req.splitRequest[i].substr(0, 14) != "Content-Type: ")
		return (false);
	req.contentType = req.splitRequest[i].substr(14);
	return (true);
}

bool	getContentLength(HttpRequest& req, size_t i)
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

bool	getHost(HttpRequest& req, size_t i)
{
	size_t	splitter = req.splitRequest[i].find_last_of(':');

	req.host = req.splitRequest[i].substr(6, splitter - 6);
	req.port = req.splitRequest[i].substr(splitter + 1);
	return (true);
}

bool	getMethods(HttpRequest& req, size_t i)
{
    std::stringstream	stream;

	stream.str(req.splitRequest[i]);
	stream >> req.method >> req.path >> req.protocol;
	return (true);
}

void	WebServer::interpretRequest(HttpRequest& request)
{
	Server		server;
	int			count = 0;
	bool (*func[4])(HttpRequest&, size_t) = {&getMethods, &getHost, &getContentType, &getContentLength};
	
	request.splitRequest = stringSplit(request.rawRequest);
	for (size_t i = 0; i < request.splitRequest.size() && count < 4; i++)
	{
		count += func[count](request, i);
	}
	std::cout << request;
	exit(0);
	// GET / HTTP/1.1
	// Host: 127.0.0.1:8080
	// Connection: keep-alive
	// Cache-Control: max-age=0
	// sec-ch-ua: "Google Chrome";v="131", "Chromium";v="131", "Not_A Brand";v="24"
	// sec-ch-ua-mobile: ?0
	// sec-ch-ua-platform: "Linux"
	// Upgrade-Insecure-Requests: 1
	// User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36
	// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
	// Sec-Fetch-Site: none
	// Sec-Fetch-Mode: navigate
	// Sec-Fetch-User: ?1
	// Sec-Fetch-Dest: document
	// Accept-Encoding: gzip, deflate, br, zstd
	// Accept-Language: en-US,en;q=0.9\r\n
	// \r\n
}

void	WebServer::handleClientRead(int clientFd)
{
	ssize_t		readBytes;
	std::string	buffer;

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
