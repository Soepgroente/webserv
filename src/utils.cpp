#include "WebServer.hpp"

std::vector<std::string>	stringSplit(std::string toSplit)
{
	std::vector<std::string>	split;
	toSplit = "POST /Server_1_uploads/new_upload HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nUser-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:132.0) Gecko/20100101 Firefox/132.0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nAccept-Language: en-US,en;q=0.5\r\nAccept-Encoding: gzip, deflate, br, zstd\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 11\r\nOrigin: http://127.0.0.1:8080\r\nDNT: 1\r\nConnection: keep-alive\r\nReferer: http://127.0.0.1:8080/postMethod.html\r\nUpgrade-Insecure-Requests: 1\r\nSec-Fetch-Dest: document\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-Site: same-origin\r\nSec-Fetch-User: ?1\r\nPriority: u=0, i\r\n\r\n";
	size_t	end;

	while (toSplit.empty() == false)
	{
		end = toSplit.find("\r\n");
		if (end == std::string::npos)
			throw std::runtime_error("incorrect string format");
		split.emplace_back(toSplit.substr(0, end));
		toSplit.erase(0, end + 2);
	}
	// printVector(split);
	return (split);
}
