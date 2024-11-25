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
	struct pollfd		clientPollFd;

	clientFd = accept(serverSocket, reinterpret_cast<sockaddr*> (&clientAddress), \
		&clientAddressLength);
	if (clientFd == -1)
	{
		std::cerr << "Client was not accepted." << std::endl;
		return ;
	}
	fcntl(clientFd, F_SETFL, fcntl(clientFd, F_GETFL, 0) | O_NONBLOCK);
	if (errno != 0)
		errorExit(strerror(errno), -1); // exit or keep running?
	pollDescriptors.push_back({clientFd, POLLIN, 0});
	std::cout << "Accepted new connection" << std::endl;
}

void	WebServer::loopadydoopady()
{
	while (FOREVER)
	{
		if (poll(pollDescriptors.data(), pollDescriptors.size(), -1) == -1)
			errorExit("Poll failed", -1);	// exit or keep running?
		for (pollfd it : pollDescriptors)
		{
			if ((it.revents & POLLIN) != 0)
			{
				if (isServerSocket(it.fd) == true)
				{
					acceptConnection(it.fd);
				}
				else
				{
					handleClientRead(it, servers_confs);
				}
			}
			else if ((it.revents & POLLOUT) != 0)
			{
				handleClientWrite(it);
			}
		}
	}
}

void	WebServer::startTheThing()
{
	initialize();
	pollDescriptors = createPollArray();
	loopadydoopady();
	// for (Server it : servers)
	// 	printServerStruct(it);
}