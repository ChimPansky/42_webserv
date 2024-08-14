#ifndef WS_SERVER_H
#define WS_SERVER_H

#include "c_api/MasterSocket.h"
#include "c_api/EventManager.h"
#include "utils/ICallback.h"

#include <netinet/in.h>

#include "Client.h"

#include <memory>
#include <map>

class Server {
  private:
    Server();
    Server(const Server&);
    Server& operator=(const Server&);
  public:
    // create master socket, register read callback for master socket in event manager
    Server(in_addr_t ip, in_port_t port, c_api::EventManager&);
    class MasterSocketCallback : utils::ICallback {
      public:
        MasterSocketCallback(Server& s);
        // accept, create new client, register read callback for client,
        virtual bool call();
        // {
        //     int fd = _server._master_sock.accept();
        //     if (fd < 0) {
        //         // error
        //         return false;
        //     }
        //     std::auto_ptr<Client> client(new Client());
        //     _server._event_manager.register_read_callback(fd, client->get_rd_callback());
        //     _server._clients.insert({fd, client});
        // }
      private:
        Server& _server;
    };
    // if client is ready to write register wr callback,
    // if client timed out, rm it from map
    void check_clients();
  private:
    c_api::MasterSocket _master_sock;
    std::map<int, std::auto_ptr<Client> > _clients;
    c_api::EventManager& _event_manager;
};

#endif  // WS_SERVER_H
