#pragma once

#include <fstream>
#include <iostream>
#include <map>
#include "Server.hpp"

struct HttpResponse
{
	HttpResponse() = default;
	~HttpResponse() = default;
	HttpResponse(const HttpResponse& other);

	HttpResponse&	operator=(const HttpResponse& other);

	void	constructResponse(int status, const std::string& mimeType, size_t length);
	void	clear();

	std::string		buffer;
	std::string		reply;
	int 			status;
	size_t			cgiLength;
	size_t			cgiContentLength;
	Location*		location;
	static std::map<int, std::string> defaultResponses;
};
