#include "WebServer.hpp"

std::string WebServer::getCurrentTime() const
{
    char date[1000];
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", &tm);
    return std::string(date);
}

std::string WebServer::getMimeType(const std::string& path) const
{
    size_t dotPos = path.find_last_of(".");
    if (dotPos == std::string::npos)
        return "text/plain";
    std::string extension = path.substr(dotPos + 1);
    if (extension == "html") return "text/html";
    if (extension == "css") return "text/css";
    if (extension == "js") return "application/javascript";
    if (extension == "png") return "image/png";
    if (extension == "jpg" || extension == "jpeg") return "image/jpeg";
    if (extension == "gif") return "image/gif";
    return "application/octet-stream";
}

std::string WebServer::showErrorPage(const std::string& error) const
{
    std::ifstream file("error_pages/" + error + ".html");
    std::stringstream stream;
    if (file.is_open())
    {
        stream << file.rdbuf();
        file.close();
    }
    return stream.str();
}

bool WebServer::handleClientWrite(int clientFd)
{
    HttpRequest& request = requests[clientFd];
    std::string& response = request.response;

    if (request.method == "HEAD" || request.method == "GET")
    {
        std::ifstream inFile("." + request.path, std::ios::binary);
        if (inFile.is_open())
        {
            std::stringstream buffer;
            buffer << inFile.rdbuf();
            inFile.close();
            std::string body = buffer.str();
            response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
            response += "Content-Type: " + getMimeType(request.path) + "\r\n"; // getMimeType kullanımı
            response += "Cache-Control: no-cache\r\n";
            response += "Connection: " + std::string(request.getHeader("connection") == "keep-alive" ? "keep-alive" : "close") + "\r\n";
            response += "Server: MyWebServer\r\n";
            response += "Date: " + getCurrentTime() + "\r\n";
            if (request.method == "GET")
            {
                response += "\r\n" + body;
            }
            else
            {
                response += "\r\n";
            }
        }
        else
        {
            std::string errorBody = showErrorPage("404");
            response = "HTTP/1.1 404 Not Found\r\n";
            response += "Content-Type: text/html\r\n";
            response += "Content-Length: " + std::to_string(errorBody.size()) + "\r\n";
            response += "\r\n" + errorBody;
        }
    }
    else if (request.method == "POST")
    {
        std::ofstream outFile("./www/uploads/data.txt");
        if (outFile.is_open())
        {
            outFile << request.body;
            outFile.close();
            response = "HTTP/1.1 201 Created\r\nContent-Length: 0\r\n\r\n";
        }
        else
        {
            std::string errorBody = showErrorPage("500");
            response = "HTTP/1.1 500 Internal Server Error\r\n";
            response += "Content-Type: text/html\r\n";
            response += "Content-Length: " + std::to_string(errorBody.size()) + "\r\n";
            response += "\r\n" + errorBody;
        }
    }
    else if (request.method == "PUT")
    {
        std::ofstream outFile("." + request.path, std::ios::binary);
        if (outFile.is_open())
        {
            outFile << request.body;
            outFile.close();
            response = "HTTP/1.1 201 Created\r\n";
            response += "Content-Length: 0\r\n";
            response += "Content-Type: " + getMimeType(request.path) + "\r\n"; // getMimeType kullanımı
            response += "\r\n";
        }
        else
        {
            std::string errorBody = showErrorPage("500");
            response = "HTTP/1.1 500 Internal Server Error\r\n";
            response += "Content-Type: text/html\r\n";
            response += "Content-Length: " + std::to_string(errorBody.size()) + "\r\n";
            response += "\r\n" + errorBody;
        }
    }
    else if (request.method == "DELETE")
    {
        if (remove(("./www" + request.path).c_str()) == 0)
        {
            response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
        }
        else
        {
            std::string errorBody = showErrorPage("404");
            response = "HTTP/1.1 404 Not Found\r\n";
            response += "Content-Type: text/html\r\n";
            response += "Content-Length: " + std::to_string(errorBody.size()) + "\r\n";
            response += "\r\n" + errorBody;
        }
    }
    else
    {
        std::string errorBody = showErrorPage("405");
        response = "HTTP/1.1 405 Method Not Allowed\r\n";
        response += "Content-Type: text/html\r\n";
        response += "Content-Length: " + std::to_string(errorBody.size()) + "\r\n";
        response += "\r\n" + errorBody;
    }

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