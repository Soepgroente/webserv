#include "WebServer.hpp"

WebServer::~WebServer()
{
	for (size_t i = 0; i < clients.size(); i++)
	{
		closeConnection(i + servers.size(), i);
	}
	for (size_t i = 0; i < servers.size(); i++)
	{
		if (servers[i].socket != -1)
			close(servers[i].socket);
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
	// std::cout << newConnectionTotal << " connections" << std::endl;
	printToLog("New connection accepted");
}

/*	Sets up a 408 (request timeout) for clients that have unfinished requests, 
	closes connections which haven't communicated anything in DEFAULT_TIMEOUT amount of time	*/

void	WebServer::removeInactiveConnections()
{
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].getClientStatus() == LISTENING && (clients[i].getRequest().buffer.empty() == false || \
			clients[i].getRequest().splitRequest.empty() == false) && \
			WebServer::timeout(clients[i].getLatestPing(), clients[i].getTimeout()) == true)
		{
			clients[i].setupErrorPage(408);
		}
		else if (clients[i].getClientStatus() == CLOSING || clients[i].getRemainingRequests() == 0 || \
			WebServer::timeout(clients[i].getLatestPing(), clients[i].getTimeout()) == true)
		{
			closeConnection(i + servers.size(), i);
			i--;
		}
	}
}

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

		/*	Loop through Servers to see if a new connection is attempted	*/

		for (size_t i = 0; i < amountOfServers && pollDescriptors[i].revents != 0; i++)
		{
			if (isServerSocket(i) == true)
			{
				acceptConnection(pollDescriptors[i].fd);
			}
		}
		/*	Loop through Clients to see if anything happened	*/

		for (size_t i = 0; i < clients.size(); i++)
		{
			if (pollDescriptors[i + amountOfServers].revents == 0)
				continue ;

			Client& client = clients[i];

			if ((pollDescriptors[i + amountOfServers].revents & POLLHUP) != 0)
			{
				client.setClientStatus(CLOSING);
			}
			else if ((pollDescriptors[i + amountOfServers].revents & POLLIN) != 0)
			{
				client.readIncomingRequest();
				if (client.getClientStatus() != LISTENING)
				{
					pollDescriptors[i + amountOfServers].events = POLLOUT;
				}
			}
			else if ((pollDescriptors[i + amountOfServers].revents & POLLOUT) != 0)
			{
				client.handleOutgoingState();
				if (client.getClientStatus() == LISTENING)
				{
					pollDescriptors[i + amountOfServers].events = POLLIN;
				}
			}
			client.setPingTime();
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