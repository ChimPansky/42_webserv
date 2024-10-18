#ifndef WS_SERVER_CLUSTER_H
#define WS_SERVER_CLUSTER_H

#include <vector>

#include "c_api/MasterSocket.h"
#include "config/Config.h"
#include "utils/shared_ptr.h"
#include "utils/unique_ptr.h"

class Server;
class ClientSession;

class ServerCluster {
  public:
    static void Start(const config::Config& config);
    static void Stop();

    const std::vector<utils::shared_ptr<Server> >& servers() const;

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
    std::map<int /*fd*/, std::vector<utils::shared_ptr<Server> > > sockets_to_servers_;

    // Clients
    std::map<int, utils::unique_ptr<ClientSession> > clients_;
    typedef std::map<int, utils::unique_ptr<ClientSession> >::iterator client_iterator;
    // if client is ready to write register wr callback,
    // if client timed out, rm it from map
    void CheckClients();

    static volatile bool run_;
};

#endif  // WS_SERVER_CLUSTER_H
