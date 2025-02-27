#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <algorithm>
#include <numeric>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <random>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <csignal>
#include "TestResults.hpp"
#include "StressTester.hpp"

constexpr int DEFAULT_CONCURRENT_CLIENTS = 2;
constexpr int DEFAULT_REQUESTS_PER_CLIENT = 50;
constexpr int DEFAULT_TIMEOUT_MS = 5000;
constexpr const char* DEFAULT_HOST = "localhost";
constexpr const char* DEFAULT_PORT = "8080";
constexpr int CONNECT_TIMEOUT_MS = 3000;
constexpr int BUFFER_SIZE = 32 * 1024;

extern std::atomic<bool> g_running;

class	StressTester;
struct	TestResults;
struct	RequestTemplate;

void	printUsage(const char* program);
void	signal_handler(int signal);
