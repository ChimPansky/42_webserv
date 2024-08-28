#include "Server.h"

#include <iostream>

#include "c_api/EventManager.h"
#include "c_api/utils.h"

// create master socket, register read callback for master socket in event manager
Server::Server(const std::string& name, in_addr_t ip, in_port_t port)
    : name_(name), _master_sock(ip, port)
{
    c_api::EventManager::get().RegisterReadCallback(
        _master_sock.sockfd(),
        utils::unique_ptr<utils::ICallback>(new MasterSocketCallback(*this)));
    std::cout << "Server " << name_ << " is listening on " << c_api::IPv4ToString(ip) << ":" << port
              << " ..." << std::endl;
}

Server::~Server()
{
    std::cout << "Server " << name_ << " was shut down." << std::endl;
}

Server::MasterSocketCallback::MasterSocketCallback(Server& server) : server_(server)
{}

// accept, create new client, register read callback for client,
void Server::MasterSocketCallback::Call(int fd)
{
    // assert fd = master.sockfd
    utils::unique_ptr<c_api::ClientSocket> client_sock = server_._master_sock.Accept();
    if (fd < 0) {
        // error
        std::cerr << "Error accepting connection on " << server_.name_ << std::endl;
        return;
    }
    server_.clients_[fd] = utils::unique_ptr<ClientSession>(new ClientSession(client_sock));
    std::cout << "New incoming connection on " << server_.name_ << std::endl;
}

void Server::CheckClients()
{
    client_iterator it = clients_.begin();
    while (it != clients_.end()) {
        ClientSession& client = *it->second;
        if (client.connection_closed()) {
            client_iterator tmp = it;
            ++it;
            clients_.erase(tmp);
            continue;
        }
        ++it;
    }
}
