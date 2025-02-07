#pragma once

#include <iostream>
#include <vector>

enum RequestStatus
{
	defaultStatus,
	headerIsParsed = 1,
	bodyIsParsed,
	requestIsInvalid = 400,
	requestForbidden = 403,
	requestNotFound,
	requestMethodNotAllowed,
};

struct	HttpRequest
{
	HttpRequest() = default;
	~HttpRequest() = default;
	HttpRequest(const HttpRequest& other);

	HttpRequest&	operator=(const HttpRequest& other);

	void			clear();

	std::string					buffer;
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
	std::string					fileType;
	int							status;
};

std::ostream&	operator<<(std::ostream& out, struct HttpRequest& p);
