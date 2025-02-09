#include "Client.hpp"

static void	setDefaultResponse(Client& client, HttpResponse& response)
{
	if (HttpResponse::defaultResponses.find(client.getRequest().status) != HttpResponse::defaultResponses.end())
	{
		response.buffer = HttpResponse::defaultResponses.at(client.getRequest().status);
		client.setClientStatus(RESPONDING);
	}
}

bool	Client::getContentType(const std::string& requestLine)
{
	request.contentType = requestLine.substr(14);
	return (true);
}

bool	Client::getContentLength(const std::string& requestLine)
{
	request.contentLength = std::stoi(requestLine.substr(16));
	if (static_cast<int32_t>(request.contentLength) > server.maxBodySize)
		request.status = requestIsInvalid;
	return (true);
}

bool	Client::getHost(const std::string& requestLine)
{
	size_t	splitter = requestLine.find_last_of(':');

	request.host = requestLine.substr(6, splitter - 6);
	request.port = requestLine.substr(splitter + 1);
	return (true);
}

bool	Client::getChunked(const std::string& requestLine)
{
	if (requestLine != "Transfer-Encoding: chunked")
	{
		request.status = requestIsInvalid;
		return (false);
	}
	request.chunked = true;
	return (true);
}

const Location&	Client::resolveRequestLocation(std::string& path)
{
	auto	start = getServer().locations.begin();
	auto	end = getServer().locations.end();
	auto	tmp = start;
	for (auto it = start; it != end; it++)
	{
		if (path.find(it->first) == 0)
			tmp = it;
	}
	if (tmp == getServer().locations.end())
		request.status = requestNotFound;
	if (tmp != start && tmp != end)
		tmp--;
	size_t pos = path.find_first_not_of(tmp->first);
	if (pos != std::string::npos)
		path = path.substr(pos - 1);
	return getServer().locations.at(tmp->first);
}

bool	Client::getMethods(const std::string &requestLine)
{
    std::stringstream	stream;
	

	stream.str(requestLine);
	stream >> request.method >> request.path >> request.protocol;

	const Location& location = resolveRequestLocation(request.path); // Perhasps needs more work
	if (std::find(location.methods.begin(), location.methods.end(), request.method) == location.methods.end())
		request.status = requestMethodNotAllowed;
	if (request.status != 0)
		return (false);
	/* Try to show index page or directory listing if the request.path == location.first */
	std::map<std::string, std::string>	dir = location.dirs;
	if (request.path == "/")
	{
		request.path = request.path + dir.at("index");
	}
	request.path = dir.at("root") + request.path;
	const std::filesystem::path path = '.' + request.path;
	if (std::filesystem::exists(path) == false)
	{
		request.status = requestNotFound;
		return (false);
	}
	if (std::filesystem::is_regular_file(path))
	{
		fileFd = openFile(path.c_str(), Client::fileAndCgiDescriptors);
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
			request.status = requestForbidden;
			return (false);
		}
	}
	return (true);
}

bool	Client::getConnectionType(const std::string& requestLine)
{
	request.connectionType = requestLine.substr(12);
	if (request.connectionType == "Close")
		this->remainingRequests = 1;
	return (true);
}

bool	Client::getKeepAlive(const std::string& requestLine)
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
	if (request.status == headerIsParsed)
		return (true);
	if (request.buffer.find("\r\n\r\n") == std::string::npos)
		return (false);

	const std::map<std::string, std::function<bool(Client*, const std::string&)>> parseFunctions = 
	{
		{"GET", &Client::getMethods},
		{"POST", &Client::getMethods},
		{"DELETE", &Client::getMethods},
		{"Connection:", &Client::getConnectionType},
		{"Transfer-Encoding:", &Client::getChunked},
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
			if (parseFunctions.at(firstWord)(this, request.splitRequest[i]) == false)
			{
				setDefaultResponse(*this, response);
				return (true);
			}
		}
	}
	request.body += request.buffer.substr(request.buffer.find("\r\n\r\n") + 4);
	request.status = headerIsParsed;
	return (true);
}

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

static void	parseBody(HttpRequest& request)
{
	if (request.chunked == true && request.buffer.find(CHUNKED_EOF) != std::string::npos)
	{
		request.status = decodeChunks(request.buffer, request.body);
	}
	else if (request.contentLength == 0)
	{
		if (request.body.size() == 0)
			request.status = bodyIsParsed;
		else
			request.status = requestIsInvalid;
	}
	else if (request.contentLength == request.buffer.size())
	{
		request.status = bodyIsParsed;
	}
}

void	Client::interpretRequest()
{
	if (parseHeaders() == false)
		return ;
	parseBody(request);
	if (request.status == bodyIsParsed)
	{
		remainingRequests--;
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
		if (request.fileType == ".cgi")
		{
			status = launchCgi;
		}
		status = readingFromFile;
		puts("Status is readingfromfile");
	}
}
