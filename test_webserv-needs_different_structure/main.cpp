
#include "WebServer.hpp"

void	handleError(const std::string& error)
{
	// perror(error.c_str());
	std::cerr << error << "\n" << "-------------------" << std::endl;
	return;
}

int main(int argc, char* argv[])
{
	int 		port = DEFAULT_PORT;
	std::string	config_file;

	if (argc > 2)
	{
		std::cerr << "Usage: " << argv[0] << " or " << argv[0] << " [config file]" << std::endl;
		return 1;
	}
	if (argc == 1)
		config_file = "./webserv.conf";
	else
		config_file = argv[1];

	WebServer	server(port);
	server.loadConfig(config_file);
	server.printServerConfs();
	server.start();
	
	return 0;
}