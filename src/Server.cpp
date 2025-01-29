#include "Server.hpp"

std::ostream&	operator<<(std::ostream& out, const Server& p)
{
	out << "Host: " << p.host << std::endl;
	out << "Port: " << p.port << std::endl;
	out << "Socket: " << p.socket << std::endl;
	out << "Name: " << p.serverName << std::endl;
	out << "Body size: " << p.maxBodySize << std::endl;
	out << "Error location: " << p.errorLocation << "\n" << std::endl;
	for (const std::pair<std::string, Location> pair : p.locations)
	{
		out << "Location: " << pair.first << std::endl;
		out << "Methods: ";
		for (const std::string& iter : pair.second.methods)
		{
			out << iter << ", ";
		}
		out << std::endl;
		out << "CGI extensions: ";
		for (const std::string& iter : pair.second.cgiExtensions)
		{
			out << iter << ", ";
		}
		out << std::endl;
		for (const std::pair<std::string, std::string> despair : pair.second.dirs)
		{
			out << despair.first << ": " << despair.second << std::endl;
		}
		out << "--------------------------------------------" << std::endl;
	}
	out << "==============================================" << std::endl;
	return (out);
}
