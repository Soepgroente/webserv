#pragma once

#include "Project.hpp"
#include "Server.hpp"

struct	Webserver
{
	public:

	Webserver();
	~Webserver();
	Webserver(int port);
	Webserver(const Webserver& original) = delete;
	void operator=(const Webserver& original) = delete;


	private:

	std::vector<Server>	servers;
};