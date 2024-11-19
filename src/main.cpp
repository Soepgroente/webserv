#include "Project.hpp"

int	main(int argc, char** argv)
{
	WebServer	webserver;
	std::string	config;
	
	if (argc > 2)
		std::cout << "Please run the Webserver as default (no arguments) or provide a configuration file" << std::endl;
	if (argc == 1)
		config = "default.conf";
	else
		config = argv[1];
	webserver.parseConfigurations(config);

}