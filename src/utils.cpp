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
