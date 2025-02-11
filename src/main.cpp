#include "WebServer.hpp"

void	startWebserver(const std::string& config)
{
	printToLog("Initial Server Bootup");
	while (FOREVER)
	{
		try
		{
			WebServer	webserver;

			printToLog("WebServer starting up");
			webserver.parseConfigurations(config);
			webserver.startTheThing();
		}
		catch (std::exception& e)
		{
			printToLog("WebServer shutting down due to exception: " + std::string(e.what()));
			std::exit(EXIT_FAILURE);
		}
		std::exit(EXIT_SUCCESS);
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