#pragma once

#include <iostream>
#include <vector>

enum RequestStatus
{
	headerIsParsed,
	bodyIsParsed,
	requestIsInvalid = 400,
	requestNotFound = 404,
};

struct HttpRequest
{
	HttpRequest() = default;
	~HttpRequest() = default;
	HttpRequest(const HttpRequest& other);

	HttpRequest&	operator=(const HttpRequest& other);

	void			clear();

	std::string					rawRequest;
	size_t						contentLength;
	std::vector<std::string> 	splitRequest;
	std::string					connectionType;
	std::string					keepAlive;
	std::string					host;
	std::string					port;
	std::string					method;
	std::string					path;
	std::string					protocol;
	std::string					contentType;
	std::string					body;
	std::string					fileType;
	int							status;
};

std::ostream& operator<<(std::ostream& out, struct HttpRequest& p);