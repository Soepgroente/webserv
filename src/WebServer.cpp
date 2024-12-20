#include "WebServer.hpp"

WebServer::~WebServer()
{
	for (pollfd it : pollDescriptors)
	{
		if (it.fd != -1)
		{
			shutdown(it.fd, SHUT_RDWR);	// wanna check if we can use this function and whether it is helpful at all?
			close(it.fd);
		}
	}
}

void	WebServer::acceptConnection(int serverSocket)
{
	int					clientFd;
	struct sockaddr_in	clientAddress;
	socklen_t			clientAddressLength = sizeof(sockaddr_in);

	clientFd = accept(serverSocket, reinterpret_cast<sockaddr*> (&clientAddress), \
		&clientAddressLength);
	if (clientFd == -1)
	{
		std::cerr << "Client was not accepted." << std::endl;
		return ;
	}
	if (fcntl(clientFd, F_SETFL, fcntl(clientFd, F_GETFL, 0) | O_NONBLOCK) == -1)
		errorExit(strerror(errno), -1);
	pollDescriptors.push_back({clientFd, POLLIN, 0});
	requests.emplace(clientFd, HttpRequest{});
	std::cout << "Accepted new connection" << std::endl;
}

void	WebServer::loopadydoopady()
{
	while (serverShouldRun == true)
	{
		if (poll(pollDescriptors.data(), pollDescriptors.size(), 0) == -1)
			errorExit("Poll function failed", -1);
		for (size_t i = 0; i < pollDescriptors.size(); i++)
		{
			if ((pollDescriptors[i].revents & POLLIN) != 0)
			{
				if (isServerSocket(pollDescriptors[i].fd) == true)
				{
					acceptConnection(pollDescriptors[i].fd);
				}
				else
				{
					handleClientRead(pollDescriptors[i].fd);
				}
			}
			else if ((pollDescriptors[i].revents & POLLOUT) != 0)
			{
				handleClientWrite(pollDescriptors[i].fd);
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