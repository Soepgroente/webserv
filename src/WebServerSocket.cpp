#include "WebServer.hpp"

static uint32_t ipStringToInt(std::string ip)
{
	uint32_t	result = 0;

	for (int i = 0; i < 4; i++)
	{
		result |= std::stoi(ip.substr(0, ip.find('.'))) << (3 - i) * 8;
		ip.erase(0, ip.find('.') + 1);
	}
	return (result);
}

static int	setupSocket(int& sock)
{
	int	options = 1;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
		return (-1);
	#ifdef __APPLE__
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(options)) == -1)
	{
		close(sock);
		return (-1);
	}
	#elif defined(__linux__)
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &options, sizeof(options)) == -1)
	{
		close(sock);
		return (-1);
	}
	#endif
	return (fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK));
}

static int	bindSocket(sockaddr_in& serverAddress, Server& server, uint32_t ip)
{
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(server.port);
	serverAddress.sin_addr.s_addr = htonl(ip);

	return (bind(server.socket, reinterpret_cast<const sockaddr*> (&serverAddress), sizeof(serverAddress)));
}

void	WebServer::initialize()
{
	sockaddr_in	serverAddress{};

	for (Server& it : servers)
	{
		if (setupSocket(it.socket) == -1)
		{
			errorExit("Failed to set up socket", -1);
		}
		if (bindSocket(serverAddress, it, ipStringToInt(it.host)) == -1)
		{
			errorExit("Failed to bind socket on ip: " + it.host + ":" + std::to_string(it.port), -1);
		}
		if (listen(it.socket, SOMAXCONN) == -1)
		{
			errorExit("Failed to listen on socket", -1);
		}
	}
	set_signals();
}