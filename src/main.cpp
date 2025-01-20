#include "WebServer.hpp"

void	startWebserver(const std::string& config)
{
	while (FOREVER)
	{
		try
		{
			WebServer	webserver;
			webserver.parseConfigurations(config);
			webserver.startTheThing();
		}
		catch (std::exception& e)
		{
			std::cerr << "A tragic error has occured 😭 " << e.what() << std::endl;
			std::exit(1);
		}
	}
}

int	main(int argc, char** argv)
{
	std::string	config;

	if (argc > 2)
		std::cout << "Please run the Webserver as default (no arguments) or provide a configuration file" << std::endl;
	if (argc == 1)
		config = "default.conf";
	else
		config = argv[1];
	startWebserver(config);
}