#include "HttpResponse.hpp"
#include "HttpRequest.hpp"

std::map<int, std::string> HttpResponse::defaultResponses =
{
	{200, "HTTP/1.1 200 OK\r\nContent-Type: "},
	{201, "HTTP/1.1 201 Created\r\nContent-Type: "},
	{204, "HTTP/1.1 204 No Content\r\nContent-Type: "},

	{301, "HTTP/1.1 301 Moved Permanently\r\nContent-Type: "},
	{302, "HTTP/1.1 302 Found\r\nContent-Type: "},
	{307, "HTTP/1.1 307 Temporary Redirect\r\nContent-Type: "},

	{400, "HTTP/1.1 400 Bad Request\r\nContent-Type: "},
	{401, "HTTP/1.1 401 Unauthorized\r\nContent-Type: "},
	{403, "HTTP/1.1 403 Forbidden\r\nContent-Type: "},
	{404, "HTTP/1.1 404 Not Found\r\nContent-Type: "},
	{405, "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: "},
	{409, "HTTP/1.1 409 Conflict\r\nContent-Type: "},
	{411, "HTTP/1.1 411 Length Required\r\nContent-Type: "},
	{413, "HTTP/1.1 413 Payload Too Large\r\nContent-Type: "},
	{414, "HTTP/1.1 414 URI Too Long\r\nContent-Type: "},
	{415, "HTTP/1.1 415 Unsupported Media Type\r\nContent-Type: "},
	
	{500, "HTTP/1.1 500 Internal Server Error\r\nContent-Type: "},
	{501, "HTTP/1.1 501 Not Implemented\r\nContent-Type: "},
	{502, "HTTP/1.1 502 Bad Gateway\r\nContent-Type: "},
	{503, "HTTP/1.1 503 Service Unavailable\r\nContent-Type: "},
	{505, "HTTP/1.1 505 HTTP Version Not Supported\r\nContent-Type: "},
	{522, "HTTP/1.1 522 Connection Timed Out\r\nContent-Type: "}
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
		status = other.status;
		cgiContentLength = other.cgiContentLength;
	}
	return (*this);
}

void	HttpResponse::clear()
{
	buffer.clear();
	reply.clear();
	status = defaultStatus;
	cgiContentLength = 0;
}

std::ostream&	operator<<(std::ostream& out, const HttpResponse& p)
{
	out << "Buffer: " << p.buffer << std::endl;
	out << "Reply: " << p.reply << std::endl;
	out << "Status: " << p.status << std::endl;
	out << "CGI Content Length: " << p.cgiContentLength << std::endl;
	return (out);
}

void	HttpResponse::constructResponse(int status, const std::string& mimeType, size_t length)
{
	if (mimeType == "unsupported")
	{
		reply = defaultResponses[status] + "text/html\r\nContent-Length: 0\r\n\r\n";
	}
	else if (status == temporaryRedirect)
	{
		reply = defaultResponses[status] + mimeType + "\r\nRefresh: 3; url=" + location->dirs.at("redirection") + "\r\nContent-Length: " + std::to_string(length) + "\r\nLocation: " +  + "\r\n\r\n" + buffer;
	}
	else
	{
		reply = defaultResponses[status] + mimeType + "\r\nContent-Length: " + std::to_string(length) + "\r\n\r\n" + buffer;
	}
	buffer.clear();
}
