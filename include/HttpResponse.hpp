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

	void			clear();
	std::string		buffer;
	std::string		reply;
	static std::map<int, std::string> defaultResponses;
};
