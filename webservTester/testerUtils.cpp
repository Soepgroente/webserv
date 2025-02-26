#include "Tester.hpp"

void signal_handler(int signal)
{
    if (signal == SIGINT)
	{
        std::cout << "\nReceived interrupt signal. Stopping test gracefully...\n";
        g_running = false;
    }
}

void printUsage(const char* program)
{
    std::cout << "Usage: " << program << " [OPTIONS]\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --host HOST       Target host (default: " << DEFAULT_HOST << ")\n";
    std::cout << "  -p, --port PORT       Target port (default: " << DEFAULT_PORT << ")\n";
    std::cout << "  -c, --clients N       Number of concurrent clients (default: " << DEFAULT_CONCURRENT_CLIENTS << ")\n";
    std::cout << "  -n, --requests N      Number of requests per client (default: " << DEFAULT_REQUESTS_PER_CLIENT << ")\n";
    std::cout << "  -t, --timeout MS      Request timeout in milliseconds (default: " << DEFAULT_TIMEOUT_MS << ")\n";
    std::cout << "  -v, --verbose         Enable verbose output\n";
    std::cout << "  -r, --random-paths    Use randomly generated paths\n";
    std::cout << "  -m, --random-methods  Use random HTTP methods\n";
    std::cout << "  --help                Display this help and exit\n";
    std::cout << "\nExample:\n";
    std::cout << "  " << program << " -h localhost -p 8080 -c 50 -n 200 -v\n";
}
