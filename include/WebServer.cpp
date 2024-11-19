#include "WebServer.hpp"

std::ifstream	file;

static void	errorExit(std::string errorMessage, int errorLocation)
{
	std::cerr << errorMessage << " on line " << errorLocation << std::endl;
	exit(EXIT_FAILURE);
}

static void	parseHost(const std::string& input, Server& server, int& lineCount)
{
	if (server.host != "")
		errorExit("Invalid configuration file", lineCount);
	server.host = input;
}

static void	parsePort(const std::string& input, Server& server, int& lineCount)
{
	;
}

static void	parseName(const std::string& input, Server& server, int& lineCount)
{
	;
}

static void	parseErrorPage(const std::string& input, Server& server, int& lineCount)
{
	;
}

static void	parseBodySize(const std::string& input, Server& server, int& lineCount)
{
	;
}

static void	parseLocation(const std::string& input, Server& server, int& lineCount)
{
	std::string	line;
	Location	location;

	line.erase(line.find_first_of("{") - 1, std::string::npos);
	server.locations.insert({line, location});
	while (file.eof() == false && line != "}")
	{
		std::getline(file, line, '\n');
		lineCount++;
		line.erase(0, line.find_first_not_of(" \t\r\n"));
		line.erase(line.find_last_not_of(" \t\r\n") + 1, std::string::npos);
		if (line == "methods")
		{

		}
		if (line == "cgi_extensions")
		{
			
		}
		try
		{

		}
		catch (std::exception& e)
		{
			errorExit("Invalid location variable", lineCount);
		}
	}

	
}

void	WebServer::printServerStruct(const Server& toPrint)	const
{
	std::cout << "host: " << toPrint.host << std::endl;
	std::cout << "Port: " << toPrint.port << std::endl;
	std::cout << "fd: " << toPrint.fd << std::endl;
	std::cout << "Name: " << toPrint.serverName << std::endl;
	std::cout << "Body size: " << toPrint.bodySize << std::endl;
	std::cout << "Error location: " << toPrint.errorLocation << "\n\n" << std::endl;
	for (const std::pair<std::string, Location>& pair : toPrint.locations)
	{
		std::cout << "Location: " << pair.first << std::endl;
		std::cout << "Methods: " << pair.first << std::endl;
		for (const std::string& iter : pair.second.methods)
		{
			std::cout << iter << ", ";
		}
		std::cout << std::endl;
		std::cout << "CGI extensions: " << pair.first << std::endl;
		for (const std::string& iter : pair.second.cgiExtensions)
		{
			std::cout << iter << ", ";
		}
		std::cout << std::endl;
		for (const std::pair<std::string, std::string>& despair : pair.second.dirs)
		{
			std::cout << despair.first << ": " << despair.second << std::endl;
		}
	}
}

static Server	parseSingleServer(std::ifstream& file, int& lineCount)
{
	std::string			line;
	std::stringstream	s_input;
	Server				server;
	size_t 				pos;

	std::map<std::string, std::function<void(const std::string&, Server&, int&)>>	parseFunctions = 
	{
		{"host", &parseHost},
		{"port", &parsePort},
		{"server_name", &parseName},
		{"error_page", &parseErrorPage},
		{"client_body_size", &parseBodySize},
		{"location", &parseLocation},
	};
	
	std::getline(file, line, '\n');
	lineCount++;
	if (line != "server {")
		errorExit("Error in configuration file", lineCount);
	while (file.eof() == false && line != "}")
	{
		std::getline(file, line, '\n');
		line.erase(0, line.find_first_not_of(" \t\r\n"));
		line.erase(line.find_last_not_of(" \t\r\n") + 1, std::string::npos);
		s_input.str(line);
		pos = line.find_first_of(' ');
		try
		{
			parseFunctions[line.substr(0, pos)](&line[pos + 1], server, lineCount);
		}
		catch (std::exception& e)
		{
			errorExit("Invalid variable name", lineCount);
		}
		lineCount++;
	}
	return (server);
}

void	WebServer::parseConfigurations(const std::string& fileLocation)
{
	std::string		line;
	std::stringstream	s_input;
	int				lineCount = 1;

	file.open("./configs/" + fileLocation, std::ifstream::in);
	if (file.is_open() == false)
	{
		errorExit("Configuration file " + fileLocation + " failed to open", -1);
	}
	while (file.eof() == false)
	{
		servers.push_back(parseSingleServer(file, lineCount));
		while (line == "")
		{
			std::getline(file, line, '\n');
			lineCount++;
		}
	}
	for (Server it : servers)
		printServerStruct(it);
	file.close();
}
