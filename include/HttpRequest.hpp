#pragma once
#include "WebServer.hpp"

struct HttpRequest
{
    std::string rawRequest;
    time_t lastRead;
    size_t contentLength;
    std::vector<std::string> splitRequest;
    std::string host;
    std::string port;
    std::string method;
    std::string path;
    std::string protocol;
    std::string contentType;
    std::string body;
    bool isValidRequest = false;
    std::string response;
    std::map<std::string, std::string> headers;

    std::string getHeader(const std::string& headerName) const
    {
        std::map<std::string, std::string>::const_iterator it = headers.find(headerName);
        if (it != headers.end())
        {
            return it->second;
        }
        return "";
    }
};

std::ostream& operator<<(std::ostream& out, struct HttpRequest& p);