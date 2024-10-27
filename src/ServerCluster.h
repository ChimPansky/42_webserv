#ifndef WS_SERVER_CLUSTER_H
#define WS_SERVER_CLUSTER_H

#include <vector>

#include <Server.h>
#include <Config.h>
#include <unique_ptr.h>
#include <MasterSocket.h>
#include <Config.h>
#include <shared_ptr.h>
#include <multiplexers/ICallback.h>
#include <MasterSocket.h>
#include "ClientSession.h"

#include <signal.h>

class ClientSession;

class ServerCluster {
  public:
    ServerCluster(const config::Config& config);
    void Run();
    static void StopHandler();
    void PrintDebugInfo() const;
    utils::shared_ptr<Server> ChooseServer() const;

  private:
    class MasterSocketCallback : public c_api::ICallback {
      public:
        MasterSocketCallback(ServerCluster& cluster);
        // accept, create new client, register read callback for client,
        virtual void Call(int fd);

      private:
        ServerCluster& cluster_;
    };


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

    void CreateServers_(const config::Config& config);
    void MapListenersToServer_(const std::vector<std::pair<in_addr_t, in_port_t> >& listeners,
                               utils::shared_ptr<Server>& serv);
    int CreateListener_(struct sockaddr_in addr);
    int GetListenerFd_(struct sockaddr_in addr);
    void CheckClients_();

    static volatile sig_atomic_t run_;
};

#endif  // WS_SERVER_CLUSTER_H
