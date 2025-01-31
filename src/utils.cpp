#include "WebServer.hpp"

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

int	openFile(const char* path, std::vector<struct pollfd>& pdArray)
{
	int fd = open(path, O_RDONLY);
	if (fd == -1)
	{
		throw std::runtime_error("Failed to open file even though it exists");
	}
	if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) == -1)
	{
		close(fd);
		throw std::runtime_error("Failed to set file fd to non-blocking");
	}
	pdArray.push_back({fd, POLLIN, 0});
	return (fd);
}