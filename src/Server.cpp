#include "Server.h"

// create master socket, register read callback for master socket in event manager
Server::Server(const std::string& name)
    : name_(name)
{

}

Server::~Server()
{

}

const std::string& Server::name() const {
    return name_;
}
