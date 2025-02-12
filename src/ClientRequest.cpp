#include "Client.hpp"

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
		return (server.locations.at(start->first));
	}
	if (tmp->first != "/")
	{
		path = path.substr(tmp->first.size());
		if (path[0] != '/')
		{
			request.status = requestNotFound;
			return (server.locations.at(start->first));
		}
	}
	return (server.locations.at(tmp->first));
}

// location /12 {
// 	methods GET;
// 	directoryListing on;
// 	root /34
// 	index home.html;
// }

// /34/mars.jpg
// /345/mars.jpg

// / + planets/mars.jpg
// /upload/index

// /var/www/html/home.html
// /methodNotAllowed2/mars.jpg
// /var/testDirectories/methodNotAllowed/mars.jpg
// /home.html
// /var/testDirectories/directoryListingOn/home.html

bool	Client::getMethods(const std::string &requestLine)
{
    std::stringstream	stream;
	
	stream.str(requestLine);
	stream >> request.method >> request.path >> request.protocol;

	const Location& location = resolveRequestLocation(request.path);
	if (request.status != defaultStatus && std::find(location.methods.begin(), location.methods.end(), request.method) == location.methods.end())
		request.status = requestMethodNotAllowed;
	if (request.status != defaultStatus)
		return (false);
	/* Try to show index page or directory listing if the request.path == location.first */
	const std::map<std::string, std::string>&	dir = location.dirs;
	if (request.path == "/")
	{
		request.path = request.path + dir.at("index");
	}
	request.path = dir.at("root") + request.path;

	// for GET

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
		remainingRequests = 1;
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
				setupErrorPage(request.status);
				return (true);
			}
		}
	}
	request.body += request.buffer.substr(request.buffer.find("\r\n\r\n") + 4);
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
