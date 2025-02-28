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

static void	setupSocket(int& sock)
{
	int	options = 1;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
		errorExit(strerror(errno), -1);
	#ifdef __APPLE__
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(options)) == -1)
	{
		close(sock);
		errorExit(strerror(errno), -1);
	}
	#elif defined(__linux__)
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &options, sizeof(options)) == -1)
	{
		close(sock);
		errorExit(strerror(errno), -1);
	}
	#endif
	if (fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK) == -1)
		errorExit(strerror(errno), -1);
}

static void	bindSocket(sockaddr_in& serverAddress, Server& server, uint32_t ip)
{
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(server.port);
	serverAddress.sin_addr.s_addr = htonl(ip);

	if (bind(server.socket, reinterpret_cast<const sockaddr*> \
		(&serverAddress), sizeof(serverAddress)) == -1)
	{
		errorExit("Socket failed to bind", -1);
	}
}

static void	listenSocket(int& socket)
{
	if (listen(socket, 10) == -1)
		errorExit("Listen failed", -1);
}

void	WebServer::initialize()
{
	sockaddr_in	serverAddress{};

	serverShouldRun = true;
	for (Server& it : servers)
	{
		setupSocket(it.socket);
		bindSocket(serverAddress, it, ipStringToInt(it.host));
		listenSocket(it.socket);
	}
	set_signals();
}