#ifndef WS_CLIENT_H
#define WS_CLIENT_H

#include <vector>

#include "http/Request.h"
#include "http/Response.h"
#include "utils/ICallback.h"

class Client {
  private:
    Client(const Client&);
    Client& operator=(const Client&);
  public:
    Client();
    class ClientReadCallback : utils::ICallback {
      public:
        ClientReadCallback(Client& c);
        // read from sock, 
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
        Client& _server;
    };
  private:
    std::vector<char> _buf;  // string?
    http::Request _rq;
    http::Response _rs;

};

#endif  // WS_CLIENT_H
