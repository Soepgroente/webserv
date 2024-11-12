#include "WebServ.hpp"

WebServ::WebServ(const std::string& name) : name(name) {
    std::cout << "WebServ " << name << " created." << std::endl;
}

WebServ::~WebServ() {
    std::cout << "WebServ " << name << " destroyed." << std::endl;
}

void WebServ::start() {
    std::cout << "WebServ " << name << " started." << std::endl;
}

void WebServ::stop() {
    std::cout << "WebServ " << name << " stopped." << std::endl;
}