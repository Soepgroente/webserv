#include "WebServer.hpp"

std::string	WebServer::showErrorPage(std::string error)
{
	std::ifstream		file("error_pages/" + error + ".html");
    std::stringstream	stream;

	if (file.is_open() == false)
	{
		return ("404 Not Found: The requested resource could not be opened.");
	}
	stream << file.rdbuf();
	file.close();
	return (stream.str());
}
