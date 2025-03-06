#include "Client.hpp"

Location*	Client::resolveRequestLocation(std::string& path)
{
	using MapIterator = std::map<std::string, Location>::const_iterator;

	MapIterator	start = server->locations.begin();
	MapIterator	end = server->locations.end();
	MapIterator	tmp = end;

	for (MapIterator it = start; it != end; it++)
	{
		if (path.find(it->first) == 0)
			tmp = it;
	}
	if (tmp == end || std::filesystem::exists('.' + server->locations.at(tmp->first).dirs.at("root")) == false)
	{
		request.status = requestNotFound;
		request.locationPath = start->first;
		return (const_cast<Location*>(&(server->locations.at(start->first))));
	}
	if (tmp->first != "/")
	{
		
		path = path.substr(tmp->first.size());
		if (path.size() != 0 && path[0] != '/')
		{
			request.status = requestNotFound;
			request.locationPath = start->first;
			return (const_cast<Location*>(&(server->locations.at(start->first))));
		}
	}
	request.locationPath = tmp->first;
	return (const_cast<Location*>(&(tmp->second)));
}

static void restoreWhitespace(std::string& path)
{
	size_t position = path.find("%20");

	while (position != std::string::npos)
	{
		path.replace(position, 3, " ");
		position = path.find("%20");
	}
}

/*	Sets up the correct path for the next step, shows index if no particular path	*/

bool	Client::parsePath(const std::string& requestLine)
{
    std::stringstream	stream;
	
	stream.str(requestLine);
	stream >> request.method >> request.path >> request.protocol;

	if (request.protocol != "HTTP/1.1")
	{
		request.status = versionNotSupported;
		return (false);
	}
	restoreWhitespace(request.path);
	request.location = resolveRequestLocation(request.path);
	if (request.location == nullptr)
	{
		request.status = internalServerError;
		return (false);
	}
	response.location = request.location;
	if (request.status == defaultStatus && std::find(request.location->methods.begin(), request.location->methods.end(), request.method) == request.location->methods.end())
		request.status = requestMethodNotAllowed;
	if (request.status != defaultStatus)
		return (false);
	const std::map<std::string, std::string>&	dir = request.location->dirs;
	if (request.path == "/")
	{
		request.path = request.path + dir.at("index");
	}
	request.path = dir.at("root") + request.path;
	request.dotPath = "." + request.path;
	return (true);
}

bool	Client::parseGet(const std::string &requestLine)
{
	if (parsePath(requestLine) == false)
		return (false);

	if (request.location->dirs.at("redirection").empty() == false)
	{
		request.status = temporaryRedirect;
		status = redirection;
		return (false);
	}
	if (std::filesystem::exists(request.dotPath) == false)
	{
		request.status = requestNotFound;
		return (false);
	}
    if (std::filesystem::is_directory(request.dotPath) == true)
	{
		if (request.location->dirs.at("directory_listing") == "on")
		{
			status = showDirectory;
			if (request.dotPath.back() != '/')
				request.dotPath += "/";
		}
		else
		{
			request.status = requestForbidden;
			return (false);
		}
	}
	else if (std::filesystem::is_regular_file(request.dotPath) == true)
	{
		status = readingFromFile;
	}
	return (true);
}

bool	Client::parsePost(const std::string& requestLine)
{
	if (parsePath(requestLine) == false)
		return (false);
	if (std::filesystem::exists(request.dotPath) == true)
	{
		request.status = fileAlreadyExists;
		return (false);
	}
	return (true);
}

bool	Client::parseDelete(const std::string& requestLine)
{
	if (parsePath(requestLine) == false)
		return (false);
	if (std::filesystem::exists(request.dotPath) == false)
	{
		request.status = requestNotFound;
		return (false);
	}
	if (std::filesystem::remove(request.dotPath) == false)
	{
		request.status = requestForbidden;
		return (false);
	}
	status = RESPONDING;
	response.constructResponse(requestIsOk, "text/plain", 0);
	return (true);
}
