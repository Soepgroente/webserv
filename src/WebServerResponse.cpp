#include "WebServer.hpp"
#include <fstream>

// static std::string	getMimeType(const std::string& fileType)
// {
//     if (dotPos == std::string::npos)
//         return "text/plain";
//     std::string extension = path.substr(dotPos + 1);
//     if (extension == "html") return "text/html";
//     if (extension == "css") return "text/css";
//     if (extension == "js") return "application/javascript";
//     if (extension == "png") return "image/png";
//     if (extension == "jpg" || extension == "jpeg") return "image/jpeg";
//     if (extension == "gif") return "image/gif";
//     return "application/octet-stream";
// }

bool	WebServer::handleGet(Client& client, std::string& buffer)
{
	if (client.getClientStatus() == writeCgiResults)
	{
		buffer = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(client.getResponse().buffer.size()) + "\r\nContent-Type: image/svg+xml\r\n\r\n" + client.getResponse().buffer;
		return (true);
	}
	if (client.getFileFd() != -1)
	{
		char fileBuffer[BUFFERSIZE];
		ssize_t bytesRead = read(client.getFileFd(), fileBuffer, BUFFERSIZE);
		if (bytesRead == -1)
		{
			std::cerr << "Error reading from file: " << strerror(errno) << std::endl;
			close(client.getFileFd());
			client.setFileFd(-1);
			buffer = HttpResponse::defaultResponses[500];
			return (true);
		}
		if (bytesRead == 0)
		{
			close(client.getFileFd());
			client.setFileFd(-1);
			buffer = HttpResponse::defaultResponses[200];
			return (true);
		}
		buffer = std::string(fileBuffer, bytesRead);
		return (true);
	}
	// buffer = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body; // I don't know if the content-type is necessary here, I saw that firefox understands it automatically
	return (true);
}

bool	WebServer::handlePost(Client& client, std::string& buffer)
{
	std::ofstream outFile(client.getRequest().path, std::ios::binary);
	if (outFile.is_open())
	{
		outFile << client.getRequest().body;
		outFile.close();
		buffer = HttpResponse::defaultResponses[200];
	}
	else
	{
		buffer = HttpResponse::defaultResponses[500];
	}
	return (true);
}

bool	WebServer::handleDelete(Client& client, std::string& buffer)
{
	if (remove(client.getRequest().path.c_str()) == 0)
	{
		buffer = HttpResponse::defaultResponses[200];
	}
	else
	{
		buffer = HttpResponse::defaultResponses[404];
	}
	return (true);
}

bool	WebServer::replyToClient(std::string& buffer, int clientFd)
{
	int pollIndex = getPollfdIndex(clientFd);

	if (buffer.empty() == true)
	{
		assert(pollIndex != -1);
        pollDescriptors[pollIndex].events = POLLIN;
		return (true);
	}
	assert(buffer.size() != 0);
	assert(buffer.c_str() != nullptr);
    ssize_t writtenBytes = write(clientFd, buffer.c_str(), buffer.size());

    if (writtenBytes == -1)
    {
        std::cerr << "Error writing to client_fd: " << strerror(errno) << std::endl;
        closeConnection(clientFd);
        return (false);
    }
    if (static_cast<size_t>(writtenBytes) < buffer.size())
    {
        buffer.erase(0, writtenBytes);
		std::cout << "size of buffer: " << buffer.size() << std::endl;
    }
	if (buffer.size() == 0)
	{
		puts("I'm triggered");
        pollDescriptors[getPollfdIndex(clientFd)].events = POLLIN;
	}
	return (true);
}

bool	WebServer::handleResponse(Client& client, int clientFd)
{
	std::string& buffer = client.getResponse().buffer;

	std::map<std::string, std::function<bool(WebServer*, Client&, std::string&)>> methods =
	{
		{"GET", &WebServer::handleGet},
		{"POST", &WebServer::handlePost},
		{"DELETE", &WebServer::handleDelete}
	};
    if (methods[client.getRequest().method](this, client, buffer) == true)
		return (replyToClient(buffer, clientFd));
	return (false);
}
