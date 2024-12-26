#include "ServerCluster.h"

#include <ChildProcessesManager.h>
#include <EventManager.h>
#include <c_api_utils.h>
#include <errors.h>
#include <time_utils.h>

namespace {
void SigIntHandler(int /*signum*/)
{
    LOG(INFO) << " SIGINT caught, shutting down...";
    ServerCluster::StopHandler();
}
}  // namespace

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
    c_api::ChildProcessesManager::init();
    CreateServers_(config);
    for (SocketsIt sock_it = sockets_.begin(); sock_it != sockets_.end(); ++sock_it) {
        LOG_IF(FATAL, !c_api::EventManager::TryRegisterCallback(
                          sock_it->second->sockfd(), c_api::CT_READ,
                          utils::unique_ptr<c_api::ICallback>(new MasterSocketCallback(*this))))
            << "Could not register callback for listener: " << sock_it->second->sockfd();
    }
}

void ServerCluster::CreateServers_(const config::Config& config)
{
    for (config::ServerConfConstIt serv_conf_it = config.http_config().server_configs().begin();
         serv_conf_it != config.http_config().server_configs().end(); ++serv_conf_it) {
        servers_.push_back(utils::shared_ptr<Server>(
            new Server(*serv_conf_it, config.http_config().error_pages())));
        MapListenersToServer_(serv_conf_it->listeners(), servers_.back());
    }
    keep_alive_timeout_s_ = config.http_config().keepalive_timeout();
}

void ServerCluster::MapListenersToServer_(
    const std::vector<std::pair<in_addr_t, in_port_t> >& listeners, utils::shared_ptr<Server>& serv)
{
    for (config::ListenersConfConstIt l_it = listeners.begin(); l_it != listeners.end(); ++l_it) {
        int sockfd = GetListenerFdForServer_(*l_it);
        sockets_to_servers_[sockfd].push_back(serv);
        LOG(INFO) << serv->name() << " is listening on " << c_api::IPv4ToString(l_it->first) << ":"
                  << l_it->second << " (fd: " << sockfd << ")";
    }
}

int ServerCluster::GetListenerFdForServer_(const std::pair<unsigned int, unsigned short>& address)
{
    struct sockaddr_in addr = c_api::GetIPv4SockAddr(address.first, address.second);
    for (SocketsIt sock_it = sockets_.begin(); sock_it != sockets_.end(); ++sock_it) {
        if (sock_it->second->IsSameSockAddr(addr)) {
            return sock_it->second->sockfd();
        }
    }
    utils::unique_ptr<c_api::MasterSocket> listener(new c_api::MasterSocket(addr));
    int sockfd = listener->sockfd();
    sockets_[sockfd] = listener;
    return sockfd;
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
    return (best_match.first == NO_MATCH ? instance_->sockets_to_servers_[master_fd][0]
                                         : matched_server);
}

void ServerCluster::PrintDebugInfo() const
{
    for (ServersConstIt cit = servers_.begin(); cit != servers_.end(); ++cit) {
        LOG(DEBUG) << "Hi, i am Server " << (*cit)->name()
                   << ". My config is: " << (*cit)->GetDebugString();
        LOG(DEBUG);
    }
}

void ServerCluster::Run()
{
    run_ = true;
    // instance_->PrintDebugInfo();
    while (run_) {
        try {
            c_api::EventManager::CheckOnce();
            c_api::ChildProcessesManager::CheckOnce();
        } catch (const std::bad_alloc& e) {
            instance_->KillAllClients_();
        }
        instance_->CheckClients_();
        LOG(INFO) << utils::GetFormatedTime();
    }
}


void ServerCluster::CheckClients_() throw()
{
    UnixTimestampS now = utils::Now();
    client_iterator it = clients_.begin();
    while (it != clients_.end()) {
        ClientSession& client = *it->second;
        if (client.connection_closed()) {
            client_iterator tmp = it;
            ++it;
            clients_.erase(tmp);
            continue;
        }
        if (now - client.last_activity_time() > keep_alive_timeout_s_) {
            client.CloseConnection();
        }
        ++it;
    }
}

void ServerCluster::KillAllClients_() throw()
{
    for (client_iterator it = clients_.begin(); it != clients_.end(); ++it) {
        it->second->CloseConnection();
    }
}

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
        LOG(ERROR) << "error accepting connection on "
                   << c_api::PrintIPv4SockAddr(acceptor.addr_in()) << ": "
                   << utils::GetSystemErrorDescr();
        return;
    }
    const int client_fd = client_sock->sockfd();
    LOG(INFO) << "New incoming connection on: " << c_api::PrintIPv4SockAddr(acceptor.addr_in())
              << ", client_fd: " << client_fd
              << " From: " << c_api::PrintIPv4SockAddr(client_sock->addr_in());
    cluster_.clients_[client_fd] = utils::unique_ptr<ClientSession>(new ClientSession(
        client_sock, fd, cluster_.sockets_to_servers_[acceptor.sockfd()][/*kDefaultServer*/ 0]));
}

void ServerCluster::FillResponseHeaders(http::Response& rs)
{
    rs.AddHeader(std::make_pair("Server", kServerClusterName()));
    rs.AddHeader(std::make_pair("Date", utils::GetFormatedTime()));
    // rs.AddHeader(std::make_pair(
    //     "Connection", "Close"));  // TODO fix or embrace (move to different place if embraced)
}
