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
	// if (server.port != 0)
	// 	errorExit("Invalid configuration file", lineCount);
	server.port = std::stoi(input);
}

static void	parseName(const std::string& input, Server& server, int& lineCount)
{
	if (server.serverName != "")
		errorExit("Invalid configuration file", lineCount);
	server.serverName = input;
}

static void	parseErrorPage(const std::string& input, Server& server, int& lineCount)
{
	if (server.errorLocation != "")
		errorExit("Invalid configuration file", lineCount);
	server.errorLocation = input;
}

static void	parseBodySize(const std::string& input, Server& server, int& lineCount)
{
	// if (server.bodySize != 0)
	// 	errorExit("Invalid configuration file", lineCount);
	server.bodySize = std::stoi(input);
}

// static void	parseLocationRoot()
// {

// }

static void	parseLocation(const std::string& input, Server& server, int& lineCount)
{
	std::string			line;
	std::string 		path;
	Location			location;
	std::string	tmp;

	line = input.substr(0, line.find_first_of("{") - 1);
	path = line;
	server.locations.insert({path, location});
	while (file.eof() == false)
	{
		std::getline(file, line, '\n');
		lineCount++;
		line.erase(0, line.find_first_not_of(" \t\r\n"));
		line.erase(line.find_last_not_of(" \t\r\n") + 1, std::string::npos);
		if (line == "}")
			break;
		std::stringstream	s_line;
		s_line.str(line);
		std::cout << "---------\n" << s_line.str() << std::endl;
		s_line >> tmp;
		std::cout << "---------\n|" << tmp << '|' << std::endl;
		if (tmp == "methods")
		{
			if (server.locations[path].methods.empty() == false)
				errorExit("Invalid configuration file", lineCount);
			while (s_line >> tmp)
				server.locations[path].methods.push_back(tmp);
			continue;		
		}
		if (tmp == "cgi_extensions")
		{
			if (server.locations[path].cgiExtensions.empty() == false)
				errorExit("Invalid configuration file", lineCount);
			while (s_line >> tmp)
				server.locations[path].cgiExtensions.push_back(tmp);
			continue;
		}
		std::map<std::string, std::string>& tmp_map = server.locations[path].dirs;
		std::map<std::string, std::string>::iterator it;
		it = tmp_map.find(tmp);
		if (it == tmp_map.end())
			errorExit("Invalid location variable", lineCount);
		s_line >> tmp;
		it->second = tmp;
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
		pos = line.find_first_of(' ');
		try
		{
			parseFunctions[line.substr(0, pos)](&line[pos + 1], server, lineCount);
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			errorExit("Error in configuration file", lineCount);
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
