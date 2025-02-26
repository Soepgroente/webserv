#include "TestResults.hpp"

void	TestResults::addResponseTime(double time_ms)
{
	std::lock_guard<std::mutex> lock(response_times_mutex);
	response_times.push_back(time_ms);
}

double	TestResults::averageResponseTime()
{
	std::lock_guard<std::mutex> lock(response_times_mutex);
	if (response_times.empty())
		return (0.0);
	return (std::accumulate(response_times.begin(), response_times.end(), 0.0) / response_times.size());
}

double	TestResults::medianResponseTime()
{
	std::lock_guard<std::mutex> lock(response_times_mutex);
	if (response_times.empty())
		return (0.0);
	std::vector<double> sorted_times = response_times;
	std::sort(sorted_times.begin(), sorted_times.end());
	if (sorted_times.size() % 2 == 0)
		return ((sorted_times[sorted_times.size() / 2 - 1] + sorted_times[sorted_times.size() / 2]) / 2);
	return (sorted_times[sorted_times.size() / 2]);
}

double	TestResults::minResponseTime()
{
	std::lock_guard<std::mutex> lock(response_times_mutex);
	if (response_times.empty())
		return (0.0);
	return (*std::min_element(response_times.begin(), response_times.end()));
}

double	TestResults::maxResponseTime()
{
	std::lock_guard<std::mutex> lock(response_times_mutex);
	if (response_times.empty())
		return (0.0);
	return (*std::max_element(response_times.begin(), response_times.end()));
}

double	TestResults::percentile(double p)
{
	std::lock_guard<std::mutex> lock(response_times_mutex);
	if (response_times.empty())
		return (0.0);
	std::vector<double> sorted_times = response_times;
	std::sort(sorted_times.begin(), sorted_times.end());
	int idx = static_cast<int>(p * sorted_times.size() / 100);
	return (sorted_times[idx]);
}

double	TestResults::requestsPerSecond() const
{
	auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
	return (duration_ms > 0 ? (total_requests.load() * 1000.0 / duration_ms) : 0.0);
}