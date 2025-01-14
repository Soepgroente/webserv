#include "WebServer.hpp"

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
	return (pid);
}

void	WebServer::launchCGI(Client& client, int clientFd)
{
	int pipeFd[2];
	int pid;

	createPipe(pipeFd);
	pid = forkProcess();

	if (pid == 0)
	{
		close(pipeFd[0]);
		duplicate_fd(pipeFd[1], STDOUT_FILENO);
		// execve("cgi.py", args, envp);
		doCgiThings(); // call CGI, not done yet
	}
	pollDescriptors.push_back({pipeFd[0], POLLIN, 0});
	client.setCgiFd(pipeFd[0]);
}

