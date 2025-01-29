#include "WebServer.hpp"
#include "Server.hpp"

std::ifstream	file;

static void	closeAndExit(std::string message, int& lineCount)
{
	file.close();
	errorExit(message, lineCount);
}

static void	parseHost(const std::string& input, Server& server, int& lineCount)
{
	if (server.host != "")
		closeAndExit("Invalid configuration file", lineCount);
	server.host = input;
}

static void	parsePort(const std::string& input, Server& server, int& lineCount)
{
	if (server.port != 0)
		closeAndExit("Invalid configuration file", lineCount);
	server.port = std::stoi(input);
}

static void	parseName(const std::string& input, Server& server, int& lineCount)
{
	if (server.serverName != "")
		closeAndExit("Invalid configuration file", lineCount);
	server.serverName = input;
}

static void	parseErrorPage(const std::string& input, Server& server, int& lineCount)
{
	if (server.errorLocation != "")
		closeAndExit("Invalid configuration file", lineCount);
	server.errorLocation = input;
}

static void	parseBodySize(const std::string& input, Server& server, int& lineCount)
{
	if (server.maxBodySize != -1)
		closeAndExit("Invalid configuration file", lineCount);
	server.maxBodySize = std::stoi(input);
	if (server.maxBodySize > MAXBODYSIZE)
		closeAndExit("Invalid configuration file (body size too large)", lineCount);
}

static void	removeWhiteSpaces(std::string& line)
{
	line.erase(0, line.find_first_not_of(" \t\r\n"));
	line.erase(line.find_last_not_of(" \t\r\n") + 1, std::string::npos);
}

template<typename Container>
static void	checkIfEmpty(const Container& var, bool (Container::*method)() const, int& lineCount)
{
	if ((var.*method)() == false)
		closeAndExit("Invalid configuration file", lineCount);
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
		std::stringstream	s_line;

		std::getline(file, line, '\n');
		removeWhiteSpaces(line);
		lineCount++;
		if (line == "}")
			break;
		s_line.str(line);
		s_line >> tmp;
		if (tmp == "methods")
		{
			checkIfEmpty(server.locations[path].methods, &std::vector<std::string>::empty, lineCount);
			while (s_line >> tmp)
				server.locations[path].methods.push_back(tmp);
		}
		else if (tmp == "cgi_extensions")
		{
			checkIfEmpty(server.locations[path].cgiExtensions, &std::vector<std::string>::empty, lineCount);
			while ((s_line >> tmp).eof() == false)
				server.locations[path].cgiExtensions.push_back(tmp);
		}
		else
		{
			std::map<std::string, std::string>::iterator it;

			it = server.locations[path].dirs.find(tmp);
			if (it == server.locations[path].dirs.end())
				closeAndExit("Invalid location variable", lineCount);
			s_line >> it->second;
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

	server.maxBodySize = -1;
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
			closeAndExit("Error during parsing configuration file", lineCount);
		}
	}
	file.close();
}
