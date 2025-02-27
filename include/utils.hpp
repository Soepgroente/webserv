#pragma once

#include <assert.h>
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <poll.h>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>

void	signals_for_kids(void);
int		openFile(const char* path, int openFlags, int16_t state, std::vector<pollfd>& pdArray);
int		getPollfdIndex(std::vector<pollfd>& polls, int fdToFind);
void	closeAndResetFd(std::vector<pollfd>& polls, int& fd);
void	errorExit(std::string errorMessage, int errorLocation);
void	printToLog(const std::string& message);

std::string					getMimeType(const std::string& input);
std::vector<std::string>	stringSplit(std::string toSplit);
std::ostream&				operator<<(std::ostream& out, const std::vector<pollfd>& p);

template <typename T>
void	printVector(std::vector<T>& toPrint)
{
	for (T& it : toPrint)
	{
		std::cout << it << std::endl;
	}
}

class	DateTime
{
	public:

	DateTime() : currentTime(std::time(nullptr)), localTime(std::localtime(&currentTime)){}
	DateTime(const DateTime& original) = delete;
	DateTime&	operator=(const DateTime& original) = delete;
	~DateTime() = default;

	time_t		currentTime;
	std::tm*	localTime;
};

std::ostream&	operator<<(std::ostream& out, const DateTime& currentTime);