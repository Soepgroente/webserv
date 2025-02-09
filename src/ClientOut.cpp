#include "Client.hpp"

void	Client::writeToClient()
{
	ssize_t	writtenBytes;

	writtenBytes = write(fd, &response.buffer[writePos], \
		std::min(response.buffer.size() - writePos, (size_t)BUFFERSIZE));
	if (writtenBytes == -1)
	{
        std::cerr << "Error writing to client_fd: " << strerror(errno) << std::endl;
		status = CLOSING;
		return ;
	}
	latestPing = WebServer::getTime();
	if (writtenBytes < BUFFERSIZE)
	{
		status = LISTENING;
		writePos = 0;
		readPos = 0;
		response.buffer.clear();
		request.clear();
	}
	else
		writePos += BUFFERSIZE;
}

// void	Client::parseDirectory()
// {

// 	status = RESPONDING;
// }

void	Client::handleOutgoingState()
{
	// std::cout << *this << std::endl;
	if (status == RESPONDING)
	{
		writeToClient();
	}
	else
	{
		if (fileFd == -1)
		{
			return ;
		}
		response.buffer.resize(response.buffer.size() + BUFFERSIZE);
		ssize_t readBytes = read(fileFd, &response.buffer[readPos], BUFFERSIZE);
		if (readBytes == -1)
		{
			std::cerr << "Error reading from file: " << strerror(errno) << std::endl;
			response.buffer = HttpResponse::defaultResponses[500];
			status = RESPONDING;
			return ;
		}
		if (readBytes < BUFFERSIZE)
		{
			response.buffer.resize(response.buffer.size() - (BUFFERSIZE - readBytes));
			response.buffer = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(response.buffer.size()) + "\r\n\r\n" + response.buffer;
			readPos = 0;
			status = RESPONDING;
		}
		readPos += BUFFERSIZE;
	}
	// std::map<clientStatus, std::function<void (Client*)>> outgoingMethods =
	// {
	// 	{RESPONDING, &Client::writeToClient},
	// 	{launchCgi, &Client::launchCGI},
	// 	{showDirectory, &Client::parseDirectory},
	// 	{readingFromFile, &Client::readFromFile},
	// };
	// outgoingMethods[status](this);
}
/* if (inFile.is_open())
        {
            std::stringstream buffer;
            buffer << inFile.rdbuf();
            inFile.close();
            std::string body = buffer.str();
            response = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body; // I don't know if the content-type is necessary here, I saw that firefox understands it automatically
        } */