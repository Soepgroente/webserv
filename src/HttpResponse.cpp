#include "HttpResponse.hpp"

std::map<int, std::string> HttpResponse::defaultResponses =
{
	{200, "HTTP/1.1 200 OK\r\nContent-Length: 6\r\n\r\n200\nOK\r\n"},
	{201, "HTTP/1.1 201 Created\r\nContent-Length: 0\r\n\r\n"},
	{204, "HTTP/1.1 204 No Content\r\nContent-Length: 0\r\n\r\n"},

	{301, "HTTP/1.1 301 Moved Permanently\r\nContent-Length: 0\r\n\r\n"},
	{302, "HTTP/1.1 302 Found\r\nContent-Length: 0\r\n\r\n"},
	{304, "HTTP/1.1 304 Not Modified\r\nContent-Length: 0\r\n\r\n"},

	{400, "HTTP/1.1 400 Bad Request\r\nContent-Length: 15\r\n\r\n400\nBad Request\r\n"},
	{401, "HTTP/1.1 401 Unauthorized\r\nContent-Length: 0\r\n\r\n"},
	{403, "HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\n\r\n"},
	{404, "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\n404\nNot Found\r\n"},
	{405, "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 22\r\n\r\n405\nMethod Not Allowed\r\n"},
	{413, "HTTP/1.1 413 Content Too Large\r\nContent-Length: 0\r\n\r\n"},
	{414, "HTTP/1.1 414 URI Too Long\r\nContent-Length: 0\r\n\r\n"},

	{500, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n"},
	{501, "HTTP/1.1 501 Not Implemented\r\nContent-Length: 0\r\n\r\n"},
	{502, "HTTP/1.1 502 Bad Gateway\r\nContent-Length: 0\r\n\r\n"},
	{503, "HTTP/1.1 503 Service Unavailable\r\nContent-Length: 0\r\n\r\n"},
	{505, "HTTP/1.1 505 HTTP Version Not Supported\r\nContent-Length: 0\r\n\r\n"}
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
	}
	return (*this);
}

std::ostream&	operator<<(std::ostream& out, const HttpResponse& p)
{
	out << "Buffer: " << p.buffer << std::endl;
	return (out);
}
