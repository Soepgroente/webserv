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

#define DEFAULT_TIMEOUT 10

enum cgiStatus
{
	cgiIsFalse,
	parseCgi,
	launchCgi
};

enum clientStatus
{
	clientIsActive,
	clientShouldClose
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
	void	initializeSocket(int newFd);
	void	setCgiStatus(int status);
	int		getCgiStatus() const;
	int		getCgiFd() const;
	void	setCgiFd(int newFd);

	const Server&	getServer() const;
	HttpRequest&	getRequest();
	HttpResponse&	getResponse();
	time_t			getLatestPing() const;
	void			setPingTime();
	void			setTimeout(time_t newTimeout);
	time_t			getTimeout() const;
	time_t			getTime()	const;
	void			receivedRequest();
	void			setRemainingRequests(int input);
	int				getRemainingRequests() const;
	int				getClientStatus() const;

	private:

	time_t				latestPing;
	time_t				timeout;
	int					remainingRequests;
	int					status;
	int					fd;
	int					cgiFd;
	int					cgiStatus;
	HttpRequest			request;
	HttpResponse		response;
	const Server&		server;
};

std::ostream&	operator<<(std::ostream& out, const Client& p);