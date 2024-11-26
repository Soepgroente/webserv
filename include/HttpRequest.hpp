#include "WebServer.hpp"

struct	HttpRequest
{
	std::string	req;
	time_t		lastRead;
};