#include "Client.hpp"

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

static void	restoreWhitespace(std::string& path)
{
	size_t	position = path.find("%20");
	while (position != std::string::npos)
	{
		path.replace(position, 3, " ");
		position = path.find("%20");
	}
}

bool	Client::parsePath(const std::string& requestLine)
{
    std::stringstream	stream;
	
	stream.str(requestLine);
	stream >> request.method >> request.path >> request.protocol;
	restoreWhitespace(request.path); // this can become a function for all the encodings
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
		response.reply = HttpResponse::defaultResponses[requestIsOk];
		return (true);
	}
	else
	{
		request.status = requestNotFound;
		return (false);
	}
}
