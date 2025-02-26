#pragma once

#include <iostream>
#include <vector>
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

constexpr int DEFAULT_CONCURRENT_CLIENTS = 10;
constexpr int DEFAULT_REQUESTS_PER_CLIENT = 100;
constexpr int DEFAULT_TIMEOUT_MS = 5000;
constexpr const char* DEFAULT_HOST = "localhost";
constexpr const char* DEFAULT_PORT = "8080";
constexpr int CONNECT_TIMEOUT_MS = 3000;
constexpr int BUFFER_SIZE = 4096;

std::atomic<bool> g_running(true);

class StressTester;
class TestResults;

enum RequestMethod
{
    GET,
    POST,
    DELETE,
};

struct RequestTemplate
{
    RequestMethod method;
    std::string path;
    std::string data;
    std::string content_type;
};

void	printUsage(const char* program);
void	signal_handler(int signal);
