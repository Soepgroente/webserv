#include "WebServer.hpp"
#include "Server.hpp"

std::ifstream	file;

static void	closeAndExit(std::string message, int& lineCount)
{
	if (file.is_open() == true)
		file.close();
	errorExit(message, lineCount);
}

static void	parseHost(const std::string& input, Server& server, int& lineCount)
{
	if (server.host != "")
	{
		closeAndExit("Invalid configuration file", lineCount);
	}
	server.host = input;
}

static void	parsePort(const std::string& input, Server& server, int& lineCount)
{
	if (server.port != 0)
	{
		closeAndExit("Invalid configuration file", lineCount);
	}
	int32_t	largePortTemp = std::stoi(input);
	server.port = std::stoi(input);
	if (largePortTemp < 0 || static_cast<uint16_t>(largePortTemp) != server.port)
	{
		closeAndExit("Invalid configuration file (port out of range)", lineCount);
	}
}

static void	parseName(const std::string& input, Server& server, int& lineCount)
{
	if (server.serverName != "")
	{
		closeAndExit("Invalid configuration file", lineCount);
	}
	server.serverName = input;
}

static void	parseErrorPage(const std::string& input, Server& server, int& lineCount)
{
	if (server.errorLocation != "")
	{
		closeAndExit("Invalid configuration file", lineCount);
	}
	server.errorLocation = input;
}

static void	parseBodySize(const std::string& input, Server& server, int& lineCount)
{
	if (server.maxBodySize != -1)
	{
		closeAndExit("Invalid configuration file", lineCount);
	}
	server.maxBodySize = std::stoi(input);
	if (server.maxBodySize > MAXBODYSIZE)
	{
		closeAndExit("Invalid configuration file (body size too large)", lineCount);
	}
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
	{
		closeAndExit("Invalid configuration file", lineCount);
	}
}

static void	parseLocation(const std::string& input, Server& server, int& lineCount)
{
	std::string path;
	Location	location;

	path = input.substr(0, input.size() - 1);
	removeWhiteSpaces(path);
	server.locations.insert({path, location});

	while (file.eof() == false)
	{
		std::stringstream	s_line;
		std::string			line;
		std::string			tmp;

		std::getline(file, line, '\n');
		removeWhiteSpaces(line);
		lineCount++;
		if (line == "}")
		{
			break ;
		}
		s_line.str(line);
		s_line >> tmp;
		if (tmp == "methods")
		{
			checkIfEmpty(server.locations[path].methods, &std::vector<std::string>::empty, lineCount);
			while (s_line.eof() == false)
			{
				s_line >> tmp;
				server.locations[path].methods.push_back(tmp);
			}
		}
		else if (tmp == "cgi_extensions")
		{
			checkIfEmpty(server.locations[path].cgiExtensions, &std::vector<std::string>::empty, lineCount);
			while (s_line.eof() == false)
			{
				s_line >> tmp;
				server.locations[path].cgiExtensions.push_back(tmp);
			}
		}
		else
		{
			std::map<std::string, std::string>::iterator it;

			it = server.locations[path].dirs.find(tmp);
			if (it == server.locations[path].dirs.end() || it->second != "")
			{
				closeAndExit("Invalid or duplicate location variable", lineCount);
			}
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
	while (file.eof() == false && (line == "" || line == "\n"))
	{
		std::getline(file, line, '\n');
		lineCount++;
	}
	if (file.eof() == true)
	{
		server.host = "Error";
		return (server);
	}
	removeWhiteSpaces(line);
	if (line != "server {")
	{
		closeAndExit("Error in configuration file", lineCount);
	}
	while (file.eof() == false)
	{
		std::getline(file, line, '\n');
		removeWhiteSpaces(line);
		if (line == "}")
		{
			return (server);
		}
		pos = line.find_first_of(' ');
		if (pos == std::string::npos)
		{
			closeAndExit("Error in configuration file", lineCount);
		}
		std::string firstWord = line.substr(0, pos);
		if (parseFunctions.find(firstWord) == parseFunctions.end())
		{
			closeAndExit("Error in configuration file", lineCount);
		}
		parseFunctions[firstWord](&line[pos + 1], server, lineCount);
		lineCount++;
	}
	closeAndExit("Error in configuration file", lineCount);
	return (server); // makes no sense, but can't compile otherwise
}

static void	checkDuplicateIps(const std::vector<Server>& servers)
{
	std::vector<std::string>	ips;

	for (const Server& server : servers)
	{
		std::string	fullIp = server.host + ":" + std::to_string(server.port);

		if (std::find(ips.begin(), ips.end(), fullIp) != ips.end())
		{
			errorExit("Multiple servers with the same IP address in configuration file", -1);
		}
		ips.push_back(fullIp);
	}
}

static void	checkFolderValidity(const std::vector<Server>& servers)
{
	using MapIterator = std::map<std::string, Location>::const_iterator;

	for (const Server& server : servers)
	{
		if (server.errorLocation != "" && std::filesystem::exists("." + server.errorLocation) == false)
		{
			errorExit("Error folder " + server.errorLocation + " does not exist", -1);
		}
		for (MapIterator it = server.locations.begin(); it != server.locations.end(); it++)
		{
			if (std::filesystem::exists("." + it->second.dirs.at("root")) == false)
			{
				errorExit("Root folder " + it->second.dirs.at("root") + " does not exist", -1);
			}
		}
	}
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
			Server	server = parseSingleServer(file, lineCount);

			if (server.host == "Error")
			{
				if (servers.empty() == false)
				{
					continue ;
				}
				closeAndExit("Error in configuration file", lineCount);
			}
			servers.push_back(server);
		}
		catch (std::exception& e)
		{
			std::string error = e.what();
			closeAndExit(error, lineCount);
		}
	}
	file.close();
	checkDuplicateIps(servers);
	checkFolderValidity(servers);
}
