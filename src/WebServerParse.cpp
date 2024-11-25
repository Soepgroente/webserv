#include "WebServer.hpp"
#include "Server.hpp"

std::ifstream	file;

static void	parseHost(const std::string& input, Server& server, int& lineCount)
{
	if (server.host != "")
	{
		file.close();
		errorExit("Invalid configuration file", lineCount);
	}
	server.host = input;
}

static void	parsePort(const std::string& input, Server& server, int& lineCount)
{
	if (server.port != 0)
	{
		file.close();
		errorExit("Invalid configuration file", lineCount);
	}
	server.port = std::stoi(input);
}

static void	parseName(const std::string& input, Server& server, int& lineCount)
{
	if (server.serverName != "")
	{
		file.close();
		errorExit("Invalid configuration file", lineCount);
	}
	server.serverName = input;
}

static void	parseErrorPage(const std::string& input, Server& server, int& lineCount)
{
	if (server.errorLocation != "")
	{
		file.close();
		errorExit("Invalid configuration file", lineCount);
	}
	server.errorLocation = input;
}

static void	parseBodySize(const std::string& input, Server& server, int& lineCount)
{
	if (server.bodySize != -1)
	{
		file.close();
		errorExit("Invalid configuration file", lineCount);
	}
	server.bodySize = std::stoi(input);
}

static void	removeWhiteSpaces(std::string& line)
{
	line.erase(0, line.find_first_not_of(" \t\r\n"));
	line.erase(line.find_last_not_of(" \t\r\n") + 1, std::string::npos);
}


static void	parseLocation(const std::string& input, Server& server, int& lineCount)
{
	std::string	line;
	std::string path;
	Location	location;
	std::string	tmp;

	line = input.substr(0, input.size() - 1);
	removeWhiteSpaces(line);
	path = line;
	server.locations.insert({line, location});
	while (file.eof() == false)
	{
		std::getline(file, line, '\n');
		removeWhiteSpaces(line);
		lineCount++;
		if (line == "}")
			break;
		std::stringstream	s_line;
		s_line.str(line);
		s_line >> tmp;
		if (tmp == "methods")
		{
			if (server.locations[path].methods.empty() == false)
			{
				file.close();
				errorExit("Invalid configuration file", lineCount);
			}
			while (s_line >> tmp)
				server.locations[path].methods.push_back(tmp);
		}
		else if (tmp == "cgi_extensions")
		{
			if (server.locations[path].cgiExtensions.empty() == false)
			{
				file.close();
				errorExit("Invalid configuration file", lineCount);
			}
			while ((s_line >> tmp).eof() == false)
				server.locations[path].cgiExtensions.push_back(tmp);
		}
		else
		{
			// std::map<std::string, std::string>& tmp_map = server.locations[path].dirs;
			std::map<std::string, std::string>::iterator it;
			it = server.locations[path].dirs.find(tmp);
			if (it == server.locations[path].dirs.end())
			{
				file.close();
				errorExit("Invalid location variable", lineCount);
			}
			s_line >> tmp;
			it->second = tmp;
		}
	}
}

static Server	parseSingleServer(std::ifstream& file, int& lineCount)
{
	std::string			line("");
	Server				server{};
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

	server.bodySize = -1;
	while (line == "")
	{
		std::getline(file, line, '\n');
		lineCount++;
	}
	removeWhiteSpaces(line);
	if (line != "server {")
	{
		file.close();
		errorExit("Error in configuration file", lineCount);
	}
	while (file.eof() == false)
	{
		std::getline(file, line, '\n');
		removeWhiteSpaces(line);
		if (line == "}")
			return (server);
		pos = line.find_first_of(' ');
		parseFunctions[line.substr(0, pos)](&line[pos + 1], server, lineCount);
		lineCount++;
	}
	return (server);
}

void	WebServer::parseConfigurations(const std::string& fileLocation)
{
	std::stringstream	s_input;
	int					lineCount = 0;

	file.open("./configs/" + fileLocation, std::ifstream::in);
	if (file.is_open() == false)
	{
		errorExit("Configuration file " + fileLocation + " failed to open", -1);
	}
	while (file.eof() == false)
	{
		try
		{
			servers.push_back(parseSingleServer(file, lineCount));
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			file.close();
			errorExit("Error in configuration file", lineCount);
		}
	}
	file.close();
}
