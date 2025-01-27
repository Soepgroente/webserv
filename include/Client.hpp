#pragma once

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
	clientIsActive,
	clientShouldRespond,
	clientShouldClose,
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
	void			receivedRequest();
	void			setRemainingRequests(int input);
	int				getRemainingRequests() const;
	int				getClientStatus() const;
	void			setClientStatus(int newStatus);

	private:

	int64_t				latestPing;
	int64_t				timeout;
	int					remainingRequests;
	int					status;
	int					fd;
	int					fileFd;
	int					cgiFd;
	HttpRequest			request;
	HttpResponse		response;
	const Server&		server;
};

std::ostream&	operator<<(std::ostream& out, const Client& p);