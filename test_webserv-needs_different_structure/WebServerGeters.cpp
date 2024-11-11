/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   WebServerGeters.cpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: akasiota <akasiota@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/29 16:41:07 by akasiota      #+#    #+#                 */
/*   Updated: 2024/11/11 13:14:01 by akasiota      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

void	WebServer::printServerConfs() const
{
	std::cout << "-------------------------" << std::endl;
	std::cout << "  Server configurations" << std::endl;
	std::cout << "-------------------------" << std::endl;
	for (auto& server : servers_confs)
	{
		std::cout << "Host: " << server.host << std::endl;
		std::cout << "Port: " << server.port << std::endl;
		std::cout << "Server name: " << server.server_name << std::endl;
		std::cout << "Error page directory: " << server.error_page_dir << std::endl;
		std::cout << "Client_body_size: " << server.client_body_size << std::endl;
		std::cout << "-----Locations-----" << std::endl;
		for (auto& pair : server.routes)
		{
			std::cout << "Path: " << pair.first << std::endl;
			std::cout << "Methods:";
			for (auto& method : pair.second.methods)
				std::cout << ' ' << method;
			std::cout << std::endl;
			std::cout << "Redirection: " << pair.second.redirection << std::endl;
			std::cout << "Root: " << pair.second.root << std::endl;
			std::cout << "Upload dir: " << pair.second.upload_dir << std::endl;
		}
		std::cout << "--------------------------------------------------" << std::endl;
	}
}

bool	WebServer::isServerSocket(int fd) const
{
	for (auto& server_fd : server_fds)
	{
		if (fd == server_fd)
			return true;
	}
	return false;
}


