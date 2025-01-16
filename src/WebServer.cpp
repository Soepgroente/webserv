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
	clients.push_back({serverSocket, getServer(serverSocket)});
	pollDescriptors.push_back({clients.back().getFd(), POLLIN, 0});
	std::cout << "Accepted new connection" << std::endl;
}

void	WebServer::loopadydoopady()
{
	while (serverShouldRun == true)
	{
		if (poll(pollDescriptors.data(), pollDescriptors.size(), 0) == -1)
			throw std::runtime_error("Failed to poll");
		for (size_t i = 0; i < pollDescriptors.size(); i++)
		{
			Client* client = getClient(pollDescriptors[i].fd);
			
			if ((pollDescriptors[i].revents & POLLIN) != 0)
			{
				if (isServerSocket(pollDescriptors[i].fd) == true)
				{
					acceptConnection(pollDescriptors[i].fd);
				}
				else if (client != nullptr && client->getCgiStatus() == parseCgi)
				{
					parseCgiOutput(*client);
				}
				else if (handleClientRead(pollDescriptors[i].fd) == false)
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
