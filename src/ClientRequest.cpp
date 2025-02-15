#include "Client.hpp"

bool	Client::parseContentType(const std::string& requestLine)
{
	request.contentType = requestLine.substr(sizeof("Content-Type: ") - 1);
	if (request.contentType.find("multipart/form-data", 0) == 0)
	{
		size_t	index = request.contentType.find("boundary=", 0);

		if (index == std::string::npos)
		{
			request.status = requestIsInvalid;
			return (false);
		}
		request.boundary = request.contentType.substr(index + 9);
	}
	std::cout << request << std::endl;
	return (true);
}

bool	Client::parseContentLength(const std::string& requestLine)
{
	request.contentLength = std::stoi(requestLine.substr(16));
	if (static_cast<int32_t>(request.contentLength) > server.maxBodySize)
	{
		request.status = payloadTooLarge;
		return (false);
	}
	return (true);
}

bool	Client::parseHost(const std::string& requestLine)
{
	size_t	splitter = requestLine.find_last_of(':');

	request.host = requestLine.substr(6, splitter - 6);
	request.port = requestLine.substr(splitter + 1);
	return (true);
}

bool	Client::parseChunked(const std::string& requestLine)
{
	if (requestLine != "Transfer-Encoding: chunked")
	{
		request.status = requestIsInvalid;
		return (false);
	}
	request.chunked = true;
	return (true);
}

bool	Client::parseConnectionType(const std::string& requestLine)
{
	request.connectionType = requestLine.substr(12);
	if (request.connectionType == "Close")
		remainingRequests = 1;
	return (true);
}

bool	Client::parseKeepAlive(const std::string& requestLine)
{
	if (request.connectionType != "Keep-Alive")
	{
		request.status = requestIsInvalid;
		return (false);
	}
	try
	{
		timeout = std::stoi(requestLine.substr(requestLine.find_first_of("timeout=") + 8, requestLine.find_first_of(',')));
		remainingRequests = std::stoi(requestLine.substr(requestLine.find_first_of("max=") + 4));
	}
	catch (std::exception& e)
	{
		request.status = requestIsInvalid;
		return (false);
	}
	return (true);
}

bool	Client::parseHeaders()
{
	if (request.status == headerIsParsed || request.status == bodyIsParsed)
		return (true);
	if (request.buffer.find(EMPTY_LINE) == std::string::npos)
		return (false);

	const std::map<std::string, std::function<bool(Client*, const std::string&)>> parseFunctions = 
	{
		{"GET", &Client::parseGet},
		{"POST", &Client::parsePost},
		{"DELETE", &Client::parseDelete},
		{"Connection:", &Client::parseConnectionType},
		{"Transfer-Encoding:", &Client::parseChunked},
		{"Keep-Alive:", &Client::parseKeepAlive},
		{"Host:", &Client::parseHost},
		{"Content-Type:", &Client::parseContentType},
		{"Content-Length:", &Client::parseContentLength}
	};

	request.splitRequest = stringSplit(request.buffer);
	for (size_t i = 0; i < request.splitRequest.size(); i++)
	{
		std::string firstWord = request.splitRequest[i].substr(0, request.splitRequest[i].find(' '));

		if (parseFunctions.find(firstWord) != parseFunctions.end())
		{
			if (parseFunctions.at(firstWord)(this, request.splitRequest[i]) == false)
			{
				setupErrorPage(request.status);
				return (false);
			}
		}
	}
	request.body += request.buffer.substr(request.buffer.find(EMPTY_LINE) + 4);
	if (request.boundary.empty() == false)
	{
		if (request.body.find(request.boundary, 0) != 0)
		{
			setupErrorPage(requestIsInvalid);
			return (false);
		}
	}
	request.buffer.clear();
	request.status = headerIsParsed;
	return (true);
}

/*	Validates the chunks and unchunks, or returns invalid request	*/

static int	decodeChunks(std::string& buffer, std::string& body)
{
	while (buffer.empty() == false)
	{
		size_t		chunkSize;
		std::string	newChunk;
		try
		{
			chunkSize = std::stoi(buffer.substr(0, buffer.find_first_of("\r\n")), nullptr, 16);
		}
		catch (std::exception& e)
		{
			return (requestIsInvalid);
		}
		buffer.erase(0, buffer.find_first_of("\r\n") + 2);
		newChunk = buffer.substr(0, chunkSize);
		buffer.erase(0, chunkSize);
		if (buffer.compare(0, 2, "\r\n") != 0)
			return (requestIsInvalid);
		buffer.erase(0, 2);
		body += newChunk;
	}
	return (bodyIsParsed);
}

/*	Checks whether the full request has been received	*/

static void	trimMultipart(HttpRequest& request)
{
	size_t	closingBoundary = request.body.find(std::string("--") + request.boundary + "--" + EMPTY_LINE, request.body.size() - request.boundary.size() - 8);

	if (closingBoundary == std::string::npos || request.body.find(std::string("--") + request.boundary, 0) != 0)
	{
		request.status = requestIsInvalid;
		return ;
	}
	puts("before");
	std::cout << request.body << std::endl;
	request.body.erase(0, request.body.find_first_of(EMPTY_LINE) + 4);
	puts("first chomp");
	std::cout << request.body << std::endl;
	request.body.erase(closingBoundary);
	puts("second chomp");
	std::cout << request.body << std::endl;
	request.status = bodyIsParsed;
}

static void	parseBody(HttpRequest& request)
{
	if (request.chunked == true && request.buffer.find(CHUNKED_EOF) != std::string::npos)
	{
		request.status = decodeChunks(request.buffer, request.body);
	}
	else if (request.contentLength == 0)
	{
		if (request.body.empty() == true)
		{
			request.status = bodyIsParsed;
		}
		else
		{
			request.status = lengthRequired;
		}
	}
	else if (request.body.size() == request.contentLength)
	{
		std::cout << "boundary size: " << request.boundary.size();
		if (request.boundary.empty() == true)
		{
			std::cout << "boundary size: " << request.boundary.size();
			request.status = bodyIsParsed;
		}
		else
			trimMultipart(request);
	}
	else
	{
		request.body += request.buffer;
		request.buffer.clear();
	}
	if (request.body.size() > request.contentLength)
	{
		request.status = requestIsInvalid;
	}
}

void	Client::interpretRequest()
{
	if (parseHeaders() == false)
		return ;
	std::cout << request << std::endl;
	parseBody(request);
	if (request.status == requestIsInvalid)
	{
		setupErrorPage(request.status);
		return ;
	}
	if (request.status == bodyIsParsed)
	{
		remainingRequests--;
		if (request.method == "GET" && status != showDirectory)
			status = readingFromFile;
		if (request.method == "POST")
		{
			status = writingToFile;
		}
		size_t index = request.path.find_last_of('.');
		if (index != std::string::npos)
		{
			request.fileType = request.path.substr(index);
		}
		if (request.fileType == ".cgi")
		{
			status = launchCgi;
		}
	}
}
