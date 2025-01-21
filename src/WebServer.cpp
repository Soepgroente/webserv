#include "WebServer.hpp"

WebServer::~WebServer()
{
	for (Client& client : clients)
	{
		closeConnection(client.getFd());
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
		if (timeout(clients[i].getLatestPing(), clients[i].getTimeout()) == true)
		{
			closeConnection(clients[i].getFd());
			pollDescriptors.erase(pollDescriptors.begin() + i);
			i--;
		}
	}
}

void	WebServer::checkConnectionStatuses()
{
	if (poll(pollDescriptors.data(), pollDescriptors.size(), 0) == -1)
		throw std::runtime_error("Failed to poll");
	removeInactiveConnections();
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
			Client* client = getClient(pollDescriptors[i].fd);

			if (isServerSocket(pollDescriptors[i].fd) == true)
			{
				acceptConnection(pollDescriptors[i].fd);
			}
			else if ((pollDescriptors[i].revents & POLLIN) != 0)
			{
				if (client != nullptr && client->getCgiStatus() == parseCgi)
				{
					parseCgiOutput(*client);
				}
				if (handleRequest(client, pollDescriptors[i].fd) == false)
				{
					i--;
				}
			}
			else if (client != nullptr && (pollDescriptors[i].revents & POLLOUT) != 0)
			{
				if (client->getCgiStatus() == launchCgi)
					launchCGI(*client);
				else if (handleResponse(*client, pollDescriptors[i].fd) == false)
					i--;
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