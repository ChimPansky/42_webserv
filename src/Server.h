#ifndef WS_SERVER_H
#define WS_SERVER_H

#include <netinet/in.h>

#include <map>
#include <string>

#include "ClientSession.h"
#include "c_api/MasterSocket.h"
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
        virtual void Call(int fd);

      private:
        Server& server_;
    };
    // if client is ready to write register wr callback,
    // if client timed out, rm it from map
    void CheckClients();
    const std::string& name();

  private:
    std::string name_;
    c_api::MasterSocket master_sock_;
    std::map<int, utils::unique_ptr<ClientSession> > clients_;
    typedef std::map<int, utils::unique_ptr<ClientSession> >::iterator client_iterator;
};

#endif  // WS_SERVER_H
