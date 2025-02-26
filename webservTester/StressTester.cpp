#include "StressTester.hpp"

StressTester::StressTester(const std::string& host, const std::string& port, \
	int concurrent_clients, int requests_per_client, int timeout_ms, bool verbose, \
	bool use_random_paths, bool use_random_methods)

	:

	m_host(host), m_port(port), m_concurrent_clients(concurrent_clients), \
	m_requests_per_client(requests_per_client),	m_timeout_ms(timeout_ms), \
	m_verbose(verbose),	m_use_random_paths(use_random_paths), \
	m_use_random_methods(use_random_methods), m_rng(std::random_device{}())
{
	setupRequestTemplates();
}

void	StressTester::setupRequestTemplates()
{
	m_paths = {
		"/",
		"/index.html",
		"/about",
		"/images/logo.png",
		"/api/users",
		"/api/data",
		"/blog/2025/02/26",
		"/search?q=webserv",
		"/non-existent-page",
		"/very/deep/nested/path/structure"
	};

	// Create some request templates
	m_request_templates.push_back({RequestMethod::GET, "/", "", ""});
	m_request_templates.push_back({RequestMethod::GET, "/index.html", "", ""});
	m_request_templates.push_back({RequestMethod::POST, "/api/submit", "name=test&data=example", "application/x-www-form-urlencoded"});
	m_request_templates.push_back({RequestMethod::DELETE, "/api/remove", "id=123", "application/x-www-form-urlencoded"});
}

void	StressTester::runTest()
{
	std::signal(SIGINT, signal_handler);
	
	std::cout << "\n🚀 Starting stress test for " << m_host << ":" << m_port << "\n";
	std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
	std::cout << "Concurrent clients: " << m_concurrent_clients << "\n";
	std::cout << "Requests per client: " << m_requests_per_client << "\n";
	std::cout << "Total requests to send: " << m_concurrent_clients * m_requests_per_client << "\n";
	std::cout << "Timeout: " << m_timeout_ms << "ms\n";
	std::cout << "Random paths: " << (m_use_random_paths ? "Yes" : "No") << "\n";
	std::cout << "Random methods: " << (m_use_random_methods ? "Yes" : "No") << "\n" << std::endl;

	m_results.start_time = std::chrono::high_resolution_clock::now();

	std::vector<std::thread> threads;
	for (int i = 0; i < m_concurrent_clients; ++i)
	{
		threads.emplace_back(&StressTester::clientWorker, this, i);
	}

	std::thread progress_thread(&StressTester::displayProgress, this);

	for (auto& thread : threads)
	{
		thread.join();
	}
	g_running = false;
	progress_thread.join();
	m_results.end_time = std::chrono::high_resolution_clock::now();
	displayFinalResults();
}

void	StressTester::clientWorker(int client_id)
{
	for (int i = 0; i < m_requests_per_client && g_running; ++i)
	{
		RequestTemplate req = getRandomRequestTemplate();

		auto start_time = std::chrono::high_resolution_clock::now();
		bool success = sendRequest(req);
		auto end_time = std::chrono::high_resolution_clock::now();
		double response_time = std::chrono::duration_cast<std::chrono::duration
			<double, std::milli>>(end_time - start_time).count();

		m_results.total_requests++;
		if (success == true)
		{
			m_results.successful_requests++;
			m_results.addResponseTime(response_time);
			
			if (m_verbose == true)
			{
				std::lock_guard<std::mutex> lock(m_output_mutex);
				std::cout << "Client " << client_id << ": " 
							<< methodToString(req.method) << " " << req.path 
							<< " - Success (" << response_time << "ms)\n";
			}
		}
		else
		{
			m_results.failed_requests++;
			
			if (m_verbose == true)
			{
				std::lock_guard<std::mutex> lock(m_output_mutex);
				std::cout << "Client " << client_id << ": " 
							<< methodToString(req.method) << " " << req.path 
							<< " - Failed (" << response_time << "ms)\n";
			}
		}
		m_progress++;
		if (i < m_requests_per_client - 1)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(20 + (std::rand() % 80)));
		}
	}
}

void	StressTester::displayProgress()
{
	int total_requests = m_concurrent_clients * m_requests_per_client;
	int prev_progress = 0;
	
	while (g_running && m_progress < total_requests)
	{
		if (m_progress > prev_progress)
		{
			int progress_percent = (m_progress * 100) / total_requests;
			int bar_width = 50;
			int filled_width = bar_width * m_progress / total_requests;
			
			std::lock_guard<std::mutex> lock(m_output_mutex);
			std::cout << "\r[";
			for (int i = 0; i < bar_width; ++i)
			{
				if (i < filled_width) std::cout << "=";
				else if (i == filled_width) std::cout << ">";
				else std::cout << " ";
			}
			
			std::cout << "] " << progress_percent << "% (" 
						<< m_progress << "/" << total_requests << " requests, "
						<< m_results.successful_requests << " successful, "
						<< m_results.failed_requests << " failed)" << std::flush;
			
			prev_progress = m_progress;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	if (m_progress >= total_requests)
	{
		std::lock_guard<std::mutex> lock(m_output_mutex);
		std::cout << "\r[";
		for (int i = 0; i < 50; ++i) std::cout << "=";
		std::cout << "] 100% (" << total_requests << "/" << total_requests 
					<< " requests, " << m_results.successful_requests 
					<< " successful, " << m_results.failed_requests 
					<< " failed)" << std::endl;
	}
}

void	StressTester::displayFinalResults()
{
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
		m_results.end_time - m_results.start_time).count() / 1000.0;
	
	auto requests_per_second = m_results.requestsPerSecond();
	auto avg_response_time = m_results.averageResponseTime();
	auto median_response_time = m_results.medianResponseTime();
	auto min_response_time = m_results.minResponseTime();
	auto max_response_time = m_results.maxResponseTime();
	auto p90_response_time = m_results.percentile(90);
	auto p95_response_time = m_results.percentile(95);
	auto p99_response_time = m_results.percentile(99);
	
	auto now = std::chrono::system_clock::now();
	auto now_time = std::chrono::system_clock::to_time_t(now);
	std::stringstream filename;
	filename << "webserv_stress_report_" << std::put_time(std::localtime(&now_time), "%Y%m%d_%H%M%S") << ".log";

	std::cout << "\n\n📊 Test Results Summary:\n";
	std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
	std::cout << "Test Duration: " << std::fixed << std::setprecision(2) << duration << " seconds\n";
	std::cout << "Total Requests: " << m_results.total_requests << "\n";
			std::cout << "Successful Requests: " << m_results.successful_requests << " (" 
				<< std::fixed << std::setprecision(2) 
				<< (m_results.successful_requests * 100.0 / m_results.total_requests) << "%)\n";
	std::cout << "Failed Requests: " << m_results.failed_requests << " (" 
				<< std::fixed << std::setprecision(2)
				<< (m_results.failed_requests * 100.0 / m_results.total_requests) << "%)\n";
	std::cout << "Connection Errors: " << m_results.connection_errors << "\n";
	std::cout << "Timeouts: " << m_results.timeouts << "\n";
	std::cout << "Requests per Second: " << std::fixed << std::setprecision(2) << requests_per_second << "\n";
	std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
	std::cout << "Response Time Statistics (ms):\n";
	std::cout << "  Average: " << std::fixed << std::setprecision(2) << avg_response_time << "\n";
	std::cout << "  Median: " << std::fixed << std::setprecision(2) << median_response_time << "\n";
	std::cout << "  Min: " << std::fixed << std::setprecision(2) << min_response_time << "\n";
	std::cout << "  Max: " << std::fixed << std::setprecision(2) << max_response_time << "\n";
	std::cout << "  90th percentile: " << std::fixed << std::setprecision(2) << p90_response_time << "\n";
	std::cout << "  95th percentile: " << std::fixed << std::setprecision(2) << p95_response_time << "\n";
	std::cout << "  99th percentile: " << std::fixed << std::setprecision(2) << p99_response_time << "\n";
	std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";

	std::ofstream report_file(filename.str());
	if (report_file.is_open() == false)
	{
		std::cerr << "Failed to save report to file.\n";
		return ;
	}
	report_file << "WebServ Stress Test Report\n";
	report_file << "========================\n\n";
	report_file << "Test Configuration:\n";
	report_file << "  Host: " << m_host << ":" << m_port << "\n";
	report_file << "  Concurrent Clients: " << m_concurrent_clients << "\n";
	report_file << "  Requests per Client: " << m_requests_per_client << "\n";
	report_file << "  Timeout: " << m_timeout_ms << "ms\n";
	report_file << "  Random Paths: " << (m_use_random_paths ? "Yes" : "No") << "\n";
	report_file << "  Random Methods: " << (m_use_random_methods ? "Yes" : "No") << "\n\n";
	
	report_file << "Test Results:\n";
	report_file << "  Test Duration: " << std::fixed << std::setprecision(2) << duration << " seconds\n";
	report_file << "  Total Requests: " << m_results.total_requests << "\n";
	report_file << "  Successful Requests: " << m_results.successful_requests << " (" 
				<< std::fixed << std::setprecision(2) 
				<< (m_results.successful_requests * 100.0 / m_results.total_requests) << "%)\n";
	report_file << "  Failed Requests: " << m_results.failed_requests << " (" 
				<< std::fixed << std::setprecision(2)
				<< (m_results.failed_requests * 100.0 / m_results.total_requests) << "%)\n";
	report_file << "  Connection Errors: " << m_results.connection_errors << "\n";
	report_file << "  Timeouts: " << m_results.timeouts << "\n";
	report_file << "  Requests per Second: " << std::fixed << std::setprecision(2) << requests_per_second << "\n\n";
	
	report_file << "Response Time Statistics (ms):\n";
	report_file << "  Average: " << std::fixed << std::setprecision(2) << avg_response_time << "\n";
	report_file << "  Median: " << std::fixed << std::setprecision(2) << median_response_time << "\n";
	report_file << "  Min: " << std::fixed << std::setprecision(2) << min_response_time << "\n";
	report_file << "  Max: " << std::fixed << std::setprecision(2) << max_response_time << "\n";
	report_file << "  90th percentile: " << std::fixed << std::setprecision(2) << p90_response_time << "\n";
	report_file << "  95th percentile: " << std::fixed << std::setprecision(2) << p95_response_time << "\n";
	report_file << "  99th percentile: " << std::fixed << std::setprecision(2) << p99_response_time << "\n";
	
	std::cout << "Report saved to " << filename.str() << "\n";
	report_file.close();
}

std::string	StressTester::methodToString(RequestMethod method) const
{
	const std::array<std::string, 3> method_names = {"GET", "POST", "DELETE"};

	if (method >= DELETE)
		return ("UNKNOWN");
	return (method_names[static_cast<int>(method)]);
}

RequestTemplate	StressTester::getRandomRequestTemplate()
{
	if (m_use_random_methods == false && m_use_random_paths == false)
	{
		static size_t template_index = 0;
		RequestTemplate tmpl = m_request_templates[template_index % m_request_templates.size()];
		template_index++;
		return (tmpl);
	}
	
	RequestTemplate tmpl;

	if (m_use_random_methods == true)
	{
		std::uniform_int_distribution<int> method_dist(0, 4);
		tmpl.method = static_cast<RequestMethod>(method_dist(m_rng));
	}
	else
	{
		std::uniform_int_distribution<int> method_dist(0, 8);
		int method_selector = method_dist(m_rng);
		if (method_selector < 7)
		{
			tmpl.method = RequestMethod::GET;
		}
		else if (method_selector == 7)
		{
			tmpl.method = RequestMethod::POST;
		}
		else if (method_selector == 8)
		{
			tmpl.method = RequestMethod::DELETE;
		}
	}
	if (m_use_random_paths == true)
	{
		std::uniform_int_distribution<int> segments_dist(1, 5);
		int segments = segments_dist(m_rng);
		
		std::string path = "/";
		std::vector<std::string> segment_options =
		{
			"api", "users", "data", "images", "posts", "comments",
			"admin", "login", "profile", "settings", "public", "assets",
			"css", "js", "docs", "help", "search", "categories"
		};
		
		for (int i = 0; i < segments; ++i)
		{
			std::uniform_int_distribution<int> segment_dist(0, segment_options.size() - 1);
			path += segment_options[segment_dist(m_rng)];
			
			// Sometimes add an ID-like number
			std::uniform_int_distribution<int> add_id_dist(0, 3);
			if (add_id_dist(m_rng) == 0)
			{
				std::uniform_int_distribution<int> id_dist(1, 9999);
				path += "/" + std::to_string(id_dist(m_rng));
			}
			
			if (i < segments - 1)
			{
				path += "/";
			}
		}
		std::uniform_int_distribution<int> add_query_dist(0, 4);
		if (add_query_dist(m_rng) == 0)
		{
			path += "?";
			std::uniform_int_distribution<int> params_dist(1, 3);
			int params = params_dist(m_rng);
			
			std::vector<std::string> param_names = { "id", "page", "limit", "sort", "filter", "q", "type", "format"};
			for (int i = 0; i < params; ++i)
			{
				std::uniform_int_distribution<int> param_dist(0, param_names.size() - 1);
				std::uniform_int_distribution<int> value_dist(1, 100);
				
				path += param_names[param_dist(m_rng)] + "=" + std::to_string(value_dist(m_rng));
				
				if (i < params - 1)
				{
					path += "&";
				}
			}
		}
		
		tmpl.path = path;
	}
	else
	{
		std::uniform_int_distribution<int> path_dist(0, m_paths.size() - 1);
		tmpl.path = m_paths[path_dist(m_rng)];
	}
	if (tmpl.method == RequestMethod::POST)
	{
		std::uniform_int_distribution<int> data_type_dist(0, 2);
		int data_type = data_type_dist(m_rng);
		
		if (data_type == 0)
		{
			tmpl.data = "name=test&value=example&timestamp=" + std::to_string(time(nullptr));
			tmpl.content_type = "application/x-www-form-urlencoded";
		}
		else if (data_type == 1)
		{
			tmpl.data = "{\"name\":\"test\",\"value\":\"example\",\"timestamp\":" + std::to_string(time(nullptr)) + "}";
			tmpl.content_type = "application/json";
		}
		else
		{
			tmpl.data = "This is a test message with timestamp " + std::to_string(time(nullptr));
			tmpl.content_type = "text/plain";
		}
	}
	return (tmpl);
}

bool	StressTester::sendRequest(const RequestTemplate& req)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((rv = getaddrinfo(m_host.c_str(), m_port.c_str(), &hints, &servinfo)) != 0)
	{
		m_results.connection_errors++;
		return (false);
	}
	for (p = servinfo; p != nullptr; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			continue ;
		}
		int flags = fcntl(sockfd, F_GETFL, 0);
		fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
		int connect_result = connect(sockfd, p->ai_addr, p->ai_addrlen);
		if (connect_result == -1)
		{
			if (errno != EINPROGRESS)
			{
				close(sockfd);
				continue ;
			}
			fd_set write_fds;
			FD_ZERO(&write_fds);
			FD_SET(sockfd, &write_fds);
			
			struct timeval timeout;
			timeout.tv_sec = CONNECT_TIMEOUT_MS / 1000;
			timeout.tv_usec = (CONNECT_TIMEOUT_MS % 1000) * 1000;
			
			int select_result = select(sockfd + 1, NULL, &write_fds, NULL, &timeout);
			
			if (select_result <= 0)
			{
				close(sockfd);
				m_results.connection_errors++;
				continue ;
			}
			int error = 0;
			socklen_t len = sizeof(error);
			if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error)
			{
				close(sockfd);
				continue ;
			}
		}
		fcntl(sockfd, F_SETFL, flags);
		break ;
	}
	if (p == nullptr)
	{
		freeaddrinfo(servinfo);
		m_results.connection_errors++;
		return (false);
	}
	freeaddrinfo(servinfo);

	std::string http_request = createHttpRequest(req);

	if (send(sockfd, http_request.c_str(), http_request.length(), 0) == -1)
	{
		close(sockfd);
		return (false);
	}

	char buffer[BUFFER_SIZE];
	struct timeval timeout;
	timeout.tv_sec = m_timeout_ms / 1000;
	timeout.tv_usec = (m_timeout_ms % 1000) * 1000;
	
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
	{
		close(sockfd);
		return (false);
	}
	
	int received = 0;
	bool success = false;
	int status_code = -1;

	while ((received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0)) > 0)
	{
		buffer[received] = '\0';
		if (status_code == -1)
		{
			std::string response(buffer, received);
			size_t status_pos = response.find(' ');
			if (status_pos != std::string::npos)
			{
				status_pos++;
				status_code = std::stoi(response.substr(status_pos));
			}
		}
		if (status_code >= 200 && status_code < 400)
		{
			success = true;
		}
	}
	
	if (received == 0)
	{
		if (status_code == -1)
		{
			success = false;
		}
	}
	else if (received == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			m_results.timeouts++;
		}
		success = false;
	}
	close(sockfd);
	return (success);
}

std::string	StressTester::createHttpRequest(const RequestTemplate& req)
{
	std::string request = methodToString(req.method) + " " + req.path + " HTTP/1.1\r\n";

	request += "Host: " + m_host + ":" + m_port + "\r\n";
	request += "User-Agent: WebservStressTester/1.0\r\n";
	request += "Accept: */*\r\n";
	request += "Connection: close\r\n";
	if (req.method == RequestMethod::POST)
	{
		request += "Content-Type: " + req.content_type + "\r\n";
		request += "Content-Length: " + std::to_string(req.data.length()) + "\r\n";
	}
	std::uniform_int_distribution<int> add_headers_dist(0, 2);
	int add_headers = add_headers_dist(m_rng);
	
	if (add_headers > 0)
	{
		request += "Accept-Language: en-US,en;q=0.9\r\n";
	}
	if (add_headers > 1)
	{
		request += "Accept-Encoding: gzip, deflate\r\n";
	}
	request += "\r\n";
	if (req.method == RequestMethod::POST)
	{
		request += req.data;
	}
	return (request);
};