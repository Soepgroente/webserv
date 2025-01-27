#include "WebServer.hpp"

WebServer::~WebServer()
{
	puts("Destructor called");
	for (Client& client : clients)
	{
		closeConnection(client.getFd());
		// poll descriptors?
	}
}

const Server&	WebServer::getServer(int serverSocket)
{
	for (Server& server : servers)
	{
		if (server.socket == serverSocket)
			return (server);
	}
	throw std::runtime_error("Server not found");
}

void	WebServer::acceptConnection(int serverSocket)
{
	addClient(serverSocket);
	pollDescriptors.push_back({clients.back().getFd(), POLLIN, 0});
	std::cout << "Accepted connection" << std::endl;
}

void	WebServer::removeInactiveConnections()
{
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].getClientStatus() == clientShouldClose || \
			WebServer::timeout(clients[i].getLatestPing(), clients[i].getTimeout()) == true)
		{
			puts("inactive connecti0n");
			closeConnection(clients[i].getFd());
			i--;
		}
	}
}

void	WebServer::handleIncoming(Client* client, size_t& position, int fd)
{
	// if ()
	if (client->getClientStatus() == parseCgi)
	{
		parseCgiOutput(*client);
	}
	else if (client->getClientStatus() == readingFromFile)
	{
		ssize_t			readBytes;
		std::string		buffer;
		HttpResponse&	response = client->getResponse();

		buffer.resize(BUFFERSIZE);
		readBytes = read(client->getFileFd(), buffer.data(), BUFFERSIZE);
		if (readBytes == -1)
		{
			throw std::runtime_error("Error reading from client_fd");
		}
		response.buffer += buffer;
	}
	else if (handleRequest(*client, fd) == false)
	{
		position--;
	}
}

void	WebServer::handleOutgoing(Client& client, size_t& position, int fd)
{
	if (client.getClientStatus() == launchCgi)
	{
		launchCGI(client);
	}
	else if (handleResponse(client, fd) == false)
	{
		position--;
	}
}

void	WebServer::checkConnectionStatuses()
{
	removeInactiveConnections();
	if (poll(pollDescriptors.data(), pollDescriptors.size(), 0) == -1)
		throw std::runtime_error("Failed to poll");
}

void	WebServer::loopadydoopady()
{
	while (serverShouldRun == true)
	{
		checkConnectionStatuses();
		for (size_t i = 0; i < pollDescriptors.size(); i++)
		{
			if (pollDescriptors[i].revents == 0)
				continue ;
			if (isServerSocket(i) == true)
			{
				acceptConnection(pollDescriptors[i].fd);
				continue ;
			}
			Client* client = getClient(pollDescriptors[i].fd);

			assert(client != nullptr);
			client->setPingTime();
			if ((pollDescriptors[i].revents & POLLHUP) != 0)
			{
				closeAndResetFd(pollDescriptors[i].fd);
				client->setClientStatus(clientShouldRespond);
			}
			else if ((pollDescriptors[i].revents & POLLIN) != 0)
			{
				handleIncoming(client, i, pollDescriptors[i].fd);
			}
			else if ((pollDescriptors[i].revents & POLLOUT) != 0 && client->getClientStatus() == clientShouldRespond)
			{
				handleOutgoing(*client, i, pollDescriptors[i].fd);
			}
		}
	}
}

void	WebServer::startTheThing()
{
	initialize();
	pollDescriptors = createPollArray();
	loopadydoopady();
}

std::ostream&	operator<<(std::ostream& out, const std::vector<pollfd>& p)
{
	for (size_t i = 0; i < p.size(); i++)
	{
		out << "Index: " << i << std::endl;
		out << "Pollfd: " << p[i].fd << std::endl;
		out << "Events: " << p[i].events << std::endl;
		out << "Revents: " << p[i].revents << std::endl;
		out << std::endl;
	}
	return (out);
}