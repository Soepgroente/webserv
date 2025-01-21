#pragma once

#include <iostream>
#include <vector>

struct	HttpRequest
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
	std::string					host;
	std::string					port;
	std::string					method;
	std::string					path;
	std::string					protocol;
	std::string					contentType;
	std::string					body;
	std::string					fileType;
	bool						isValidRequest;
};

std::ostream&	operator<<(std::ostream& out, struct HttpRequest& p);
