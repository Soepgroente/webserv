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

void	WebServer::acceptConnection(int serverSocket) // we keep duplicating our Client and associated pollfd
{
	clients.push_back({serverSocket, getServer(serverSocket)});
	std::cout << clients.back() << std::endl;
	std::cout << "address of last client element: " << &(clients.back()) << std::endl;
	std::cout << "Client array size: " << clients.size() << std::endl;
	pollDescriptors.push_back({clients.back().getFd(), POLLIN, 0});
	std::cout << "Poll descriptor array: " << pollDescriptors << std::endl;
	std::cout << "Accepted new connection" << std::endl;
}

void	WebServer::loopadydoopady()
{
	while (serverShouldRun == true)
	{
		size_t size = pollDescriptors.size();

		if (poll(pollDescriptors.data(), size, 0) == -1)
			throw std::runtime_error("Failed to poll");
		// std::cout << "Poll descriptor array:\n-------" << pollDescriptors << std::endl;

		for (size_t i = 0; i < pollDescriptors.size(); i++)
		{
			Client* client = getClient(pollDescriptors[i].fd);

			std::cout << "In poll loopadydoopady " << client << std::endl;
			
			if ((pollDescriptors[i].revents & POLLIN) != 0)
			{
				if (isServerSocket(pollDescriptors[i].fd) == true)
				{
					acceptConnection(pollDescriptors[i].fd);
				}
				// else if (client != nullptr && client->getCgiStatus() == parseCgi)
				// {
				// 	parseCgiOutput(*client);
				// }
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
			sleep(1);
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