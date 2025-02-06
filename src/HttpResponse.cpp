#include "HttpResponse.hpp"

std::map<int, std::string> HttpResponse::defaultResponses =
{
	{200, "HTTP/1.1 200 OK\r\nContent-Length: 6\r\n\r\n200 OK"},
	{201, "HTTP/1.1 201 Created\r\nContent-Length: 8\r\n\r\n201 Created"},
	{204, "HTTP/1.1 204 No Content\r\nContent-Length: 10\r\n\r\n204 No Content"},

	{301, "HTTP/1.1 301 Moved Permanently\r\nContent-Length: 18\r\n\r\n301 Moved Permanently"},
	{302, "HTTP/1.1 302 Found\r\nContent-Length: 9\r\n\r\n302 Found"},
	{304, "HTTP/1.1 304 Not Modified\r\nContent-Length: 13\r\n\r\n304 Not Modified"},

	{400, "HTTP/1.1 400 Bad Request\r\nContent-Length: 15\r\n\r\n400 Bad Request"},
	{401, "HTTP/1.1 401 Unauthorized\r\nContent-Length: 12\r\n\r\n401 Unauthorized"},
	{403, "HTTP/1.1 403 Forbidden\r\nContent-Length: 10\r\n\r\n403 Forbidden"},
	{404, "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\n404 Not Found"},
	{405, "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 22\r\n\r\n405 Method Not Allowed"},
	{413, "HTTP/1.1 413 Content Too Large\r\nContent-Length: 17\r\n\r\n413 Content Too Large"},
	{414, "HTTP/1.1 414 URI Too Long\r\nContent-Length: 13\r\n\r\n414 URI Too Long"},

	{500, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 21\r\n\r\n500 Internal Server Error"},
	{501, "HTTP/1.1 501 Not Implemented\r\nContent-Length: 16\r\n\r\n501 Not Implemented"},
	{502, "HTTP/1.1 502 Bad Gateway\r\nContent-Length: 13\r\n\r\n502 Bad Gateway"},
	{503, "HTTP/1.1 503 Service Unavailable\r\nContent-Length: 19\r\n\r\n503 Service Unavailable"},
	{505, "HTTP/1.1 505 HTTP Version Not Supported\r\nContent-Length: 25\r\n\r\n505 HTTP Version Not Supported"}
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
