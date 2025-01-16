#include "WebServer.hpp"

const std::string defaultCgiLocation = "../CGI/cgi.py";

static char** getArgs(std::string cgiLocation)
{
	char** args = new char*[2];

	args[0] = (char*)cgiLocation.substr(0).c_str();
	args[1] = nullptr;
	return (args);
}

static char** getEnvp()
{
	char** envp = new char*[2];
	envp[0] = (char*)(new std::string("CONTENT_LENGTH=0"));
	envp[1] = nullptr;
	return (envp);
}

static void	duplicate_fd(int oldFd, int newFd)
{
	if (dup2(oldFd, newFd) == -1)
		throw std::runtime_error("Failed to dup2");
	close(oldFd);
}

static void	createPipe(int (&fd)[2])
{
	if (pipe(fd) == -1)
		throw std::runtime_error("Failed to pipe");
}

static int forkProcess()
{
	int pid = fork();

	if (pid == -1)
	{
		// send internal server error to client
		throw std::runtime_error("Failed to fork process");
	}
	if (pid == 0)
		signals_for_kids();
	return (pid);
}

void	WebServer::launchCGI(Client& client)
{
	int pipeFd[2];
	int pid;

	createPipe(pipeFd);
	pid = forkProcess();
	if (pid == 0)
	{
		std::string	cgiLocation = client.getServer().cgiPath;

		std::cout << "HELLO IM A BABY PROCESS" << std::endl;
		if (cgiLocation.empty() == true)
			cgiLocation = defaultCgiLocation;
		close(pipeFd[0]);
		duplicate_fd(pipeFd[1], STDOUT_FILENO);
		char** args = getArgs(cgiLocation);
		char** envp = getEnvp();
		if (execve(cgiLocation.c_str(), args, envp) == -1) // alex hates this with passion
		{
			// internal server error
			std::cerr << "Failed to execve " << cgiLocation << std::endl;
			std::exit(EXIT_FAILURE);
		}
	}
	pollDescriptors.push_back({pipeFd[0], POLLIN, 0});
	client.setCgiFd(pipeFd[0]);
}
