#include "utils.hpp"

std::vector<std::string>	stringSplit(std::string toSplit)
{
	std::vector<std::string>	split;
	size_t	end;

	while (toSplit.empty() == false)
	{
		end = toSplit.find("\r\n");
		if (end == std::string::npos)
		{
			split.emplace_back(toSplit);
			break ;
		}
		split.emplace_back(toSplit.substr(0, end));
		toSplit.erase(0, end + 2);
	}
	printVector(split);
	return (split);
}

int	openFile(const char* path, int openFlags, int16_t state, std::vector<pollfd>& pdArray)
{
	int fd = open(path, openFlags);
	if (fd == -1)
	{
		throw std::runtime_error("Failed to open file even though it exists");
	}
	if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) == -1)
	{
		close(fd);
		throw std::runtime_error("Failed to set file fd to non-blocking");
	}
	pdArray.push_back({fd, state, 0});
	return (fd);
}

int	getPollfdIndex(std::vector<pollfd>& polls, int fdToFind)
{
	assert(fdToFind != -1);
	for (size_t i = 0; i < polls.size(); i++)
	{
		if (polls[i].fd == fdToFind)
			return (static_cast<int>(i));
	}
	throw std::runtime_error("Pollfd not found");
}

void	closeAndResetFd(std::vector<pollfd>& polls, int& fd)
{
	int pollFdIndex = getPollfdIndex(polls, fd);

	close(fd);
	fd = -1;
	polls.erase(polls.begin() + pollFdIndex);
}

std::ostream&	operator<<(std::ostream& out, const DateTime& currentTime)
{
	out << std::asctime(currentTime.localTime);
	return (out);
}

void	printToLog(const std::string& message)
{
	std::ofstream	file;
	DateTime		time;

	file.open("log.txt", std::ofstream::out | std::ofstream::app);
	if (file.is_open() == false)
	{
		std::cerr << "Failed to open log file" << std::endl;
		std::exit(EXIT_FAILURE);
	}
	file << time << "---" << message << "---\n" << std::endl;
	file.close();
}