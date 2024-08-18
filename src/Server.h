#ifndef WS_SERVER_H
#define WS_SERVER_H

#include <netinet/in.h>

#include <map>
#include <string>

#include "c_api/MasterSocket.h"
#include "Client.h"
#include "utils/ICallback.h"
#include "utils/unique_ptr.h"

class Server {
  private:
    Server();
    Server(const Server&);
    Server& operator=(const Server&);
  public:
    // create master socket, register read callback for master socket in event manager
    Server(const std::string& name, in_addr_t ip, in_port_t port);
    ~Server();
    class MasterSocketCallback : public utils::ICallback {
      public:
        MasterSocketCallback(Server& server);
        // accept, create new client, register read callback for client,
        virtual int call(int fd);
      private:
        Server& _server;
    };
    // if client is ready to write register wr callback,
    // if client timed out, rm it from map
    void check_clients();
    const std::string& name();
  private:
    std::string _name;
    c_api::MasterSocket _master_sock;
    std::map<int, utils::unique_ptr<Client> > _clients;
    typedef std::map<int, utils::unique_ptr<Client> >::iterator client_iterator;
};

#endif  // WS_SERVER_H
