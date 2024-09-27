#include "Server.h"

Server::Server(const std::string& name) : name_(name) {}

const std::string& Server::name() { return name_; }
