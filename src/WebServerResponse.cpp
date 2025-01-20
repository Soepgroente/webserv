#include "WebServer.hpp"
#include <fstream>

static bool	handleGet(Client& client, std::string& buffer)
{
	// GET işlemi: Dosyayı oku ve yanıtla
	std::ifstream inFile("." + client.getRequest().path, std::ios::binary);
	if (inFile.is_open())
	{
		std::stringstream buf;
		buf << inFile.rdbuf();
		inFile.close();
		std::string body = buf.str();
		buffer = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body; // I don't know if the content-type is necessary here, I saw that firefox understands it automatically
	}
	else
	{
		buffer = HttpResponse::defaultResponses["404"];
	}
	return (true);
}

static bool	handlePost(Client& client, std::string& buffer)
{
	std::ofstream outFile(client.getRequest().path, std::ios::binary);
	if (outFile.is_open())
	{
		outFile << client.getRequest().body;
		outFile.close();
		buffer = HttpResponse::defaultResponses["200"];
	}
	else
	{
		buffer = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
	}
	return (true);
}

static bool	handleDelete(Client& client, std::string& buffer)
{
	if (remove(client.getRequest().path.c_str()) == 0)
	{
		buffer = HttpResponse::defaultResponses["200"];
	}
	else
	{
		buffer = HttpResponse::defaultResponses["404"];
	}
	return (true);
}

bool	WebServer::replyToClient(std::string& buffer, int clientFd)
{
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
    }
    else
    {
        pollDescriptors[getPollfdIndex(clientFd)].events = POLLIN;
        buffer.clear();
    }
	return (true);
}

bool	WebServer::handleClientWrite(Client& client, int clientFd)
{
	std::string& buffer = client.getResponse().buffer;

	std::map<std::string, std::function<bool(Client&, std::string&)>> methods =
	{{"GET", &handleGet},
	{"POST", &handlePost},
	{"DELETE", &handleDelete}};

    if (methods[client.getRequest().method](client, buffer) == true)
		return (replyToClient(buffer, clientFd));
	return (false);
}

/* void	WebServer::handleClientWrite(int clientFd)
{

    requests[clientFd].response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello World!";
    ssize_t writtenBytes = write(clientFd, requests[clientFd].response.c_str(), requests[clientFd].response.size());

    if (writtenBytes == -1)
    {
        std::cerr << "Error writing to client_fd: " << strerror(errno) << std::endl;
        closeConnection(clientFd);
        return;
    }
    if (static_cast<size_t>(writtenBytes) < requests[clientFd].response.size())
    {
        requests[clientFd].response.erase(0, writtenBytes);
    }
    else
    {
        pollDescriptors[getPollfdIndex(clientFd)].events = POLLIN;
        requests[clientFd].response.clear();
    }
} */