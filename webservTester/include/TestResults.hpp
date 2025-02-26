#pragma once

#include "Tester.hpp"

struct TestResults
{
	std::atomic<int> total_requests{0};
	std::atomic<int> successful_requests{0};
	std::atomic<int> failed_requests{0};
	std::atomic<int> connection_errors{0};
	std::atomic<int> timeouts{0};
	std::vector<double> response_times;
	std::mutex response_times_mutex;
	std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
	std::chrono::time_point<std::chrono::high_resolution_clock> end_time;
    
	void	addResponseTime(double time_ms);
	double	averageResponseTime() const;
	double	medianResponseTime() const;
	double	minResponseTime() const;
    double	maxResponseTime() const;    
    double	percentile(double p) const;
    double	requestsPerSecond() const;
};
