/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   WebServerHandleRequests.cpp                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: akasiota <akasiota@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/30 17:46:31 by akasiota      #+#    #+#                 */
/*   Updated: 2024/11/12 14:54:08 by akasiota      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

static int	findServerFromPort(std::istringstream& request_s, const std::vector<serverConf_s>& servers)
{
	std::string	tmp;
	request_s >> tmp;
	if (tmp != "Host:")
		return -1;
	request_s >> tmp;
	std::size_t	pos;
	pos = tmp.find(':');
	if (tmp.find(':') == std::string::npos)
		return -1;
	std::string	host_str = tmp.substr(0, pos);
	if (host_str == "localhost")
		host_str = "127.0.0.1";
	std::string	port_str = tmp.substr(pos + 1);
	int	i = 0;
	for (auto& server : servers)
	{
		if (server.host == host_str && server.port == std::stoi(port_str))
			return i;
		i++;
	}
	return -1;
}

static std::string handleDirectoryRequest(const std::string& dir_path, const routeConf_s& route, const serverConf_s& server)
{
	if (!route.directory_listing && !route.index.empty())
	{
		std::string index_path = dir_path + "/" + route.index;
		std::ifstream index_file(index_path, std::ios::binary);
		if (!index_file.is_open())
			return "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 25\r\n\r\n500\nInternal Server Error";
		std::stringstream buffer;
		buffer << index_file.rdbuf();
		std::string content = buffer.str();
		std::ostringstream response;
		response << "HTTP/1.1 200 OK\r\n";
		response << "Content-Length: " << content.size() << "\r\n";
		response << "Content-Type: text/html\r\n\r\n";
		response << content;
		return response.str();
	}
	if (route.directory_listing)
	{
		DIR *dir = opendir(dir_path.c_str());
		if (!dir)
		{
			if (!route.redirection.empty())
			{
				return "HTTP/1.1 200 OK\r\nContent-Length: 20\r\n\r\nRedirection Homepage";
			}
			return constructErrorResponse("403", server);
		}
		std::stringstream content;
		content << "<html><body><ul>";
		struct dirent* entry;
		entry = readdir(dir);
		while (entry)
		{
			std::string name = entry->d_name;
			if (name == ".")
				continue;
			content << "<li><a href=\"" << name << "\">" << name << "</a></li>";
		}
		content << "</ul></body></html>";
		closedir(dir);
		std::string body = content.str();
		std::ostringstream response;
		response << "HTTP/1.1 200 OK\r\n";
		response << "Content-Length: " << body.size() << "\r\n";
		response << "Content-Type: text/html\r\n\r\n";
		response << body;
		
		return response.str();
	}
	if (!route.redirection.empty())
	{
		std::ostringstream response;
		response << "HTTP/1.1 301 Moved Permanently\r\n";
		response << "Location: " << route.redirection << "\r\n";
		response << "Content-Length: 0\r\n\r\n";
		
		// response << "Content-Length: " << route.redirection.size() + 23 << "\r\n";
		// response << "Content-Type: text/html\r\n\r\n";
		// response << "301\nMoved Permanently: " << route.redirection;
		return response.str();  
	}
	return constructErrorResponse("403", server);
}

static std::string executeCGI(const std::string& script_path, const std::string& path_info, const serverConf_s& server)
{
	int pipe_fd[2];
	if (pipe(pipe_fd) == -1)
		constructErrorResponse("500", server);
	pid_t pid = fork();
	if (pid == -1)
		constructErrorResponse("500", server);
	else if (pid == 0)
	{
		close(pipe_fd[0]);
		dup2(pipe_fd[1], STDOUT_FILENO);
		// dup2(pipe_fd[1], STDERR_FILENO);
		close(pipe_fd[1]);
		std::vector<const char*> args;
		args.push_back(script_path.c_str());
		args.push_back(path_info.c_str());
		args.push_back(nullptr);
		execve(script_path.c_str(), const_cast<char* const*>(args.data()), nullptr);
		exit(EXIT_FAILURE);
	}
	close(pipe_fd[1]);
	std::string cgi_output;
	char buffer[BUFFER_SIZE];
	int bytes;
	while ((bytes = read(pipe_fd[0], buffer, BUFFER_SIZE)) > 0)
	{
		cgi_output.append(buffer, bytes);
	}
	close(pipe_fd[1]);
	if (waitpid(pid, nullptr, 0) == -1)
		return constructErrorResponse("500", server);
	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n";
	response << "Content-Length: " << cgi_output.size() << "\r\n";
	response << "Content-Type: text/html\r\n\r\n";
	response << cgi_output;
	return response.str();
}

static std::string	getMimeType(const std::string& file_path)
{
	size_t	dot = file_path.find_last_of('.');
	if (dot == std::string::npos)
		return "text/plain";
	std::string	ext = file_path.substr(dot + 1);
	if (ext == "html" || ext == "htm")
		return "text/html";
	if (ext == "css")
		return "text/css";
	if (ext == "js")
		return "application/javascript";
	if (ext == "json")
		return "application/json";
	if (ext == "png")
		return "image/png";
	if (ext == "jpeg" || ext == "jpg")
		return "image/jpeg";
	if (ext == "gif")
		return "image/gif";
	if (ext == "svg")
		return "image/svg+xml";
	if (ext == "txt")
		return "text/plain";
	if (ext == "pdf")
		return "application/pdf";
	if (ext == "cpp")
		return "text/cpp";
	return "application/octet-stream";
}

static std::string	handleGetRequest(const std::string& path, const serverConf_s server)
{
	std::string	file_path;
	
	if (path == "/")
	{
		file_path = "Homepage.html";
		// return "HTTP/1.1 200 OK\r\nContent-Length: 8\r\n\r\nHomepage";
	}
	else
		file_path = path.substr(1);
	std::ifstream	file(file_path, std::ios::binary);
	std::cout << "FILE PATH: " << file_path << "\n\n" << std::endl;
	if (!file.is_open())
	{
		std::cout << "\nFILE PATH IN ERROR: " << file_path << std::endl;
		return constructErrorResponse("404", server);
	}	
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string	content;
	content = buffer.str();
	std::stringstream response;
	response << "HTTP/1.1 200 OK\r\n";
	response << "Content-Length: " << content.size() << "\r\n";
	response << "Content-Type: " << getMimeType(file_path) << "\r\n\r\n";
	response << content;
	file.close();
	return response.str();
}

static std::string	trimWhitespace(std::string& line)
{
	size_t	start = line.find_first_not_of(" \t\r\n");
	size_t	end = line.find_last_not_of(" \t\r\n");
	if (start == std::string::npos)
		return "";
	return (line.substr(start, end - start + 1));
}

static void	parseHeaders(std::istringstream& request, std::map<std::string, std::string>& headers)
{
	std::string	line;
	std::string	trimmed_line;
	size_t		pos;
	
	request.seekg(0);
	while (std::getline(request, line))
	{
		trimmed_line = trimWhitespace(line);
		if (trimmed_line == "")
			break;
		pos = trimmed_line.find(": ");
		if (pos != std::string::npos)
		{
			std::string	key = trimmed_line.substr(0, pos);
			std::string	value = trimmed_line.substr(pos + 2);
			if (key.empty() || value.empty())
			{
				handleError("Invalid header");
				return ;
			}
			headers[key] = value;
		}
	}	
}

static std::string	handlePostRequest(const std::string& path, std::istringstream& request, const routeConf_s* route, const serverConf_s& server)
{
	std::map<std::string, std::string> headers;
	std::string	str_request = request.str();
	std::string file_content;
		
	parseHeaders(request, headers);
	if (std::stoi(headers["Content-Length"]) > server.client_body_size)
		return constructErrorResponse("413", server);
	if (headers.find("Content-Type") != headers.end())
	{
		if (headers["Content-Type"].find("application/x-www-form-urlencoded") != std::string::npos)
		{
			// adjust
			file_content = str_request.substr(str_request.find("\r\n\r\n") + 4);
			// std::cout << "\n\n----REQUEST_STREAM-------\n" << body << std::endl;
			
			// return "HTTP/1.1 200 OK\r\nContent-Length: 11\r\n\r\nURL-ENCODED";
		}
		else if (headers["Content-Type"].find("multipart/form-data") != std::string::npos)
		{
			std::string boundary = "--" + headers["Content-Type"].substr(headers["Content-Type"].find("boundary=") + 9);
			std::string	body = str_request.substr(str_request.find("/r/n/r/n") + 4);
			size_t		start = body.find(boundary) + boundary.size() + 2;
			size_t		end = body.find(boundary, start) - 4;
			file_content = body.substr(start, end - start);
		}
		else
		{	
			// Error - adjust
			return constructErrorResponse("500", server);
		}
	}
	else
	{
		// Error - adjust
		return constructErrorResponse("400", server);
	}
	std::string		upload_dir = route->upload_dir;
	try
	{
		if (!std::filesystem::exists(upload_dir))
		{
			if (!std::filesystem::create_directories(upload_dir))
				return constructErrorResponse("500", server);
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return constructErrorResponse("500", server);
	}
	size_t pos = path.find_last_of('/');
	std::string filename = upload_dir + path.substr(pos);
	// std::string		upload_dir = "uploads";//server.routes; // this needs to be the upload directory of the server
	// struct stat st;
	// if (stat(upload_dir.c_str(), &st) == -1)
	// 	mkdir(upload_dir.c_str(), 0755);
	// std::string		filename = upload_dir + path;
	std::cout << "\n\n-----FILENAME: " << filename << std::endl;
	std::ofstream	file(filename, std::ios::binary);
	if (!file.is_open())
	{
		return constructErrorResponse("500", server);
	}
	file.write(file_content.c_str(), file_content.size());
	file.close();
	

	return "HTTP/1.1 200 OK\r\nContent-Length:11\r\n\r\nUpload done";
}

std::string	WebServer::handleRequest(const std::string& request, const std::vector<serverConf_s>& servers)
{
	std::istringstream	request_s(request);
	std::string	method, path, version;
	
	request_s >> method >> path >> version;
	// Find the correct server configuration to use
	int i = findServerFromPort(request_s, servers);
	if (i == -1)
		return "HTTP/1.1 400 Bad Request\r\nContent-Length: 15\r\n\r\n400\nBad Request";
	// Find the most specific matching route (longest prefix)
	const routeConf_s*	matched_route = nullptr;
	std::string		matched_path;
	for (auto& route : servers[i].routes)
	{
		std::string	route_path = route.first;
		if (path.compare(0, route_path.length(), route_path) == 0 && \
		route_path.length() > matched_path.length())
		{
			matched_route = &route.second;
			matched_path = route_path;
		}
		// std::cout << "Matched path: " << matched_path << std::endl;
		// std::cout << "Route path: " << route_path << std::endl;
	}
	if (matched_route != nullptr)
	{
		if (matched_route->methods.empty() == false && \
		std::find(matched_route->methods.begin(), matched_route->methods.end(), method) == matched_route->methods.end())
			return constructErrorResponse("405", servers[i]);
		if (!matched_route->redirection.empty())
		{
			std::ostringstream	response;
			response << "HTTP/1.1 301 Moved Permanently\r\n";
			response << "Location: " << matched_route->redirection << "\r\n";
			response << "Content-Length: 0\r\n\r\n";
			return response.str();
		}
		// Check if path is a directory
		std::string	fs_path = matched_route->root + path.substr(matched_path.length());
		struct stat path_stat;
		if (stat(fs_path.c_str(), &path_stat) == 0 && S_ISDIR(path_stat.st_mode))
			return handleDirectoryRequest(fs_path, *matched_route, servers[i]);
		// Check for CGI execution based on file extension
		for (const auto& ext : matched_route->cgi_extensions)
		{
			if (fs_path.size() >= ext.size() && fs_path.compare(fs_path.size() - ext.size(), ext.size(), ext) == 0)
				return executeCGI(fs_path, path, servers[i]);
		}

		if (method == "GET")
			return handleGetRequest(path, servers[i]);
		else if (method == "POST")
			// return "HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\nPOST";
			return handlePostRequest(path, request_s, matched_route, servers[i]);
		else if (method == "DELETE")
			return "HTTP/1.1 200 OK\r\nContent-Length: 6\r\n\r\nDELETE";
		else
			return constructErrorResponse("405", servers[i]);	
	}
	else
		return constructErrorResponse("404", servers[i]);
}