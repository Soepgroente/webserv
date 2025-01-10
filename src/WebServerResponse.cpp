#include "WebServer.hpp"
#include <fstream>

bool	WebServer::handleClientWrite(int clientFd)
{
    HttpRequest& request = requests[clientFd];
    std::string& response = request.response;

    if (request.method == "GET")
    {
        // GET işlemi: Dosyayı oku ve yanıtla
        std::ifstream inFile("." + request.path, std::ios::binary);
        if (inFile.is_open())
        {
            std::stringstream buffer;
            buffer << inFile.rdbuf();
            inFile.close();
            std::string body = buffer.str();
            response = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body; // I don't know if the content-type is necessary here, I saw that firefox understands it automatically
        }
        else
        {
            response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        }
    }
    else if (request.method == "POST")
    {
        // POST işlemi: Veriyi kaydet
        std::ofstream outFile("/path/to/save/data.txt");
        if (outFile.is_open())
        {
            outFile << request.body;
            outFile.close();
            response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
        }
        else
        {
            response = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
        }
    }
    else if (request.method == "DELETE")
    {
        // DELETE işlemi: Dosyayı sil
        if (remove(request.path.c_str()) == 0)
        {
            response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
        }
        else
        {
            response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        }
    }
    else
    {
        // Desteklenmeyen yöntemler için 405 Method Not Allowed yanıtı
        response = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n";
    }

    // std::cout << response << std::endl;
    ssize_t writtenBytes = write(clientFd, response.c_str(), response.size());

    if (writtenBytes == -1)
    {
        std::cerr << "Error writing to client_fd: " << strerror(errno) << std::endl;
        closeConnection(clientFd);
        return false;
    }
    if (static_cast<size_t>(writtenBytes) < response.size())
    {
        response.erase(0, writtenBytes);
    }
    else
    {
        pollDescriptors[getPollfdIndex(clientFd)].events = POLLIN;
        response.clear();
    }
	return true;
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