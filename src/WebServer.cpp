#include "WebServer.hpp"

WebServer::~WebServer()
{
	puts("Destructor called");
	for (size_t i = 0; i < clients.size(); i++)
	{
		closeConnection(i + servers.size(), i);
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
	std::cout << "Accepted connection" << std::endl;
}

void	WebServer::removeInactiveConnections()
{
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].getClientStatus() == CLOSING || \
			WebServer::timeout(clients[i].getLatestPing(), clients[i].getTimeout()) == true)
		{
			puts("inactive connecti0n");
			closeConnection(i + servers.size(), i);
			i--;
		}
	}
}

/* void	WebServer::handleIncoming(Client* client, size_t& position, int fd)
{
	if (client->getClientStatus() == parseCgi)
	{
		parseCgiOutput(*client);
	}
	else if (client->getClientStatus() == readingFromFile)
	{
		client->readFromFile();
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
	else
		handleResponse(client, fd);
} */

void	WebServer::loopadydoopady()
{
	size_t amountOfServers = servers.size();

	while (serverShouldRun == true)
	{
		removeInactiveConnections();
		if (poll(pollDescriptors.data(), pollDescriptors.size(), 0) == -1)
			throw std::runtime_error("Failed to poll clients");
		if (poll(Client::fileAndCgiDescriptors.data(), Client::fileAndCgiDescriptors.size(), 0) == -1)
			throw std::runtime_error("Failed to poll cgi/file descriptors");
		for (size_t i = 0; i < pollDescriptors.size(); i++)
		{
			if (pollDescriptors[i].revents == 0)
				continue ;
			if (isServerSocket(i) == true)
			{
				acceptConnection(pollDescriptors[i].fd);
				continue ;
			}
			Client& client = clients[i - amountOfServers];

			client.setPingTime();
			if ((pollDescriptors[i].revents & POLLHUP) != 0)
			{
				// closeAndResetFd(pollDescriptors[i].fd);
				client.setClientStatus(CLOSING);
			}
			else if ((pollDescriptors[i].revents & POLLIN) != 0)
			{
				client.readIncomingRequest();
				if (client.getClientStatus() != LISTENING)
				{
					pollDescriptors[i].events = POLLOUT;
				}
			}
			else if ((pollDescriptors[i].revents & POLLOUT) != 0)
			{
				client.handleOutgoingState();
				if (client.getClientStatus() == LISTENING)
				{
					pollDescriptors[i].events = POLLIN;
				}
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