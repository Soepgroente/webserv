/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   WebServerHandleRequests.cpp                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: akasiota <akasiota@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/30 17:46:31 by akasiota      #+#    #+#                 */
/*   Updated: 2024/10/31 17:10:14 by akasiota      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

std::string	handleRequest(const std::string& request)
{
	std::istringstream	request_s(request);
	std::string	method, path, version;
	
	request_s >> method >> path >> version;
	if (method == "GET")
		return handleGetRequest(path);
	else if (method == "POST")
		// return "HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\nPOST";
		return handlePostRequest(path, request_s);
	else if (method == "DELETE")
		return "HTTP/1.1 200 OK\r\nContent-Length: 6\r\n\r\nDELETE";
	else
		return "HTTP/1.1 405 Method not allowed\r\nContent-Length: 22\r\n\r\n405\nMethod not allowed";	
}

std::string	handleGetRequest(const std::string& path)
{
	if (path == "/")
	{
		return "HTTP/1.1 200 OK\r\nContent-Length: 8\r\n\r\nHomepage";
	}
	std::string		file_path = path.substr(1);
	std::ifstream	file(file_path);
	if (!file.is_open())
	{
		return "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\n404\nNot Found";
	}	
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string	content;
	content = buffer.str();
	std::stringstream response;
	response << "HTTP/1.1 200 OK\r\n";
	response << "Content-Length: " << content.size() << "\r\n\r\n";
	response << content;
	file.close();
	return response.str();
}

std::string	trimWhitespace(std::string& line)
{
	size_t	start = line.find_first_not_of(" \t\r\n");
	size_t	end = line.find_last_not_of(" \t\r\n");
	if (start == std::string::npos)
		return "";
	return (line.substr(start, end - start + 1));
}

void	parseHeaders(std::istringstream& request, std::map<std::string, std::string>& headers)
{
	std::string	line;
	std::string	trimmed_line;
	size_t		pos;

	while (std::getline(request, line))
	{
		trimmed_line = trimWhitespace(line);
		if (trimmed_line == "")
			break;
		pos = trimmed_line.find(": ");
		if (pos != std::string::npos)
		{
			std::string	key = trimmed_line.substr(0, pos);
			std::string	value = trimmed_line.substr(pos + 2);
			if (key.empty() || value.empty())
			{
				handleError("Invalid header");
				return ;
			}
			headers[key] = value;
		}
	}
}

std::string	handlePostRequest(const std::string& path, std::istringstream& request)
{
	std::map<std::string, std::string> headers;
	std::string	str_request = request.str();
	std::string file_content;
	
	parseHeaders(request, headers);
	if (headers.find("Content-Type") != headers.end())
	{
		if (headers["Content-Type"].find("application/x-www-form-urlencoded") != std::string::npos)
		{
			// adjust 
			return "HTTP/1.1 200 OK\r\nContent-Length: 11\r\n\r\nURL-ENCODED";
		}
		else if (headers["Content-Type"].find("multipart/form-data") != std::string::npos)
		{
			std::string boundary = "--" + headers["Content-Type"].substr(headers["Content-Type"].find("boundary=") + 9);
			std::string	body = str_request.substr(str_request.find("/r/n/r/n") + 4);
			size_t		start = body.find(boundary) + boundary.size() + 2;
			size_t		end = body.find(boundary, start) - 4;
			file_content = body.substr(start, end - start);
		}
		else
		{	
			// Error - adjust
			return "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 25\r\n\r\n500\nInternal Server Error";
		}
	}
	else
	{
		// Error - adjust
		return "HTTP/1.1 400 Bad Request\r\nContent-Length: 15\r\n\r\n400\nBad Request";
	}
	std::string		upload_dir = "./uploads"; // this needs to be the upload directory of the server
	std::string		filename = upload_dir + path;
	std::ofstream	file(filename, std::ios::binary);
	if (!file.is_open())
	{
		return "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 25\r\n\r\n500\nInternal Server Error";
	}
	file.write(file_content.c_str(), file_content.size());
	file.close();
	

	return "HTTP/1.1 200 OK\r\nContent-Length:11\r\n\r\nUpload done";
}
