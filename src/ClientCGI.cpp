#include "WebServer.hpp"

static char** getArgs(const std::string& location)
{
	char** args = new char*[2];

	args[0] = strdup(location.c_str());
	args[1] = nullptr;

	std::cerr << args[0] << " " << args[1] << std::endl;
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

static int forkProcess()
{
	int pid = fork();

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
	int pipeFd[2];
	int pid;

	createPipe(pipeFd);
	pid = forkProcess();
	if (pid == 0)
	{
		request.path = "." + request.path;
		close(pipeFd[0]);
		duplicate_fd(pipeFd[1], STDOUT_FILENO);
		char** args = getArgs(request.path);
		std::cout << args[0] << " " << args[1] << std::endl;
		char** envp = getEnvp();
		if (execve(request.path.c_str(), args, envp) == -1)
		{
			if (write(STDOUT_FILENO, "Error: 500", 10) == -1)
				printToLog("Failed to write to cgi pipe");
			printToLog("Failed to execve");
			std::exit(EXIT_FAILURE);
		}
	}
	Client::fileAndCgiDescriptors.push_back({pipeFd[0], POLLIN, 0});
	fileFd = pipeFd[0];
	status = parseCgi;
}
