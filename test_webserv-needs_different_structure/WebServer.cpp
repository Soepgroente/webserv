/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   WebServer.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: akasiota <akasiota@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/09/24 15:29:02 by akasiota      #+#    #+#                 */
/*   Updated: 2024/11/11 17:36:30 by akasiota      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

WebServer::WebServer()
{
	std::cout << "WebServer Default Constructor called" << std::endl;
}

WebServer::~WebServer()
{
	std::cout << "WebServer Destructor called" << std::endl;
}

WebServer::WebServer(WebServer& original) : port(original.port), server_fd(original.server_fd)
{
	std::cout << "WebServer Copy Constructor called" << std::endl;
}

WebServer& WebServer::operator=(WebServer& original)
{
	if (this != &original)
	{
		port = original.port;
		server_fd = original.server_fd;
	}
	return *this;
}

WebServer::WebServer(int port) : port(port), server_fd(-1)
{
	std::cout << "WebServer Port Constructor called" << std::endl;
}

void	WebServer::loadConfig(const std::string& config_file)
{
	std::ifstream		conf_ifstream(config_file);
	std::string			line;
	serverConf_s		new_server_conf;
	
	if (!conf_ifstream.is_open())
	{
		std::cerr << "Failed to open the config file" << std::endl;
		exit(2);
	}
	std::cout << "Opened the config file" << std::endl;
	while (std::getline(conf_ifstream, line))
	{
		// Add some code to remove leading-trailing whitespace and skip empty lines and comments
		line.erase(0, line.find_first_not_of(" \t\r\n"));
		line.erase(line.find_last_not_of(" \t\r\n") + 1);
		if (line.empty() || line[0] == '#')
			continue;
			
		std::istringstream	ss(line);
		std::string			key;
		if (line.find("server {") != std::string::npos)
			new_server_conf = serverConf_s();
		else if (line.find("}") != std::string::npos)
			servers_confs.push_back(new_server_conf);
		else if (ss >> key) // safeguard the other ss manipulations later
		{
			if (key == "host")
				ss >> new_server_conf.host;
			else if (key == "port")
				ss >> new_server_conf.port;
			else if (key == "server_name")
				ss >> new_server_conf.server_name;
			else if (key == "error_page_dir")
				ss >> new_server_conf.error_page_dir;
			else if (key == "client_body_size")
				ss >> new_server_conf.client_body_size;
			else if (key == "location")
			{
				routeConf_s	route;
				std::string	path;
				ss >> path;
				while (std::getline(conf_ifstream, line) && line.find("}") == std::string::npos)
				{
					// Add some code to remove leading-trailing whitespace and skip empty lines and comments
					line.erase(0, line.find_first_not_of(" \t\r\n"));
					line.erase(line.find_last_not_of(" \t\r\n") + 1);
					if (line.empty() || line[0] == '#')
						continue;
					
					std::istringstream	route_ss(line);
					std::string			route_key;
					if (route_ss >> route_key)
					{
						if (route_key == "methods")
						{
							std::string	method;
							while (route_ss >> method)
								route.methods.push_back(method);
						}
						else if (route_key == "redirection")
							route_ss >> route.redirection;
						else if (route_key == "root")
							route_ss >> route.root;
						else if (route_key == "upload_dir")
							route_ss >> route.upload_dir;
						else if (route_key == "directory_listing")
						{
							std::string	str;
							route_ss >> str;
							// if (value == "on")
							// 	route.directory_listing = true;
							route.directory_listing = (str == "on");
						}
						else if (route_key == "index")
							route_ss >> route.index;
						else if (route_key == "cgi_extensions")
						{
							std::string	str;
							while (route_ss >> str)
								route.cgi_extensions.push_back(str);
														
						}
					}
				}
				new_server_conf.routes[path] = route;
			}	
		}
	}
	conf_ifstream.close();
	std::cout << "Closed the config file" << std::endl;
}

void	WebServer::start()
{	
	/* Initialize server sockets and add them to the server_fds vector */
	for (auto& server : servers_confs)
	{
		int	server_fd = serverSocketInit(server.host, server.port);
		if (server_fd < 0)
		{
			std::cerr << "Failed to create server socket" << std::endl;
			handleError("Server creation problem");
		}
		server_fds.push_back(server_fd);
		/* Initialize server_pollfd and add it to the poll_fds vector */
		struct pollfd server_pollfd = {server_fd, POLLIN, 0};
		poll_fds.push_back(server_pollfd);			
	}
	std::cout << "Servers are set\n---------------------" << std::endl;
	while (true)
	{
		if (poll(poll_fds.data(), poll_fds.size(), -1) < 0)
		{
			std::cerr << "Poll failed" << std::endl;
			close(server_fd);
			handleError(strerror(errno));
		}
		
		for (size_t i = 0; i < poll_fds.size(); i++)
		{
			if (poll_fds[i].revents & POLLIN)
			{
				if (isServerSocket(poll_fds[i].fd))
				{
					acceptConnection(poll_fds[i].fd);
				}
				else
				{
					handleClientRead(poll_fds[i], servers_confs);
				}
			}
			else if (poll_fds[i].revents & POLLOUT)
			{
				handleClientWrite(poll_fds[i]);
			}
		}
	}
	
	close(server_fd);
	std::cout << "Server fd is closed" << std::endl;
}

int	WebServer::serverSocketInit(const std::string& host, int port)
{
	int	server_fd;
	struct addrinfo hints = {};
	struct addrinfo *res, *tmp;
	int	opt = 1;
	int	ai_return;

	/* populate the addrinfo struct */
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	std::string	port_str = std::to_string(port);
	ai_return = getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res);
	if (ai_return != 0)
	{
		if (ai_return == EAI_SYSTEM)
		{
			std::cerr << "getaddrinfo failed" << std::endl;
			handleError(strerror(errno));
			return -1;
		}
		else
		{
			std::cerr << "getaddrinfo failed (else)" << std::endl;
			handleError(gai_strerror(ai_return));
			return -2;
		}
	}
	/* Build the different servers */
	for (tmp = res; tmp != nullptr; tmp = tmp->ai_next)
	{
		/* Create server fd */
		server_fd = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);
		if (server_fd == -1)
		{
			std::cerr << "Failed to create server socket" << std::endl;
			handleError(strerror(errno));
			continue;
		}
		/* Adjust socket options */
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
		{
			std::cerr << "Failed to set socket options" << std::endl;
			close(server_fd);
			handleError(strerror(errno));
			continue;
		}
		/* Set server_fd to nonblocking */
		if (fcntl(server_fd, F_SETFD, O_NONBLOCK) == -1)
		{
			std::cerr << "Failed to set the server to nonblocking mode" << std::endl;
			close(server_fd);
			handleError(strerror(errno));
			continue;
		}
		/* Bind server with socket */
		if (bind(server_fd, tmp->ai_addr, tmp->ai_addrlen) == -1)
		{
			std::cerr << "Failed to bind socket with address" << std::endl;
			close(server_fd);
			handleError(strerror(errno));
			continue;
		}
		break;
	}
	freeaddrinfo(res);
	if (tmp == nullptr)
	{
		std::cerr << "Failed to create socket on at least one address" << std::endl;
		close(server_fd);
		handleError("Server creation issue");
		return -3;
	}
	if (listen(server_fd, 10) == -1)
	{
		std::cerr << "Failed to listen" << std::endl;
		close(server_fd);
		handleError(strerror(errno));
		return -4;
	}
	return server_fd;
}

void	WebServer::acceptConnection(int	server_fd)
{
	struct sockaddr_in	client_addr;
	socklen_t			client_addrlen;
	int					client_fd;
	struct pollfd		client_pollfd;
	
	client_addrlen = sizeof(client_addr);
	/* Get client socket */
	client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addrlen);
	if (client_fd < 0)
	{
		if (errno != EWOULDBLOCK)
		{
			std::cerr << "Client accept error" << std::endl;
			handleError(strerror(errno));	
		}
		return;
	}
	/* Modify client_fd */
	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "fcntl client_fd error" << std::endl;
		close(client_fd);
		handleError(strerror(errno));
		return;
	}
	/* Initialize client_pollfd and add it to the vector */
	client_pollfd = {client_fd, POLLIN, 0};
	poll_fds.push_back(client_pollfd);
	
	std::cout << "Accepted new connection" << std::endl;
}
