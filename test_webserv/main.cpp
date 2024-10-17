
#include "WebServer.hpp"

void	handleError(const std::string& error)
{
	perror(error.c_str());
	return;
}

int main(int argc, char* argv[])
{
	int 				port = DEFAULT_PORT;
	const std::string	config_file = "./webserv.conf";

	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " [config file]" << std::endl;
		return 1;
	}

	WebServer	server(port);
	server.loadConfig(config_file);
	server.start();
	
	return 0;
}