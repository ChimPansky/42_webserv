#include "ServerCluster.h"

#include <Config.h>
#include <EventManager.h>
#include <Server.h>
#include <c_api_utils.h>

#include <string>

#include "ClientSession.h"
#include <shared_ptr.h>

namespace {
void SigIntHandler(int /*signum*/)
{
    LOG(INFO) << " SIGINT caught, shutting down...";
    ServerCluster::StopHandler();
}
}

volatile sig_atomic_t ServerCluster::run_ = false;
utils::unique_ptr<ServerCluster> ServerCluster::instance_;

void ServerCluster::Init(const config::Config& config)
{
    signal(SIGINT, SigIntHandler);
    instance_ = utils::unique_ptr<ServerCluster>(new ServerCluster(config));
}

void ServerCluster::StopHandler()
{
    run_ = false;
}

ServerCluster::ServerCluster(const config::Config& config)
{
    utils::Logger::get().set_severity_threshold(config.error_log_level());
    c_api::EventManager::init(config.mx_type());
    CreateServers_(config);
}

void ServerCluster::Run()
{
    run_ = true;
    // instance_->PrintDebugInfo();
    while (run_) {
        c_api::EventManager::get().CheckOnce();
        instance_->CheckClients_();
    }
}

utils::shared_ptr<Server> ServerCluster::ChooseServer(int master_fd, const http::Request& rq)
{
    std::pair<MatchType, std::string> best_match(NO_MATCH, std::string());
    utils::shared_ptr<Server> matched_server;

    for (ServersConstIt it = instance_->sockets_to_servers_[master_fd].begin();
         it != instance_->sockets_to_servers_[master_fd].end(); ++it) {
        std::pair<MatchType, std::string> match_result = (*it)->MatchedServerName(rq);

        if ((match_result.second.length() > best_match.second.length() &&
             match_result.first == best_match.first) ||
            match_result.first > best_match.first) {
            best_match = match_result;
            matched_server = *it;
        }
    }
    if (best_match.second.empty()) {
        return instance_->sockets_to_servers_[master_fd][0];
    }

    return matched_server;
}

void ServerCluster::PrintDebugInfo() const
{
    for (ServersConstIt cit = servers_.begin(); cit != servers_.end(); ++cit) {
        LOG(DEBUG) << "Hi, i am Server " << (*cit)->name() << ". My config is: ";
        (*cit)->server_config().Print();
        LOG(DEBUG);
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

// Wrap to serverHolder class c'tor probably as well?
// then server holder will keep servers, the map socket to servers
void ServerCluster::CreateServers_(const config::Config& config)
{
    for (config::ServerConfConstIt serv_conf_it = config.http_config().server_configs().begin();
         serv_conf_it != config.http_config().server_configs().end(); ++serv_conf_it) {
        servers_.push_back(utils::shared_ptr<Server>(new Server(*serv_conf_it)));
        MapListenersToServer_(serv_conf_it->listeners(), servers_.back());
    }
}

void ServerCluster::MapListenersToServer_(
    const std::vector<std::pair<in_addr_t, in_port_t> >& listeners, utils::shared_ptr<Server>& serv)
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
    SocketsIt acceptor_it = cluster_.sockets_.find(fd);
    if (acceptor_it == cluster_.sockets_.end()) {
        LOG(ERROR) << " this should never happen, no such socket: " << fd;
        return;
    }
    c_api::MasterSocket& acceptor = *acceptor_it->second;
    utils::unique_ptr<c_api::ClientSocket> client_sock = acceptor.Accept();
    if (!client_sock) {
        LOG(ERROR) << "error accepting connection on: "
                   << c_api::PrintIPv4SockAddr(acceptor.addr_in());
        perror("MasterSocket::Accept");
        return;
    }
    LOG(INFO) << "New incoming connection on: " << c_api::PrintIPv4SockAddr(acceptor.addr_in());
    LOG(INFO) << "From: " << c_api::PrintIPv4SockAddr(client_sock->addr_in());
    cluster_.clients_[fd] = utils::unique_ptr<ClientSession>(new ClientSession(client_sock, fd));
}
