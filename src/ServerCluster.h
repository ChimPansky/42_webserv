#ifndef WS_SERVER_CLUSTER_H
#define WS_SERVER_CLUSTER_H

#include <vector>

#include "server/Server.h"
#include "config/Config.h"
#include "utils/unique_ptr.h"
#include "c_api/MasterSocket.h"
#include "config/Config.h"
#include "utils/shared_ptr.h"
#include "c_api/multiplexers/ICallback.h"
#include "c_api/MasterSocket.h"
#include "ClientSession.h"

class ClientSession;

class ServerCluster {
  public:
    static void Start(const config::Config& config);
    static void Stop();
    void PrintServers() const;

  private:
    class MasterSocketCallback : public c_api::ICallback {
      public:
        MasterSocketCallback(ServerCluster& cluster);
        // accept, create new client, register read callback for client,
        virtual void Call(int fd);

      private:
        ServerCluster& cluster_;
    };

    ServerCluster(const config::Config&);

    // Sockets
    std::map<int /*fd*/, utils::unique_ptr<c_api::MasterSocket> > sockets_;
    typedef std::map<int /*fd*/, utils::unique_ptr<c_api::MasterSocket> >::iterator SocketsIt;

    // Servers
    std::vector<utils::shared_ptr<Server> > servers_;
    typedef std::vector<utils::shared_ptr<Server> >::const_iterator ServersConstIt;

    std::map<int /*fd*/, std::vector<utils::shared_ptr<Server> > > sockets_to_servers_;

    // Clients
    std::map<int, utils::unique_ptr<ClientSession> > clients_;
    typedef std::map<int, utils::unique_ptr<ClientSession> >::iterator client_iterator;

    void CreateServers_(const config::Config& config);
    void MapListenersToServer_(const std::vector<std::pair<in_addr_t, in_port_t> >& listeners,
                               utils::shared_ptr<Server> serv);
    int CreateListener_(struct sockaddr_in addr);
    int GetListenerFd_(struct sockaddr_in addr);
    void CheckClients_();

    static volatile bool run_;
};

#endif  // WS_SERVER_CLUSTER_H
