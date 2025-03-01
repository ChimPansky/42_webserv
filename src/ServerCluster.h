#ifndef WS_SERVER_CLUSTER_H
#define WS_SERVER_CLUSTER_H

#include <Config.h>
#include <MasterSocket.h>
#include <Server.h>
#include <shared_ptr.h>
#include <signal.h>

#include "ClientSession.h"

class ClientSession;

class ServerCluster {
  public:
    static void Init(const config::Config& config);
    static void StopHandler();
    static void Run();
    static utils::shared_ptr<Server> ChooseServer(int master_fd, const http::Request& rq);
    static void FillResponseHeaders(http::Response& rs);
    void PrintDebugInfo() const;
    static inline const char* kServerClusterName() { return "ft_webserv"; }

  private:
    ServerCluster(const config::Config& config);

    void CreateServers_(const config::Config& config);
    void MapListenersToServer_(const std::vector<std::pair<in_addr_t, in_port_t> >& listeners,
                               utils::shared_ptr<Server>& serv);
    int GetListenerFdForServer_(const std::pair<unsigned int, unsigned short>& address);
    void CheckClients_() throw();
    void KillAllClients_() throw();  // easy strat for bad_alloc

  private:
    class MasterSocketCallback : public c_api::ICallback {
      public:
        MasterSocketCallback(ServerCluster& cluster) : cluster_(cluster) {}
        // accept, create new client, register read callback for client,
        virtual void Call(int fd);

      private:
        ServerCluster& cluster_;
    };

  private:
    // Sockets
    std::map<int /*fd*/, utils::unique_ptr<c_api::MasterSocket> > sockets_;
    typedef std::map<int /*fd*/, utils::unique_ptr<c_api::MasterSocket> >::iterator SocketsIt;

    // Servers
    std::vector<utils::shared_ptr<Server> > servers_;
    typedef std::vector<utils::shared_ptr<Server> >::const_iterator ServersConstIt;

    std::map<int /*fd*/, std::vector<utils::shared_ptr<Server> > > sockets_to_servers_;

    // Clients
    std::map<int /*fd*/, utils::unique_ptr<ClientSession> > clients_;
    typedef std::map<int, utils::unique_ptr<ClientSession> >::iterator client_iterator;

    unsigned keep_alive_timeout_s_;

  private:
    static volatile sig_atomic_t run_;
    static utils::unique_ptr<ServerCluster> instance_;
};

#endif  // WS_SERVER_CLUSTER_H
