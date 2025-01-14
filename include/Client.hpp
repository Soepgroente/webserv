#include <iostream>

#include "HttpRequest.hpp"

enum cgiStatus
{
	cgiIsFalse,
	launchCgi,
	parseCgi
};

class Client
{
	public:
	
	Client();
	~Client();
	Client(const Client& other) = delete;
	Client& operator=(const Client& other) = delete;

	int		getFd() const;
	void	setFd(int newFd);
	void	setCgiStatus(int status);
	int		getCgiStatus() const;
	int		getCgiFd() const;
	void	setCgiFd(int newFd);
	void	setRequest(const struct HttpRequest& newRequest);

	struct HttpRequest& getRequest();

	private:

	int					fd;
	int					cgiFd;
	struct HttpRequest	request;
	std::string			response;
	int					cgiStatus;
};