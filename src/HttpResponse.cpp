#include "HttpResponse.hpp"

std::map<int, std::string> HttpResponse::defaultResponses =
{
	{200, "HTTP/1.1 200 OK\r\nContent-Length: "},
	{201, "HTTP/1.1 201 Created\r\nContent-Type: image/jpeg\r\nContent-Length: "},
	{204, "HTTP/1.1 204 No Content\r\nContent-Type: image/jpeg\r\nContent-Length: "},

	{301, "HTTP/1.1 301 Moved Permanently\r\nContent-Type: image/jpeg\r\nContent-Length: "},
	{302, "HTTP/1.1 302 Found\r\nContent-Type: image/jpeg\r\nContent-Length: "},
	{304, "HTTP/1.1 304 Not Modified\r\nContent-Type: image/jpeg\r\nContent-Length: "},

	{400, "HTTP/1.1 400 Bad Request\r\nContent-Type: image/jpeg\r\nContent-Length: "},
	{401, "HTTP/1.1 401 Unauthorized\r\nContent-Type: image/jpeg\r\nContent-Length: "},
	{403, "HTTP/1.1 403 Forbidden\r\nContent-Type: image/jpeg\r\nContent-Length: "},
	{404, "HTTP/1.1 404 Not Found\r\nContent-Type: image/jpeg\r\nContent-Length: "},
	{405, "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: image/jpeg\r\nContent-Length: "},
	{413, "HTTP/1.1 413 Content Too Large\r\nContent-Type: image/jpeg\r\nContent-Length: "},
	{414, "HTTP/1.1 414 URI Too Long\r\nContent-Type: image/jpeg\r\nContent-Length: "},

	{500, "HTTP/1.1 500 Internal Server Error\r\nContent-Type: image/jpeg\r\nContent-Length: "},
	{501, "HTTP/1.1 501 Not Implemented\r\nContent-Type: image/jpeg\r\nContent-Length: "},
	{502, "HTTP/1.1 502 Bad Gateway\r\nContent-Type: image/jpeg\r\nContent-Length: "},
	{503, "HTTP/1.1 503 Service Unavailable\r\nContent-Type: image/jpeg\r\nContent-Length: "},
	{505, "HTTP/1.1 505 HTTP Version Not Supported\r\nContent-Type: image/jpeg\r\nContent-Length: "}
};

HttpResponse::HttpResponse(const HttpResponse& other)
{
	*this = other;
}

HttpResponse&	HttpResponse::operator=(const HttpResponse& other)
{
	if (this != &other)
	{
		buffer = other.buffer;
		reply = other.reply;
	}
	return (*this);
}

void	HttpResponse::clear()
{
	buffer.clear();
	reply.clear();
}

std::ostream&	operator<<(std::ostream& out, const HttpResponse& p)
{
	out << "Buffer: " << p.buffer << std::endl;
	return (out);
}
