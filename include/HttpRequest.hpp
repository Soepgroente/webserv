#pragma once

#include <iostream>
#include <vector>
#include "Server.hpp"

enum RequestStatus
{
	defaultStatus,
	headerIsParsed = 1,
	bodyIsParsed,
	requestIsOk = 200,
	requestCreated = 201,
	temporaryRedirect = 307,
	requestIsInvalid = 400,
	requestForbidden = 403,
	requestNotFound = 404,
	requestMethodNotAllowed = 405,
	requestTimeout = 408,
	fileAlreadyExists = 409,
	lengthRequired = 411,
	payloadTooLarge = 413,
	unsupportedMediaType = 415,
	tooManyRequests = 429,
	internalServerError = 500,
	versionNotSupported = 505,
	connectionTimeout = 522,
	serviceOverloaded = 529
};

struct	HttpRequest
{
	HttpRequest() = default;
	~HttpRequest() = default;
	HttpRequest(const HttpRequest& other);

	HttpRequest&	operator=(const HttpRequest& other);

	void			clear();

	std::string					buffer;
	std::string					body;
	size_t						contentLength;
	std::vector<std::string> 	splitRequest;
	std::string					connectionType;
	std::string					keepAlive;
	std::string					host;
	std::string					port;
	std::string					method;
	std::string					path;
	std::string					dotPath;
	std::string					protocol;
	std::string					contentType;
	std::string					fileType;
	std::string					boundary;
	std::string					action;
	int							status;
	bool 						chunked;
	Location*					location;
	std::string					locationPath;
};

std::ostream&	operator<<(std::ostream& out, struct HttpRequest& p);
