#include "Client.hpp"

/*	Writes whichever amount is smaller between BUFFERSIZE and remaining response size to client.
	Clears the request & response structs once everything has been sent	*/

void	Client::writeToClient()
{
	ssize_t	writtenBytes;

	writtenBytes = write(fd, &response.reply[writePos], \
		std::min(response.reply.size() - writePos, (size_t)BUFFERSIZE));
	if (writtenBytes == -1)
	{
		printToLog("Error writing to client fd: " + std::string(strerror(errno)));
		status = CLOSING;
		return ;
	}
	if (writtenBytes < BUFFERSIZE)
	{
		status = LISTENING;
		writePos = 0;
		response.clear();
		request.clear();
	}
	else
		writePos += BUFFERSIZE;
}

void	Client::writeToFile()
{
	ssize_t	writtenBytes;

	writtenBytes = write(fileFd, &request.body[writePos], \
		std::min(request.body.size() - writePos, (size_t)BUFFERSIZE));
	if (writtenBytes == -1)
	{
		printToLog("Error writing to file fd: " + std::string(strerror(errno)));
		closeAndResetFd(Client::fileAndCgiDescriptors, fileFd);
		writePos = 0;
		status = CLOSING;
		return ;
	}
	if (writtenBytes < BUFFERSIZE)
	{
		status = RESPONDING;
		writePos = 0;
		response.reply = HttpResponse::defaultResponses[200];
		closeAndResetFd(Client::fileAndCgiDescriptors, fileFd);
	}
	else
		writePos += BUFFERSIZE;
}

// static std::string	generateDirectoryListing(const std::filesystem::path dir)
// {
// 	std::stringstream	ss;
// 	// std::string			normalizedPath = dir.string();

// 	// if (normalizedPath.back() != '/')
// 	// 	normalizedPath += '/';
// 	ss << "<!DOCTYPE html>\n"
//     	<< "<html>\n"
//         << "<head><meta charset=\"UTF-8\"><title>Directory Listing</title></head>\n"
//         << "<body>\n"
//         << "<h1>Index of " << dir.filename().string() << "</h1>\n"
//         << "<ul>\n";
// 	for (std::filesystem::directory_entry dirEntry : std::filesystem::directory_iterator(dir))
// 	{
// 		std::string entry = dirEntry.path().filename().string();
// 		std::cout << "entry: " << entry << std::endl;
// 		ss << " <li><a href=\"" << dir.filename().string() << entry << "\">" << entry << "</a></li>\n";
// 	}
// 	ss << "</ul>\n"
// 		<< "</body>\n"
// 		<< "</html>\n";
// 	return ss.str();
// }

void	Client::parseDirectory()
{
	std::filesystem::path	dir(request.path);
	// std::cout << "request.path: " << request.path << std::endl;
	// std::cout << "root_name: " << dir.root_name() << std::endl;
	// std::cout << "root_path: " << dir.root_path() << std::endl;
	// std::cout << "relative_path: " << dir.relative_path() << std::endl;
	// std::cout << "parent_path: " << dir.parent_path() << std::endl;
	// std::cout << "filename: " << dir.filename() << std::endl;
	// std::cout << "stem: " << dir.stem() << std::endl;
	// std::cout << "extension: " << dir.extension() << std::endl;

	// response.buffer = generateDirectoryListing(dir);
	// for (std::filesystem::directory_entry dir_entry : std::filesystem::recursive_directory_iterator(dir))
	for (std::filesystem::directory_entry dir_entry : std::filesystem::directory_iterator(dir))
	{
		std::cout << dir_entry.path().filename() << std::endl;
		response.buffer += (dir_entry.path().filename().string() + "\r\n");
	}
	response.reply = HttpResponse::defaultResponses[200] + std::to_string(response.buffer.size()) + "\r\n\r\n" + response.buffer;
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
