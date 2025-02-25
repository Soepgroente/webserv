#include "Client.hpp"

/*	Writes whichever amount is smaller between BUFFERSIZE and remaining response size to client.
	Clears the request & response structs once everything has been sent	*/

void	Client::writeToClient()
{
	ssize_t	writtenBytes;

	writtenBytes = write(fd, &response.reply[writePos], \
		std::min(response.reply.size() - writePos, (size_t)BUFFERSIZE));
	if (writtenBytes <= 0)
	{
		writePos = 0;

		// std::cout << response.reply.substr(0, 250) << std::endl;
		reset();
		if (writtenBytes == -1)
		{
			printToLog("Error writing to client fd: " + std::string(strerror(errno)));
			setupErrorPage(internalServerError);
		}
	}
	else
		writePos += writtenBytes;
}

void	Client::writeToFile()
{
	ssize_t	writtenBytes;

	writtenBytes = write(fileFd, &request.body[writePos], \
		std::min(request.body.size() - writePos, (size_t)BUFFERSIZE));
	if (writtenBytes < BUFFERSIZE)
	{
		writePos = 0;
		closeAndResetFd(Client::fileAndCgiDescriptors, fileFd);
		if (writtenBytes == -1)
		{
			printToLog("Error writing to file fd: " + std::string(strerror(errno)));
			setupErrorPage(internalServerError);
			return ;
		}
		status = RESPONDING;
		response.constructResponse(requestCreated, "text/html", 0);
	}
	else
		writePos += writtenBytes;
}

std::string	Client::generateDirectoryListing(const std::filesystem::path& dir)
{
	std::stringstream	ss;
	std::string			dotPath = '/' + dir.parent_path().filename().string();

	if (request.locationPath != dotPath)
	{
		dotPath = request.locationPath + dotPath;
	}
	ss << "<!DOCTYPE html>\n"
    	<< "<html>\n"
        << "<head><meta charset=\"UTF-8\"><title>Directory Listing</title></head>\n"
        << "<body>\n"
        << "<h1>Index of " << dotPath << "</h1>\n"
        << "<ul>\n";
	for (std::filesystem::directory_entry dirEntry : std::filesystem::directory_iterator(dir))
	{
		std::string entry = dirEntry.path().filename().string();
		ss << " <li><a href=\"" << dotPath + '/' << entry << "\">" << entry << "</a></li>\n";
	}
	ss << "</ul>\n"
		<< "</body>\n"
		<< "</html>\n";
	return (ss.str());
}

void	Client::parseDirectory()
{
	std::filesystem::path	dir(request.dotPath);	

	response.buffer = generateDirectoryListing(dir);
	response.constructResponse(requestIsOk, "text/html", response.buffer.size());
	status = RESPONDING;
}

/*	Executes a different method based on the status of the Client	*/

void	Client::handleOutgoingState()
{
	std::map<clientStatus, std::function<void (Client*)>> outgoingMethods =
	{
		{RESPONDING, &Client::writeToClient},
		{writingToFile, &Client::writeToFile},
		{launchCgi, &Client::launchCGI},
		{parseCgi, &Client::readFromFile},
		{showDirectory, &Client::parseDirectory},
		{readingFromFile, &Client::readFromFile},
	};
	outgoingMethods[status](this);
}
