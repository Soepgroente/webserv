#include "WebServer.hpp"

void	WebServer::handleClientWrite(int clientFd)
{
	(void)clientFd;
	write(clientFd, response.c_str(), response.size());
}
