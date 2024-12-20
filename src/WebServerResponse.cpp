#include "WebServer.hpp"

void	WebServer::handleClientWrite(int clientFd)
{
	std::string respondre = "YO MAMA";

	std::stringstream response;
	response << "HTTP/1.1 200 OK\r\n" << "Content-Length: 7" << "\r\n";
	response << "Content-Type: " << "text/html" << "\r\n\r\n" << respondre;

	pollDescriptors[getPollfdIndex(clientFd)].events = POLLIN;
	write(clientFd, response.str().c_str(), response.str().size());
}
