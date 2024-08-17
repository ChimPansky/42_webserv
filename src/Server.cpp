#include "Server.h"

#include <iostream>

#include "c_api/EventManager.h"

// create master socket, register read callback for master socket in event manager
Server::Server(const std::string& name, in_addr_t ip, in_port_t port)
  : _name(name), _master_sock(ip, port)
{
    c_api::EventManager::get()
        .RegisterReadCallback(_master_sock.sockfd(), utils::unique_ptr<utils::ICallback>(new MasterSocketCallback(*this)));
}

Server::MasterSocketCallback::MasterSocketCallback(Server& server)
  : _server(server)
{}

// accept, create new client, register read callback for client,
void Server::MasterSocketCallback::Call(int fd)
{
    // assert fd = master.sockfd
    utils::unique_ptr<c_api::ClientSocket> client_sock = _server._master_sock.Accept();
    if (fd < 0) {
        // error
        std::cerr << "Error accepting connection on " << _server._name << std::endl;
        return ;
    }
    _server._clients[fd] = utils::unique_ptr<Client>(new Client(client_sock));
    std::cout << "New incoming connection on " << _server._name << std::endl;
}

void Server::CheckClients() {
    client_iterator it = _clients.begin();
    while (it != _clients.end()) {
        if (it->second->connection_closed()) {
            client_iterator tmp = it;
            ++it;
            _clients.erase(tmp);
        } else {
            ++it;
        }
    }
}
