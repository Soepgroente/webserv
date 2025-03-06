#include "Tester.hpp"

/*	Disclaimer: template for tester was written by Claude 3.7	*/

std::atomic<bool> g_running;

bool	checkAndInsertInput(int argc, char** argv, int pos, std::string input, std::string& var)
{
	if (pos + 1 < argc)
	{
		var = argv[pos + 1];
		return (true);
	}
	std::cerr << "Error: " << input.substr(2) << " requires a parameter." << std::endl;
	return (false);
}

int main(int argc, char** argv)
{
	std::string host = DEFAULT_HOST;
	std::string port = DEFAULT_PORT;
	int concurrent_clients = 1;
	int requests_per_client = 1;
	int timeout_ms = DEFAULT_TIMEOUT_MS;
	bool verbose = false;
	bool use_random_paths = false;
	bool use_random_methods = false;

	g_running = true;

	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];

		if ((arg == "-h" || arg == "--host") && checkAndInsertInput(argc, argv, i, "--host", arg) == false)
			return (1);
		else if ((arg == "-p" || arg == "--port") && checkAndInsertInput(argc, argv, i, "--port", arg) == false)
			return (1);
		else if ((arg == "-c" || arg == "--clients") && checkAndInsertInput(argc, argv, i, "--clients", arg) == false)
			return (1);
		else if ((arg == "-n" || arg == "--requests") && checkAndInsertInput(argc, argv, i, "--requests", arg) == false)
			return (1);
		else if ((arg == "-t" || arg == "--timeout") && checkAndInsertInput(argc, argv, i, "--timeout", arg) == false)
			return (1);
		else if (arg == "-v" || arg == "--verbose")
			verbose = true;
		else if (arg == "-r" || arg == "--random-paths")
			use_random_paths = true;
		else if (arg == "-m" || arg == "--random-methods")
			use_random_methods = true;
		else if (arg == "--help")
		{
			printUsage(argv[0]);
			return (0);
		}
		else
		{
			std::cerr << "Unknown option: " << arg << "\n";
			printUsage(argv[0]);
			return (1);
		}
	}
	std::cout << "WebServ Stress Tester v1.0\n";

	StressTester	tester
	(
		host,
		port,
		concurrent_clients,
		requests_per_client,
		timeout_ms,
		verbose,
		use_random_paths,
		use_random_methods
	);

	tester.runTest();
	return (0);
}