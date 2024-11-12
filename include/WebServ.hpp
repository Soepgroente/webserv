#pragma once

#include <string>
#include <iostream>

class WebServ {
public:
    WebServ(const std::string& name);
    ~WebServ();

    void start();
    void stop();

private:
    std::string name;
	int port;
};