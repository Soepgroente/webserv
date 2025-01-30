#include "Client.hpp"

bool	Client::getContentType(size_t i)
{
	request.contentType = request.splitRequest[i].substr(14);
	return (true);
}

bool	Client::getContentLength(size_t i)
{
	request.contentLength = std::stoi(request.splitRequest[i].substr(16));
	if (static_cast<int32_t>(request.contentLength) > server.maxBodySize)
		request.status = requestIsInvalid;
	return (true);
}

bool	Client::getHost(size_t i)
{
	size_t	splitter = request.splitRequest[i].find_last_of(':');

	request.host = request.splitRequest[i].substr(6, splitter - 6);
	request.port = request.splitRequest[i].substr(splitter + 1);
	return (true);
}

bool	Client::getMethods(size_t i)
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
		fileFd = openFile(path.c_str());
		status = readingFromFile;
	}
    else if (std::filesystem::is_directory(path))
	{
		const std::map<std::string, Location>& locations = server.locations;

		if (locations.find(request.path) != locations.end())
		{
			if (locations.at(request.path).directoryListing == true)
        		status = showDirectory;
		}
		else
		{
			request.status = requestIsInvalid;
			return (false);
		}
	}
	return (true);
}

bool	Client::getConnectionType(size_t i)
{
	request.connectionType = request.splitRequest[i].substr(12);
	return (true);
}

bool	Client::getKeepAlive(size_t i)
{
	if (request.connectionType != "Keep-Alive")
	{
		request.status = requestIsInvalid;
		return (false);
	}
	std::string&	req = request.splitRequest[i];
	try
	{
		timeout = std::stoi(req.substr(req.find_first_of("timeout=") + 8, req.find_first_of(',')));
		remainingRequests = std::stoi(req.substr(req.find_first_of("max=") + 4));
	}
	catch (std::exception& e)
	{
		request.status = requestIsInvalid;
		return (false);
	}
	return (true);
}

static void	setDefaultResponse(Client& client, HttpResponse& response)
{
	response.buffer = HttpResponse::defaultResponses.at(client.getClientStatus());
	client.setClientStatus(RESPONDING);
}

bool	Client::parseHeaders()
{
	if (request.status == headerIsParsed)
		return (true);
	if (request.buffer.find("\r\n\r\n") == std::string::npos)
		return (false);

	const std::map<std::string, std::function<bool(Client*, size_t)>> parseFunctions = 
	{
		{"GET", &Client::getMethods},
		{"POST", &Client::getMethods},
		{"DELETE", &Client::getMethods},
		{"Connection:", &Client::getConnectionType},
		{"Keep-Alive:", &Client::getKeepAlive},
		{"Host:", &Client::getHost},
		{"Content-Type:", &Client::getContentType},
		{"Content-Length:", &Client::getContentLength}
	};

	request.splitRequest = stringSplit(request.buffer);
	for (size_t i = 0; i < request.splitRequest.size(); i++)
	{
		std::string firstWord = request.splitRequest[i].substr(0, request.splitRequest[i].find(' '));

		if (parseFunctions.find(firstWord) != parseFunctions.end())
		{
			if (parseFunctions.at(firstWord)(this, i) == false)
			{
				setDefaultResponse(*this, response);
				return (true);
			}
		}
	}
	request.buffer.erase(request.buffer.begin(), request.buffer.begin() + request.buffer.find("\r\n\r\n") + 4);
	request.status = headerIsParsed;
	return (true);
}

static void	parseBody(HttpRequest& request)
{
	if (request.contentLength == 0 && request.buffer.size() > 0)
		request.status = requestIsInvalid;
	else if (request.contentLength == request.buffer.size())
	{
		request.body = request.buffer; // delete the body variable later to not make huge copies unnecessarily
		request.status = bodyIsParsed;
	}
}

bool	Client::requestIsFinished()
{
	if (request.body.size() == request.contentLength) // alex is a scaredy cat and wants to confirm this is ok
	{
		status = RESPONDING;
	}
	if (status == RESPONDING)
	{
		return (true);
	}
	return (false);
}

void	Client::interpretRequest()
{
	if (parseHeaders() == false)
		return ;
	if (requestIsFinished() == false)
		parseBody(request);
	else
	{
		remainingRequests--;
		pollDescriptor->events = POLLOUT;
		if (request.method != "GET")
		{
			status = RESPONDING;
		}
		else
		{
			status = readingFromFile;
		}
		size_t index = request.path.find_last_of('.');
		if (index != std::string::npos)
		{
			request.fileType = request.path.substr(index);
		}
	}
}
