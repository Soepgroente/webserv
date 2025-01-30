#pragma once

#include <algorithm>
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
	parseCgi,
	launchCgi,
	writeCgiResults,
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
	int		getCgiFd() const;
	void	setCgiFd(int newFd);

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
	void			handleIncomingRequest();
	void			readIncomingRequest();

	private:

	bool	requestIsFinished();

	void	interpretRequest();
	bool	parseHeaders();
	bool	getContentType(size_t i);
	bool	getContentLength(size_t i);
	bool	getHost(size_t i);
	bool	getKeepAlive(size_t i);
	bool	getConnectionType(size_t i);
	bool	getMethods(size_t i);

	int64_t				latestPing;
	int64_t				timeout;
	size_t				writePos;
	int					remainingRequests;
	clientStatus		status;
	int					fd;
	int					fileFd;
	int					cgiFd;
	HttpRequest			request;
	HttpResponse		response;
	const Server&		server;
	struct pollfd*		pollDescriptor;
};

std::ostream&	operator<<(std::ostream& out, const Client& p);