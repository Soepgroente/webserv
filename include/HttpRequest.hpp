#pragma once
#include "WebServer.hpp"

struct	HttpRequest
{
	std::string					rawRequest;
	time_t						lastRead;
	size_t						contentLength;
	std::vector<std::string> 	splitRequest;
	std::string					host;
	std::string					port;
	std::string					method;
	std::string					path;
	std::string					protocol;
	std::string					contentType;
	bool						isValidRequest = true;

	std::string					response;
};

std::ostream&	operator<<(std::ostream& out, struct HttpRequest& p);
