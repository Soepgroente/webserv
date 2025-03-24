#include "WebServer.hpp"

void	startWebserver(const std::string& config)
{
	printToLog("Initial Server Bootup");
	while (FOREVER)
	{
		// inspect this
		Client::cgiCounter = 0;
		try
		{
			WebServer	webserver;

			printToLog("WebServer starting up");
			webserver.parseConfigurations(config);
			webserver.startTheThing();
		}
		catch (std::exception& e)
		{
			if (std::string(e.what()) == "Error: shutting down server")
			{
				return ;
			}
			if (std::string(e.what()).substr(0, 27) == "Shutting down after signal ")
			{
				std::cerr << e.what() << std::endl;
				return ;
			}
			printToLog("WebServer shutting down due to exception: " + std::string(e.what()));
		}
	}
}

int	main(int argc, char** argv)
{
	std::string	config;

	if (argc > 2)
	{
		std::cout << "Please run the Webserver as default (no arguments) or provide a configuration file" << std::endl;
		return (1);
	}
	if (argc == 1)
	{
		config = "default.conf";
	}
	else
	{
		config = argv[1];
	}
	startWebserver(config);
	return (2);
}