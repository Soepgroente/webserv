#include "Webserv.hpp"

int	main(int argc, char** argv)
{
	if (argc > 2)
		std::cout << "Please run the Webserver as default (no arguments) or provide a configuration file" << std::endl;

	Webserver	Webserver;
}