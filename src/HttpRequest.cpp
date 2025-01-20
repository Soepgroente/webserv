#include "HttpRequest.hpp"

HttpRequest::HttpRequest(const HttpRequest& other)
{
	*this = other;
}

HttpRequest&	HttpRequest::operator=(const HttpRequest& other)
{
	if (this != &other)
	{
		rawRequest = other.rawRequest;
		lastRead = other.lastRead;
		contentLength = other.contentLength;
		splitRequest = other.splitRequest;
		host = other.host;
		port = other.port;
		method = other.method;
		path = other.path;
		protocol = other.protocol;
		contentType = other.contentType;
		body = other.body;
		fileType = other.fileType;
		isValidRequest = other.isValidRequest;
		response = other.response;
	}
	return (*this);
}

std::ostream&	operator<<(std::ostream& out, struct HttpRequest& p)
{
	out << "last read: " << p.lastRead << std::endl;
	out << "contentLength: " << p.contentLength << std::endl;
	out << "host: " << p.host << std::endl;
	out << "port: " << p.port << std::endl;
	out << "method: " << p.method << std::endl;
	out << "path: " << p.path << std::endl;
	out << "file type: " << p.fileType << std::endl;
	out << "protocol: " << p.protocol << std::endl;
	out << "content type: " << p.contentType << std::endl;
	out << "request is valid: " << p.isValidRequest << std::endl;
	out << "body: " << p.body << std::endl;
	out << "response: " << p.response << std::endl;
	return (out);
}