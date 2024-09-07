#include "ServerCluster.h"

#include "Server.h"
#include "c_api/EventManager.h"
#include "c_api/utils.h"
#include "utils/logger.h"

volatile bool ServerCluster::run_ = false;

// testing with one
ServerCluster::ServerCluster(const Config& /*config*/)
{
    std::vector<std::pair<in_addr_t, in_port_t> > listeners;

    listeners.push_back(std::make_pair(c_api::IPv4FromString("localhost"), in_port_t(8081)));
    listeners.push_back(std::make_pair(c_api::IPv4FromString("localhost"), in_port_t(8082)));

    utils::shared_ptr<Server> serv(new Server("Sserv"));  // constructor of server block
    servers_.push_back(serv);

    typedef std::vector<std::pair<in_addr_t, in_port_t> >::iterator ListenersIt;
    for (ListenersIt it = listeners.begin(); it != listeners.end(); ++it) {
        // TODO use this func in config builder and iterate over sockaddr right away instead of
        // building it
        int sockfd = -1;
        struct sockaddr_in addr = c_api::GetIPv4SockAddr(it->first, it->second);
        for (SocketsIt sock_it = sockets_.begin(); sock_it != sockets_.end(); ++sock_it) {
            if (sock_it->second->IsSameSockAddr(addr)) {
                sockets_to_servers_[sock_it->second->sockfd()].push_back(serv);
                sockfd = sock_it->second->sockfd();
                break;
            }
        }
        if (sockfd == -1) {
            utils::unique_ptr<c_api::MasterSocket> listener(new c_api::MasterSocket(addr));
            sockfd = listener->sockfd();
            sockets_to_servers_[sockfd].push_back(serv);
            c_api::EventManager::get().RegisterCallback(
                sockfd,
                utils::unique_ptr<c_api::EventManager::ICallback>(new MasterSocketCallback(*this)));
            sockets_[sockfd] = listener;
        }
        LOG(INFO) << serv->name() << " is listening on " << c_api::IPv4ToString(it->first) << ":"
                  << it->second << " (fd: " << sockfd << ")";
    }
}

void ServerCluster::Stop()
{
    run_ = false;
}

// smth like
void ServerCluster::Start(const Config& config)
{
    // register signal for ^C, switch run on that
    run_ = true;
    ServerCluster cluster(config);
    while (run_) {
        c_api::EventManager::get().CheckOnce();
        cluster.CheckClients();
    }
}

void ServerCluster::CheckClients()
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

ServerCluster::MasterSocketCallback::MasterSocketCallback(ServerCluster& cluster)
    : cluster_(cluster)
{}

// accept, create new client, register read callback for client,
void ServerCluster::MasterSocketCallback::Call(int fd)
{
    SocketsIt acceptor = cluster_.sockets_.find(fd);
    if (acceptor == cluster_.sockets_.end()) {
        LOG(ERROR) << " this should never happen, no such socket: " << fd;
        return;
    }
    // utils::shared_ptr<Server> serv = cluster_.sockets_to_servers_[fd][0]; // Choose server
    // instead
    utils::unique_ptr<c_api::ClientSocket> client_sock = acceptor->second->Accept();
    if (!client_sock) {
        LOG(ERROR) << "error accepting connection on: " << fd;  // add perror
        return;
    }
    cluster_.clients_[fd] = utils::unique_ptr<ClientSession>(new ClientSession(client_sock, fd));
    LOG(INFO) << "New incoming connection on: " << fd;
}

c_api::EventManager::CallbackType ServerCluster::MasterSocketCallback::callback_mode() {
    return c_api::EventManager::CT_READ;    // MasterSocketCallback is always in read mode
}
