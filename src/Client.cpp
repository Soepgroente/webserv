#include "Client.hpp"

Client::Client(const Server& in) : 
	latestPing(WebServer::getTime()), timeout(DEFAULT_TIMEOUT), writePos(0), remainingRequests(INT_MAX), \
	status(LISTENING), fd(-1), fileFd(-1), \
	request(HttpRequest()), response(HttpResponse()), server(in)
{
}

Client::Client(const Client& other) : 
	latestPing(other.latestPing), timeout(other.timeout), writePos(other.writePos), \
	remainingRequests(other.remainingRequests), status(other.status), \
	fd(other.fd), fileFd(other.fileFd), \
	request(other.request), response(other.response), server(other.server)
{
}

Client::~Client()
{
}

Client&	Client::operator=(const Client& other)
{
	Client* tmp = new Client(other);

	return (*tmp);
}

void	Client::initializeSocket(int serverSocket)
{
	struct sockaddr_in	clientAddress;
	socklen_t			clientAddressLength = sizeof(sockaddr_in);

	this->fd = accept(serverSocket, reinterpret_cast<sockaddr*> (&clientAddress), &clientAddressLength);
	if (this->fd == -1)
		throw std::runtime_error("Failed to accept client");
	if (fcntl(this->fd, F_SETFL, fcntl(this->fd, F_GETFL, 0) | O_NONBLOCK) == -1)
		throw std::runtime_error("Failed to set client socket to non-blocking");
}
