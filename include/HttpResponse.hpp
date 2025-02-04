#pragma once

#include <fstream>
#include <iostream>
#include <map>

struct HttpResponse
{
	HttpResponse() = default;
	~HttpResponse() = default;
	HttpResponse(const HttpResponse& other);

	HttpResponse&	operator=(const HttpResponse& other);

	std::string		buffer;
	static std::map<int, std::string> defaultResponses;
};
