/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   WebServer.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: akasiota <akasiota@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/09/27 13:38:25 by akasiota      #+#    #+#                 */
/*   Updated: 2024/10/10 18:10:15 by akasiota      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

# include <iostream>
# include <sstream>
# include <stdio.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <errno.h>
# include <string.h>
# include <unistd.h>
# include <fcntl.h>
# include <fstream>
# include <poll.h>
# include <vector>
# include <map>

# define DEFAULT_PORT 8081
# define BUFFER_SIZE 1024
# define CLIENTS_MAX 100

struct routeConf_s
{
	std::vector<std::string>	methods;
	std::string					redirection;
	std::string					root;
	std::string					upload_dir;
};

struct serverConf_s
{
	std::string	host;
	int			port;
	std::string	server_name;
	std::string	error_page;
	int			client_body_size;
	std::map<std::string, routeConf_s> routes;
};

void	handleError(const std::string& error);

class WebServer
{
	private:
		int port;
		int server_fd;
		
		std::vector<serverConf_s>	servers_confs;
		std::vector<int>			server_fds;
		std::vector<struct pollfd>	poll_fds;
		// int	serverSocketInit(const std::string& host, int port);
	public:
		// OCF
		WebServer();
		~WebServer();
		WebServer(WebServer& original);
		WebServer& operator=(WebServer& original);

		WebServer(int port);
		void	loadConfig(const std::string& config_file);
		void	start();
		void	acceptConnection(std::vector<struct pollfd>& poll_fds);
		void	handleClient(struct pollfd&	client_pollfd);
};

#endif