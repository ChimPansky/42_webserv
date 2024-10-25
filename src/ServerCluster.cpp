#include "ServerCluster.h"

#include "server/Server.h"
#include "ClientSession.h"
#include "c_api/EventManager.h"
#include "c_api/utils.h"
#include "config/Config.h"

volatile bool ServerCluster::run_ = false;

ServerCluster::ServerCluster(const config::Config& config)
{
    c_api::EventManager::init(c_api::MT_EPOLL);
    CreateServers_(config);
}

// smth like
void ServerCluster::Start(const config::Config& config)
{
    // register signal for ^C, switch run on that
    run_ = true;
    ServerCluster cluster(config);
    // cluster.PrintServers();
    while (run_) {
        c_api::EventManager::get().CheckOnce();
        cluster.CheckClients_();
    }
}

void ServerCluster::Stop()
{
    run_ = false;
}

void ServerCluster::PrintServers() const
{
    for (ServersConstIt cit = servers_.begin(); cit != servers_.end(); ++cit) {
        LOG(INFO) << "Hi, i am Server " << (*cit)->name() << ". My config is: ";
        (*cit)->server_config().Print();
        LOG(INFO);
    }
}

void ServerCluster::CheckClients_()
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

void ServerCluster::CreateServers_(const config::Config& config)
{
    for (config::ServerConfConstIt serv_conf_it = config.http_config().server_configs().begin();
         serv_conf_it != config.http_config().server_configs().end(); ++serv_conf_it) {
        utils::shared_ptr<Server> serv(new Server(*serv_conf_it));
        servers_.push_back(serv);
        MapListenersToServer_(serv_conf_it->listeners(), serv);
    }
}

void ServerCluster::MapListenersToServer_(
    const std::vector<std::pair<in_addr_t, in_port_t> >& listeners, utils::shared_ptr<Server> serv)
{
    for (config::ListenersConfConstIt l_it = listeners.begin(); l_it != listeners.end(); ++l_it) {
        struct sockaddr_in addr = c_api::GetIPv4SockAddr(l_it->first, l_it->second);
        int sockfd = GetListenerFd_(addr);
        if (sockfd == -1) {
            sockfd = CreateListener_(addr);
        }
        sockets_to_servers_[sockfd].push_back(serv);
        LOG(INFO) << serv->name() << " is listening on " << c_api::IPv4ToString(l_it->first) << ":"
                  << l_it->second << " (fd: " << sockfd << ")";
    }
}

int ServerCluster::CreateListener_(struct sockaddr_in addr)
{
    utils::unique_ptr<c_api::MasterSocket> listener(new c_api::MasterSocket(addr));
    int sockfd = listener->sockfd();
    if (c_api::EventManager::get().RegisterCallback(
            sockfd, c_api::CT_READ,
            utils::unique_ptr<c_api::ICallback>(new MasterSocketCallback(*this))) != 0) {
        LOG(FATAL) << "Could not register callback for listener: " << sockfd;
    }
    sockets_[sockfd] = listener;
    return sockfd;
}

int ServerCluster::GetListenerFd_(struct sockaddr_in addr)
{
    for (SocketsIt sock_it = sockets_.begin(); sock_it != sockets_.end(); ++sock_it) {
        if (sock_it->second->IsSameSockAddr(addr)) {
            return sock_it->second->sockfd();
        }
    }
    return -1;
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
    LOG(INFO) << "New incoming connection on: " << fd;
    cluster_.clients_[fd] = utils::unique_ptr<ClientSession>(new ClientSession(client_sock, fd));
}
