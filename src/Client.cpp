#include "Client.hpp"

std::vector<struct pollfd>	Client::fileAndCgiDescriptors;

Client::Client(const Server& in) : 
	latestPing(WebServer::getTime()), timeout(DEFAULT_TIMEOUT), writePos(0), readPos(0),
	remainingRequests(INT_MAX), status(LISTENING), fd(-1), fileFd(-1), \
	request(HttpRequest()), response(HttpResponse()), server(&in)
{
}

Client::Client(const Client& other) : 
	latestPing(other.latestPing), timeout(other.timeout), writePos(other.writePos), \
	readPos(other.readPos), remainingRequests(other.remainingRequests), \
	status(other.status), fd(other.fd), fileFd(other.fileFd), \
	request(other.request), response(other.response), server(other.server)
{
}

Client::~Client()
{
}

Client&	Client::operator=(const Client& other)
{
	if (this != &other)
	{
		this->latestPing = other.latestPing;
		this->timeout = other.timeout;
		this->writePos = other.writePos;
		this->readPos = other.readPos;
		this->remainingRequests = other.remainingRequests;
		this->status = other.status;
		this->fd = other.fd;
		this->fileFd = other.fileFd;
		this->request = other.request;
		this->response = other.response;
		this->server = other.server;
	}
	return (*this);
}

void	Client::initializeSocket(int serverSocket)
{
	struct sockaddr_in	clientAddress;
	socklen_t			clientAddressLength = sizeof(sockaddr_in);

	fd = accept(serverSocket, reinterpret_cast<sockaddr*> (&clientAddress), &clientAddressLength);
	if (fd == -1)
	{
		printToLog("Failed to accept client");
		status = CLOSING;
		return ;
	}
	if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) == -1)
		throw std::runtime_error("Failed to set client socket to non-blocking");
}

void	Client::reset()
{
	request.clear();
	response.clear();
	status = LISTENING;
}