#include "WebServer.hpp"

void	WebServer::printServerStruct(const Server& toPrint)	const
{
	std::cout << "host: " << toPrint.host << std::endl;
	std::cout << "Port: " << toPrint.port << std::endl;
	std::cout << "fd: " << toPrint.fd << std::endl;
	std::cout << "Name: " << toPrint.serverName << std::endl;
	std::cout << "Body size: " << toPrint.bodySize << std::endl;
	std::cout << "Error location: " << toPrint.errorLocation << "\n" << std::endl;
	for (const std::pair<std::string, Location>& pair : toPrint.locations)
	{
		std::cout << "Location: " << pair.first << std::endl;
		std::cout << "Methods: ";
		for (const std::string& iter : pair.second.methods)
		{
			std::cout << iter << ", ";
		}
		std::cout << std::endl;
		std::cout << "CGI extensions: ";
		for (const std::string& iter : pair.second.cgiExtensions)
		{
			std::cout << iter << ", ";
		}
		std::cout << std::endl;
		for (const std::pair<std::string, std::string>& despair : pair.second.dirs)
		{
			std::cout << despair.first << ": " << despair.second << std::endl;
		}
		std::cout << "--------------------------------------------" << std::endl;
	}
	std::cout << "==============================================" << std::endl;
}