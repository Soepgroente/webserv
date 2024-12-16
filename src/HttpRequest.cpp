#include "HttpRequest.hpp"

std::ostream&	operator<<(std::ostream& out, struct HttpRequest& p)
{
	out << "last read: " << p.lastRead << std::endl;
	out << "contentLength: " << p.contentLength << std::endl;
	out << "host: " << p.host << std::endl;
	out << "port: " << p.port << std::endl;
	out << "method: " << p.method << std::endl;
	out << "path: " << p.path << std::endl;
	out << "protocol: " << p.protocol << std::endl;
	out << "content type: " << p.contentType << std::endl;
	out << "request is valid: " << p.isValidRequest << std::endl;
	out << "header is parsed: " << p.headerIsParsed << std::endl;
	out << "body: " << p.body << std::endl;
	out << "response: " << p.response << std::endl;
	return (out);
}