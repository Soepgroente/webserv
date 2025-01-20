/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   WebServer.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: akasiota <akasiota@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/09/27 13:38:25 by akasiota      #+#    #+#                 */
/*   Updated: 2024/11/19 13:17:05 by vvan-der      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

# include <iostream>
# include <sstream>
# include <stdio.h>
# include <netinet/in.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/wait.h>
# include <netdb.h>
# include <errno.h>
# include <string.h>
# include <unistd.h>
# include <fcntl.h>
# include <sys/stat.h>
# include <dirent.h>
# include <fstream>
# include <poll.h>
# include <vector>
# include <map>
# include <algorithm>
# include <array>
# include <filesystem>

# define DEFAULT_PORT 8081
# define BUFFER_SIZE 1024
# define CLIENTS_MAX 100

struct routeConf_s
{
	std::vector<std::string>	methods;
	std::string					redirection;
	std::string					root;
	std::string					upload_dir;
	bool						directory_listing = false;
	std::string					index;
	std::vector<std::string>	cgi_extensions;
};

struct serverConf_s
{
	std::string	host;
	int			port;
	std::string	server_name;
	std::string	error_page_dir;
	int			client_body_size;
	std::map<std::string, routeConf_s> routes;
};

void	handleError(const std::string& error);
std::string	constructErrorResponse(const std::string& status_code, const serverConf_s& server);


class WebServer
{
	public:
		// OCF
		WebServer();
		~WebServer();
		WebServer(WebServer& original);
		WebServer& operator=(WebServer& original);

		WebServer(int port);
		void	loadConfig(const std::string& config_file);
		void	start();
		void	acceptConnection(int server_fd);
		void	handleClientRead(struct pollfd&	client_pollfd, std::vector<serverConf_s>& servers);
		void	handleClientWrite(struct pollfd& client_pollfd);
		std::string	handleRequest(const std::string& request, const std::vector<serverConf_s>& servers);
		std::string getDefaultPage(const std::string& page_path, const std::string& status_code, const serverConf_s& server);


		// Geters
		void	printServerConfs() const;
		bool	isServerSocket(int fd) const;

	private:
		int port;
		int server_fd;
		
		std::vector<serverConf_s>	servers_confs;
		std::vector<int>			server_fds;
		std::vector<struct pollfd>	poll_fds;
		std::map<int, std::string>	client_responses;
		int	serverSocketInit(const std::string& host, int port);
};

#endif