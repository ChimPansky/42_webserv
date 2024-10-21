#include "Server.h"

#include <iostream>

#include "c_api/utils.h"

// create master socket, register read callback for master socket in event manager
Server::Server(const config::ServerConfig& server_config) : server_config_(server_config)
{}

Server::~Server()
{}

const std::string Server::name() const
{
    if (server_config_.server_names().empty()) {
        return "";
    }
    return (server_config_.server_names()[0]);
}

const config::ServerConfig& Server::server_config() const
{
    return server_config_;
}
