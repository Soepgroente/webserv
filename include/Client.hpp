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
	parseCgi,
	LISTENING,
	RESPONDING,
	CLOSING,
	readingFromFile,
	writingToFile,
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
	void			writeToFile();
	void			readIncomingRequest();
	void			handleOutgoingState();
	void			setupErrorPage(int error);

	static std::vector<pollfd>	fileAndCgiDescriptors;

	private:

	void	launchCGI();

	void	parseDirectory();

	void		interpretRequest();
	bool		parseHeaders();
	bool		parseContentType(const std::string& requestLine);
	bool		parseContentLength(const std::string& requestLine);
	bool		parseChunked(const std::string& requestLine);
	bool		parseHost(const std::string& requestLine);
	bool		parseKeepAlive(const std::string& requestLine);
	bool		parseConnectionType(const std::string& requestLine);
	bool		parseGet(const std::string& requestLine);
	bool		parsePost(const std::string& requestLine);
	bool		parseDelete(const std::string& requestLine);
	bool		parsePath(const std::string& requestLine);
	Location*	resolveRequestLocation(std::string& path);
	std::string	generateDirectoryListing(const std::filesystem::path& dir);


	void	reset();

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