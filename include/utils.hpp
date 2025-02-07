#pragma once

#include <assert.h>
#include <fcntl.h>
#include <iostream>
#include <poll.h>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>

void	signals_for_kids(void);
int		openFile(const char* path, std::vector<pollfd>& pdArray);
int		getPollfdIndex(std::vector<pollfd>& polls, int fdToFind);
void	closeAndResetFd(std::vector<pollfd>& polls, int& fd);
void	errorExit(std::string errorMessage, int errorLocation);

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
