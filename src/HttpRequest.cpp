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
		body = other.body;
		fileType = other.fileType;
		status = other.status;
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
	body.clear();
	fileType.clear();
	status = requestIsInvalid;
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
	out << "body: " << p.body << std::endl;
	out << "file type: " << p.fileType << std::endl;
	out << "status: " << p.status << std::endl;
	return (out);
}