#include "WebServer.hpp"

static void	setupSocket(int& sock)
{
	int	options = 1;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
		errorExit(strerror(errno), -1);
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &options, sizeof(options)) == -1)
		errorExit(strerror(errno), -1);
	if (fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK) == -1)
		errorExit(strerror(errno), -1);
}

static void	bindSocket(sockaddr_in& serverAddress, Server& server)
{
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(server.port);
	serverAddress.sin_addr.s_addr = INADDR_ANY;
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
		bindSocket(serverAddress, it);
		listenSocket(it.socket);
	}
	set_signals();
}