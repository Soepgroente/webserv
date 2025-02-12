#include "WebServer.hpp"

const std::string defaultCgiLocation = "./CGI/mandelbrotPython.cgi";

static char** getArgs(std::string cgiLocation)
{
	char** args = new char*[3];

	args[0] = strdup(cgiLocation.c_str());
	// args[1] = (char*)cgiLocation.substr(0).c_str();
	args[1] = strdup(defaultCgiLocation.c_str());
	args[2] = nullptr;
	return (args);
}

static char** getEnvp()
{
	char** envp = new char*[1];
	// envp[0] = strdup("PATH=/home/vvan-der/.capt:/home/vvan-der/.capt/root/usr/local/sbin:/home/vvan-der/.capt/root/usr/local/bin:/home/vvan-der/.capt/root/usr/sbin:/home/vvan-der/.capt/root/usr/bin:/home/vvan-der/.capt/root/sbin:/home/vvan-der/.capt/root/bin:/home/vvan-der/.capt/root/usr/games:/home/vvan-der/.capt/root/usr/local/games:/home/vvan-der/.capt/snap/bin:/home/vvan-der/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin:/home/vvan-der/.dotnet/tools");
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
		// send internal server error to client
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
		std::string	cgiLocation = "/usr/bin/python3";

		if (cgiLocation.empty() == true)
			cgiLocation = defaultCgiLocation;
		close(pipeFd[0]);
		duplicate_fd(pipeFd[1], STDOUT_FILENO);
		char** args = getArgs(cgiLocation);
		std::cout << args[0] << " " << args[1] << std::endl;
		char** envp = getEnvp();
		if (execve(cgiLocation.c_str(), args, envp) == -1)
		{
			if (write(STDOUT_FILENO, "Error: 500", 10) == -1)
				printToLog("Failed to write to cgi pipe");
			printToLog("Failed to execve");
			std::exit(EXIT_FAILURE);
		}
	}
	Client::fileAndCgiDescriptors.push_back({pipeFd[0], POLLIN, 0});
	fileFd = pipeFd[0];
	status = readingFromFile;
}
