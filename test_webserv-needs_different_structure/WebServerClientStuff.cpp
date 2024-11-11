/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   WebServerClientStuff.cpp                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: akasiota <akasiota@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/30 17:03:20 by akasiota      #+#    #+#                 */
/*   Updated: 2024/11/11 13:02:44 by akasiota      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

void	WebServer::handleClientRead(struct pollfd&	client_pollfd, std::vector<serverConf_s>& servers)
{
	std::string	buffer(BUFFER_SIZE, '\0');
	int			bytes_read;
	std::string	response;
	
	bytes_read = read(client_pollfd.fd, &buffer[0], BUFFER_SIZE);
	if (bytes_read < 0)
	{
		if (errno != EWOULDBLOCK)
		{
			std::cerr << "Client_fd read error" << std::endl;
			handleError(strerror(errno));
		}
	}
	else if (bytes_read == 0)
	{
		close(client_pollfd.fd);
		client_pollfd.fd = -1;
		std::cout << "Client -> Connection closed" << std::endl;
	}
	else
	{
		buffer.resize(bytes_read);
		std::cout << "Received: " << buffer << std::endl;
		response = handleRequest(buffer, servers);
		client_responses[client_pollfd.fd] = response;
		client_pollfd.events = POLLOUT;
		// response = 	"HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!";
	}
}

void	WebServer::handleClientWrite(struct pollfd& client_pollfd)
{
	std::string	response = client_responses[client_pollfd.fd];
	long		bytes_written = write(client_pollfd.fd, &response[0], response.size());
	if (bytes_written < 0)
	{
		handleError("Write to client_fd error");
		return ;
	}
	// std::cout << "wrote: " << bytes_written << std::endl;
	response.erase(0, bytes_written);
	if (response.empty())
	{
		client_pollfd.events = POLLIN;
		client_responses.erase(client_pollfd.fd);
	}
}
