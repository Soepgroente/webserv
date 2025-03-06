#include "WebServer.hpp"

static char** getArgs(const std::string& location)
{
	char** args = new char*[2];

	args[0] = strdup(location.c_str());
	args[1] = nullptr;
	return (args);
}

static char** getEnvp()
{
	char** envp = new char*;
	envp[0] = nullptr;
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

static pid_t forkProcess()
{
	pid_t	pid = fork();

	if (pid == -1)
	{
		throw std::runtime_error("Failed to fork process");
	}
	if (pid == 0)
		signals_for_kids();
	return (pid);
}

void	Client::launchCGI()
{
	int		pipeFd[2];

	if (cgiCounter >= MAX_CONCURRENT_CGIS)
	{
		setupErrorPage(serviceOverloaded);
		return ;
	}
	createPipe(pipeFd);
	cgiPid = forkProcess();
	if (cgiPid == 0)
	{
		close(pipeFd[0]);
		duplicate_fd(pipeFd[1], STDOUT_FILENO);
		if (execve(request.dotPath.c_str(), getArgs(request.dotPath), getEnvp()) == -1)
		{
			if (write(STDOUT_FILENO, "Error: 500", 10) == -1)
				printToLog("Failed to write to cgi pipe");
			printToLog("Failed to execve");
			std::exit(EXIT_FAILURE);
		}
	}
	close(pipeFd[1]);
	Client::fileAndCgiDescriptors.push_back({pipeFd[0], POLLIN, 0});
	fileFd = pipeFd[0];
	status = parseCgi;
	cgiCounter++;
	cgiTimeout = getTime();
	std::cout << cgiCounter << std::endl;
}
