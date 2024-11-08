/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   WebServerHandleErrors.cpp                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: akasiota <akasiota@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/11/06 13:15:06 by akasiota      #+#    #+#                 */
/*   Updated: 2024/11/08 16:25:18 by akasiota      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

static std::string getStatusMessage(const std::string& status_code)
{
	if (status_code == "404")
		return "Not Found";
	if (status_code == "500")
		return "Internal Server Error";
	if (status_code == "403")
		return "Forbidden";
	if (status_code == "400")
		return "Bad Request";
	if (status_code == "413")
		return "Payload Too Large";
	if (status_code == "405")
		return "Method Not Allowed";
	if (status_code == "301")
		return "Moved Permanently";
	return "OK";
}

std::string	constructErrorResponse(const std::string& status_code, const serverConf_s& server)
{
	std::string	status_message = getStatusMessage(status_code);
	// Check if there is a default error page
	if (!server.error_page.empty())
	{
		std::string		error_page_path = server.error_page + "/" + status_code + ".html"; // e.g. ./error_pages/404.html
		std::ifstream	file(error_page_path, std::ios::binary);
		if (file.is_open())
		{
			std::stringstream 	buffer;
			std::ostringstream	response;
			buffer << file.rdbuf();
			std::string	content = buffer.str();
			response << "HTTP/1.1 " << status_code << " " << status_message << "\r\n";
			response << "Content-Length: " << content.size() << "\r\n";
			response << "Content-Type: text/html\r\n\r\n";
			response << content;
			return response.str();	
		}
	}
	std::ostringstream	response;
	response << "HTTP/1.1" << status_code << " " << status_message << "\r\n";
	response << "Content-Length: " << status_message.length() << "\r\n";
	response << "Content-Type: text/plain\r\n\r\n";
	response << status_message;
	return response.str();
}

std::string WebServer::getDefaultPage(const std::string& page_path, const std::string& status_code, const serverConf_s& server)
{
	std::ifstream file(page_path);
	if (!file.is_open())
		return constructErrorResponse("500", server);
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string content = buffer.str();
	std::ostringstream response;
	response << "HTTP/1.1 " << status_code << " " << getStatusMessage(status_code) << "\r\n";
	response << "Content-Length: " << content.size() << "\r\n";
	response << "Content-Type: text/html\r\n\r\n";
	response << content;
	return response.str();
}
