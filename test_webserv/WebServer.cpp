/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   WebServer.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: akasiota <akasiota@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/09/24 15:29:02 by akasiota      #+#    #+#                 */
/*   Updated: 2024/10/10 18:14:13 by akasiota      ########   odam.nl         */
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
		std::stringstream	ss(line);
		std::string			key;
		if (line.find("server {") != std::string::npos)
		{
			new_server_conf = serverConf_s();
		}
		else if (line.find("}") != std::string::npos)
		{
			servers_confs.push_back(new_server_conf);
		}
		else if (ss >> key) // safeguard the other ss manipulations later
		{
			if (key == "host")
				ss >> new_server_conf.host;
			else if (key == "port")
				ss >> new_server_conf.port;
			else if (key == "server_name")
				ss >> new_server_conf.server_name;
			else if (key == "error page")
				ss >> new_server_conf.error_page;
			else if (key == "client_body_size")
				ss >> new_server_conf.client_body_size;
			else if (key == "location")
			{
				routeConf_s	route;
				std::string	path;
				ss >> path;
				while (std::getline(conf_ifstream, line) && line.find("}") == std::string::npos)
				{
					std::stringstream	route_ss(line);
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
					}
				}
				new_server_conf.routes[path] = route;
			}
			
		}
		
		
	}
	conf_ifstream.close();
	std::cout << "Closed the config file" << std::endl;

	return ;
}

// int	WebServer::serverSocketInit(const std::string& host, int port)
// {
// 	int	server_fd;
// 	struct addrinfo hints, *res, *p;
// }


void	WebServer::start()
{
	struct sockaddr_in	server_addr;
	int	opt = 1;				

	/* Create server fd */
	server_fd = socket(AF_INET, SOCK_STREAM | O_NONBLOCK, 0);
	if (server_fd == -1)
	{
		std::cerr << "Failed to create server socket" << std::endl;
		handleError(strerror(errno));
	}
	
	/* Adjust socket options */
	if ((setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) < 0)
	{
		std::cerr << "Failed to set socket options" << std::endl;
		close(server_fd);
		handleError(strerror(errno));
	}
	
	/* Assign values to the sockaddr_in members */
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	/* Bind server_addr with socket */
	if (bind(server_fd, (const sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		std::cerr << "Failed to bind socket with address" << std::endl;
		close(server_fd);
		handleError(strerror(errno));
	}

	/* Start listening for requests */
	if (listen(server_fd, 10) < 0)
	{
		std::cerr << "Failed to listen" << std::endl;
		close(server_fd);
		handleError(strerror(errno));
	}
	
	/* Create a vector with all the pollfds */
	std::vector<struct pollfd>	poll_fds;
	struct pollfd				server_pollfd;

	/* Initialize server_pollfd and add it to the container */
	server_pollfd.fd = server_fd;
	server_pollfd.events = POLLIN;
	server_pollfd.revents = 0;
	poll_fds.push_back(server_pollfd);
	
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
				if (poll_fds[i].fd == server_fd)
				{
					acceptConnection(poll_fds);
				}
				else
				{
					handleClient(poll_fds[i]);
				}
			}
		}
	}
	std::cout << "Server is operational on port " << port << std::endl;
	
	close(server_fd);
	std::cout << "Server fd is closed" << std::endl;
	

	
}

void	WebServer::acceptConnection(std::vector<struct pollfd>& poll_fds)
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
		std::cerr << "Client accept error" << std::endl;
		handleError(strerror(errno));
	}
	/* Modify client_fd */
	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "fcntl client_fd error" << std::endl;
		handleError(strerror(errno));
	}

	/* Initialize client_pollfd and add it to the container */
	client_pollfd.fd = client_fd;
	client_pollfd.events = POLLIN;
	client_pollfd.revents = 0;
	poll_fds.push_back(client_pollfd);
	
	std::cout << "Accepted new connection" << std::endl;
}

void	WebServer::handleClient(struct pollfd&	client_pollfd)
{
	std::string	buffer(BUFFER_SIZE, '\0');
	int			bytes_read;
	std::string	response;
	
	bytes_read = read(client_pollfd.fd, &buffer[0], BUFFER_SIZE);
	if (bytes_read < 0)
	{
		std::cerr << "Client_fd read error" << std::endl;
		handleError(strerror(errno));
	}
	else if (bytes_read == 0)
	{
		close(client_pollfd.fd);
		client_pollfd.fd = -1;
		std::cout << "Client -> Connection closed" << std::endl;
	}
	else
	{
		/* Process request (parsing stage?)*/
		buffer.resize(bytes_read);
		std::cout << "Received: " << buffer << std::endl;
		/* Send response (CGI stage?)*/
		response = 	"HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!";
		if (write(client_pollfd.fd, &response[0], response.size()) < 0)
		{
			std::cerr << "Write to client_fd error" << std::endl;
			handleError(strerror(errno));
		}
	}
}

