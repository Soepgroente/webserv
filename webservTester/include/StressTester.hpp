#pragma once

#include "Tester.hpp"

struct	TestResults;

enum RequestMethod
{
    GET,
    POST,
    DELETE,
};

struct RequestTemplate
{
    RequestMethod   method;
    std::string		path;
    std::string		data;
    std::string		content_type;
};

class StressTester
{
	public:

	StressTester() = delete;
	~StressTester() = default;
	StressTester(const StressTester&) = delete;
	StressTester& operator=(const StressTester&) = delete;

	StressTester(const std::string& host, 
        const std::string& port, 
        int concurrent_clients, 
        int requests_per_client, 
        int timeout_ms,
        bool verbose,
        bool use_random_paths,
        bool use_random_methods);

    void setupRequestTemplates();
    void runTest();
    
	private:

    void	clientWorker(int client_id);
    void	displayProgress();
    void	displayFinalResults();
    std::string methodToString(RequestMethod method) const;
    RequestTemplate getRandomRequestTemplate();

    bool sendRequest(const RequestTemplate& req);
    std::string createHttpRequest(const RequestTemplate& req);

	private:
	
	std::string m_host;
	std::string m_port;
	int m_concurrent_clients;
	int m_requests_per_client;
	int m_timeout_ms;
	bool m_verbose;
	bool m_use_random_paths;
	bool m_use_random_methods;
	std::vector<std::string> m_paths;
	std::vector<RequestTemplate> m_request_templates;
	TestResults m_results;
	std::mutex m_output_mutex;
	std::atomic<int> m_progress{0};
	std::mt19937 m_rng;
};
