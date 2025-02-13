#include "Client.hpp"

bool	Client::parseContentType(const std::string& requestLine)
{
	request.contentType = requestLine.substr(14);
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

Location*	Client::resolveRequestLocation(std::string& path)
{
	using MapIterator = std::map<std::string, Location>::const_iterator;

	MapIterator	start = server.locations.begin();
	MapIterator	end = server.locations.end();
	MapIterator	tmp = end;

	for (MapIterator it = start; it != end; it++)
	{
		if (path.find(it->first) == 0)
			tmp = it;
	}
	if (tmp == end)
	{
		request.status = requestNotFound;
		request.locationPath = start->first;
		return (const_cast<Location*>(&(server.locations.at(start->first))));
	}
	if (tmp->first != "/")
	{
		
		path = path.substr(tmp->first.size());
		if (path.size() != 0 && path[0] != '/')
		{
			request.status = requestNotFound;
			request.locationPath = start->first;
			return (const_cast<Location*>(&(server.locations.at(start->first))));
		}
	}
	request.locationPath = tmp->first;
	return (const_cast<Location*>(&(tmp->second)));
}
/*	Sets up the correct path for the next step, shows index if no particular path	*/

bool	Client::parsePath(const std::string& requestLine)
{
    std::stringstream	stream;
	
	stream.str(requestLine);
	stream >> request.method >> request.path >> request.protocol;

	request.location = resolveRequestLocation(request.path);
	const Location& location = *request.location;
	if (request.status == defaultStatus && std::find(location.methods.begin(), location.methods.end(), request.method) == location.methods.end())
		request.status = requestMethodNotAllowed;
	if (request.status != defaultStatus)
		return (false);
	const std::map<std::string, std::string>&	dir = location.dirs;
	if (request.path == "/")
	{
		request.path = request.path + dir.at("index");
	}
	request.path = dir.at("root") + request.path;
	return (true);
}

bool	Client::parseGet(const std::string &requestLine)
{
	if (parsePath(requestLine) == false)
		return (false);
	const std::filesystem::path path = '.' + request.path;

	if (std::filesystem::exists(path) == false)
	{
		request.status = requestNotFound;
		return (false);
	}
	// std::cout << path << std::endl;
    if (std::filesystem::is_directory(path))
	{
		if (request.location->directoryListing == true)
		{
			status = showDirectory;
			request.path = path;
			if (request.path.back() != '/')
				request.path += "/"; // check this as well
		}
		else
		{
			request.status = requestForbidden;
			return (false);
		}
	}
	else if (std::filesystem::is_regular_file(path))
	{
		fileFd = openFile(path.c_str(), O_RDONLY, POLLIN, Client::fileAndCgiDescriptors);
		status = readingFromFile;
	}
	return (true);
}

bool	Client::parsePost(const std::string& requestLine)
{
	if (parsePath(requestLine) == false)
		return (false);
	const std::filesystem::path path = '.' + request.path;

	if (std::filesystem::exists(path) == true)
	{
		request.status = fileAlreadyExists;
		return (false);
	}
	if (request.contentType.find("multipart/form-data", 0) == 0)
	{
		size_t	index = request.contentType.find("boundary=", 0);

		if (index == std::string::npos)
		{
			request.status = requestIsInvalid;
			return (false);
		}
		request.boundary = "\r\n--" + request.contentType.substr(index + 9);
	}
	fileFd = openFile(path.c_str(), O_WRONLY | O_CREAT, POLLOUT, Client::fileAndCgiDescriptors);
	return (true);
}

bool	Client::parseDelete(const std::string& requestLine)
{
	if (parsePath(requestLine) == false)
		return (false);
	const std::filesystem::path path = '.' + request.path;

	if (std::filesystem::exists(path) == true)
	{
		if (std::filesystem::remove(path) == false)
		{
			request.status = requestForbidden;
			return (false);
		}
		status = RESPONDING;
		response.reply = HttpResponse::defaultResponses[200];
		return (true);
	}
	else
	{
		request.status = requestNotFound;
		return (false);
	}
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
	if (request.status == headerIsParsed)
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
				return (true);
			}
		}
	}
	request.body += request.buffer.substr(request.buffer.find(EMPTY_LINE) + 4);
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
	size_t	closingBoundary = request.body.find("\r\n" + request.boundary + "--");

	if (closingBoundary == std::string::npos)
	{
		request.body += request.buffer;
		request.buffer.clear();
		return ;
	}
	if (request.body.find(request.boundary, 0) != 0)
	{
		request.status = requestIsInvalid;
		return ;
	}
	size_t index = request.body.find_first_of(std::string(EMPTY_LINE)) + 4;
	request.body.erase(0, index);
	request.body.erase(closingBoundary - index);
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
			request.status = requestIsInvalid;
		}
	}
	else if (request.body.size() == request.contentLength && request.boundary.empty() == true)
	{
		request.status = bodyIsParsed;
	}		
	else if (request.body.size() > request.contentLength)
	{
		if (request.boundary.empty() == true)
			request.status = requestIsInvalid;
		else
			trimMultipart(request);
	}
	else
	{
		request.body += request.buffer;
		request.buffer.clear();
	}
}

void	Client::interpretRequest()
{
	if (parseHeaders() == false)
		return ;
	parseBody(request);
	if (request.status == requestIsInvalid)
	{
		std::cout << "INVALID\n" << request << std::endl;
		setupErrorPage(request.status);
		return ;
	}
	if (request.status == bodyIsParsed)
	{
		std::cout << "VALID\n" << request << std::endl;
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
