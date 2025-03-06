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

/*	Adds a client and reserves space in the request buffer to avoid a lot of reallocation.
	Reserves less space as more clients are added to balance speed vs memory	*/

void	WebServer::acceptConnection(int serverSocket)
{
	addClient(serverSocket);
	clients.back().getRequest().buffer.reserve(32 * BUFFERSIZE);
	// printToLog("New connection accepted");
}

/*	Sets up a 408 (request timeout) for clients that have unfinished requests, 
	closes connections which haven't communicated anything in DEFAULT_TIMEOUT amount of time	*/

void	WebServer::removeInactiveConnections()
{
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].checkTimeout() == CLOSING)
		{
			closeConnection(i + servers.size(), i);
			i--;
		}
	}
}

void	WebServer::loopadydoopady()
{
	size_t amountOfServers = servers.size();

	while (true)
	{
		removeInactiveConnections();
		waitpid(-1, NULL, WNOHANG);
		if (poll(pollDescriptors.data(), pollDescriptors.size(), 0) == -1)
			throw std::runtime_error("Failed to poll clients");
		if (poll(Client::fileAndCgiDescriptors.data(), Client::fileAndCgiDescriptors.size(), 0) == -1)
			throw std::runtime_error("Failed to poll cgi/file descriptors");

		/*	Loop through Servers to see if a new connection is attempted	*/

		for (size_t i = 0; i < amountOfServers; i++)
		{
			if (pollDescriptors[i].revents == 0)
				continue ;
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
	throw std::runtime_error("Error: shutting down server");
}

void	WebServer::startTheThing()
{
	initialize();
	pollDescriptors = createPollArray();
	loopadydoopady();
}
