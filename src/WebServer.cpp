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

void	WebServer::acceptConnection(int serverSocket)
{
	Client	newClient;

	newClient.setFd(serverSocket);
	pollDescriptors.push_back({newClient.getFd(), POLLIN, 0});
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
			Client& client = getClient(pollDescriptors[i].fd);

			if ((pollDescriptors[i].revents & POLLIN) != 0)
			{
				if (isServerSocket(pollDescriptors[i].fd) == true)
				{
					acceptConnection(pollDescriptors[i].fd);
				}
				else if (client.getCgiStatus() == parseCgi)
				{
					parseCgiOutput(); // not done yet
				}
				else if (handleClientRead(pollDescriptors[i].fd) == false)
				{
					i--;
				}
			}		
			else if ((pollDescriptors[i].revents & POLLOUT) != 0)
			{
				if (client.getCgiStatus() == launchCgi)
					launchCGI(client, pollDescriptors[i].fd);
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
