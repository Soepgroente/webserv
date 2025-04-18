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
	return (split);
}

int	openFile(const char* path, int openFlags, int16_t state, std::vector<pollfd>& pdArray)
{
	int fd = open(path, openFlags, 0644);
	if (fd == -1)
	{
		return (-1);
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

int64_t	getTime()
{
	return (std::chrono::duration_cast<std::chrono::milliseconds>\
		(std::chrono::system_clock::now().time_since_epoch()).count());
}

bool	hasTimedOut(int64_t lastPinged, int64_t timeout)
{
	if (getTime() - lastPinged > timeout)
	{
		printToLog("Connection timed out");
		return (true);
	}
	return (false);
}

void	printToLog(const std::string& message)
{
	std::ofstream	file;
	DateTime		time;

	file.open("log.txt", std::ofstream::out | std::ofstream::app);
	if (file.is_open() == false)
	{
		std::cerr << "Failed to open log file" << std::endl;
		throw std::runtime_error("Failed to open log file");
	}
	file << time << "---" << message << "---\n" << std::endl;
	file.close();
}

std::string	getMimeType(const std::string& input)
{
	const std::map<std::string, std::string>	types = 
	{
		{".jpg", "image/jpeg"},
		{".jpeg", "image/jpeg"},
		{".png", "image/png"},
		{".gif", "image/gif"},
		{".bmp", "image/bmp"},
		{".ico", "image/x-icon"},
		{".css", "text/css"},
		{".html", "text/html"},
		{".js", "text/javascript"},
		{".txt", "text/plain"},
		{".pdf", "application/pdf"},
		{".xml", "application/xml"},
		{".json", "application/json"},
		{".csv", "text/csv"},
		{".svg", "image/svg+xml"},
		{".webp", "image/webp"},
		{".cgi", "cgi"},
		{".py", "cgi"}
	};

	if (types.find(input) == types.end())
	{
		return ("unsupported");
	}
	return (types.at(input));
}
