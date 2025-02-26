#include "Tester.hpp"

/*	Disclaimer: tester was mostly written by Claude 3.7	*/

int main(int argc, char** argv)
{
    std::string host = DEFAULT_HOST;
    std::string port = DEFAULT_PORT;
    int concurrent_clients = DEFAULT_CONCURRENT_CLIENTS;
    int requests_per_client = DEFAULT_REQUESTS_PER_CLIENT;
    int timeout_ms = DEFAULT_TIMEOUT_MS;
    bool verbose = false;
    bool use_random_paths = false;
    bool use_random_methods = false;
    
    for (int i = 1; i < argc; ++i)
	{
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--host") {
            if (i + 1 < argc) host = argv[++i];
            else {
                std::cerr << "Error: --host requires a parameter.\n";
                return (1);
            }
        } else if (arg == "-p" || arg == "--port") {
            if (i + 1 < argc) port = argv[++i];
            else {
                std::cerr << "Error: --port requires a parameter.\n";
                return (1);
            }
        } else if (arg == "-c" || arg == "--clients") {
            if (i + 1 < argc) concurrent_clients = std::stoi(argv[++i]);
            else {
                std::cerr << "Error: --clients requires a parameter.\n";
                return (1);
            }
        } else if (arg == "-n" || arg == "--requests") {
            if (i + 1 < argc) requests_per_client = std::stoi(argv[++i]);
            else {
                std::cerr << "Error: --requests requires a parameter.\n";
                return (1);
            }
        } else if (arg == "-t" || arg == "--timeout") {
            if (i + 1 < argc) timeout_ms = std::stoi(argv[++i]);
            else {
                std::cerr << "Error: --timeout requires a parameter.\n";
                return (1);
            }
        } else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-r" || arg == "--random-paths") {
            use_random_paths = true;
        } else if (arg == "-m" || arg == "--random-methods") {
            use_random_methods = true;
        } else if (arg == "--help") {
            printUsage(argv[0]);
            return (0);
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            printUsage(argv[0]);
            return (1);
        }
    }
    
    std::cout << "WebServ Stress Tester v1.0\n";

    StressTester tester
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