#pragma once

#include <iostream>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include "HttpRequest.hpp"
#include "Server.hpp"

enum cgiStatus
{
	cgiIsFalse,
	launchCgi,
	parseCgi
};

struct HttpRequest;

class Client
{
	public:
	
	Client() = delete;
	~Client();
	Client(int serverSocket, const Server& in);
	Client(const Client& other);
	Client& operator=(const Client& other);

	int		getFd() const;
	void	initializeSocket(int newFd);
	void	setCgiStatus(int status);
	int		getCgiStatus() const;
	int		getCgiFd() const;
	void	setCgiFd(int newFd);
	void	setRequest(const HttpRequest& newRequest);
	const Server&	getServer() const;

	HttpRequest& getRequest();

	private:

	int					fd;
	int					cgiFd;
	HttpRequest			request;
	std::string			response;
	int					cgiStatus;
	const Server&		server;
};