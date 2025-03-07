#include "HttpRequest.hpp"

HttpRequest::HttpRequest(const HttpRequest& other)
{
	*this = other;
}

HttpRequest&	HttpRequest::operator=(const HttpRequest& other)
{
	if (this != &other)
	{
		buffer = other.buffer;
		contentLength = other.contentLength;
		splitRequest = other.splitRequest;
		host = other.host;
		port = other.port;
		method = other.method;
		path = other.path;
		protocol = other.protocol;
		contentType = other.contentType;
		fileType = other.fileType;
		body = other.body;
		status = other.status;
		action = other.action;
		chunked = other.chunked;
		boundary = other.boundary;
		location = other.location;
		locationPath = other.locationPath;
	}
	return (*this);
}

void	HttpRequest::clear()
{
	buffer.clear();
	contentLength = 0;
	splitRequest.clear();
	host.clear();
	port.clear();
	method.clear();
	path.clear();
	protocol.clear();
	contentType.clear();
	action.clear();
	fileType.clear();
	status = defaultStatus;
	chunked = false;
	boundary.clear();
	body.clear();
	location = nullptr;
	locationPath.clear();
}

std::ostream&	operator<<(std::ostream& out, struct HttpRequest& p)
{
	out << "contentLength: " << p.contentLength << std::endl;
	out << "connectionType: " << p.connectionType << std::endl;
	out << "keepAlive: " << p.keepAlive << std::endl;
	out << "host: " << p.host << std::endl;
	out << "port: " << p.port << std::endl;
	out << "method: " << p.method << std::endl;
	out << "path: " << p.path << std::endl;
	out << "protocol: " << p.protocol << std::endl;
	out << "content type: " << p.contentType << std::endl;
	out << "file type: " << p.fileType << std::endl;
	out << "status: " << p.status << std::endl;
	out << "action: " << p.action << std::endl;
	out << "chunked: " << p.chunked << std::endl;
	out << "boundary: " << p.boundary << std::endl;
	out << "body size: " << p.body.size() << std::endl;
	return (out);
}