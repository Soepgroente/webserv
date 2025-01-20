#include "WebServer.hpp"

WebServer::~WebServer()
{
	for (pollfd it : pollDescriptors)
	{
		if (it.fd != -1)
		{
			// send message to unfinished requests
			shutdown(it.fd, SHUT_RDWR);	// wanna check if we can use this function and whether it is helpful at all?
			close(it.fd);
		}
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
	Client	client(serverSocket, getServer(serverSocket));

	for (Client& tmp : this->clients)
	{
		if (tmp.getFd() == client.getFd())
			throw std::runtime_error("Client already exists");
	}
	clients.push_back(client);
	// clients.push_back({serverSocket, getServer(serverSocket)});
	pollDescriptors.push_back({clients.back().getFd(), POLLIN, 0});
	std::cout << "Accepted connection" << std::endl;
}

void	WebServer::loopadydoopady()
{
	while (serverShouldRun == true)
	{
		int amountOfEvents;
		
		amountOfEvents = poll(pollDescriptors.data(), pollDescriptors.size(), 0);
		if (amountOfEvents == -1)
			throw std::runtime_error("Failed to poll");
		if (amountOfEvents == 0)
			continue ;
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
				if (handleClientRead(client, pollDescriptors[i].fd) == false)
				{
					i--;
				}
			}
			else if (client != nullptr && (pollDescriptors[i].revents & POLLOUT) != 0)
			{
				if (client->getCgiStatus() == launchCgi)
					launchCGI(*client);
				else if (handleClientWrite(pollDescriptors[i].fd) == false)
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