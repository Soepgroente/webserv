#pragma once

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Server.hpp"
#include "WebServer.hpp"

#define DEFAULT_TIMEOUT 10000

enum clientStatus
{
	launchCgi,
	LISTENING,
	RESPONDING,
	CLOSING,
	readingFromFile,
	showDirectory
};

struct HttpRequest;

class Client
{
	public:
	
	Client() = delete;
	~Client();
	Client(const Server& in);
	Client(const Client& other);
	Client& operator=(const Client& other);

	int		getFd() const;
	int		getFileFd() const;
	void	setFileFd(int newFd);
	void	initializeSocket(int newFd);

	const Server&	getServer() const;
	HttpRequest&	getRequest();
	HttpResponse&	getResponse();
	int64_t			getLatestPing() const;
	void			setPingTime();
	void			setTimeout(int64_t newTimeout);
	int64_t			getTimeout() const;
	void			setRemainingRequests(int input);
	int				getRemainingRequests() const;
	int				getClientStatus() const;
	void			setClientStatus(clientStatus newStatus);

	void			readFromFile();
	void			writeToClient();
	void			readIncomingRequest();
	void			handleOutgoingState();

	static std::vector<pollfd>	fileAndCgiDescriptors;

	private:

	void	launchCGI();

	void	parseDirectory();

	void	interpretRequest();
	bool	parseHeaders();
	bool	getContentType(const std::string& requestLine);
	bool	getContentLength(const std::string& requestLine);
	bool	getChunked(const std::string& requestLine);
	bool	getHost(const std::string& requestLine);
	bool	getKeepAlive(const std::string& requestLine);
	bool	getConnectionType(const std::string& requestLine);
	bool	getMethods(const std::string& requestLine);

	int64_t				latestPing;
	int64_t				timeout;
	size_t				writePos;
	size_t				readPos;
	int					remainingRequests;
	clientStatus		status;
	int					fd;
	int					fileFd;
	HttpRequest			request;
	HttpResponse		response;
	const Server&		server;
	
};

std::ostream&	operator<<(std::ostream& out, const Client& p);