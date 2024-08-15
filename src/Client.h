#ifndef WS_CLIENT_H
#define WS_CLIENT_H

#include <vector>

#include "c_api/ClientSocket.h"
#include "http/Request.h"
#include "http/Response.h"
#include "utils/ICallback.h"
#include "utils/unique_ptr.h"

class Client {
  private:
    Client(const Client&);
    Client& operator=(const Client&);
    Client();
  public:
    Client(utils::unique_ptr<c_api::ClientSocket> s);
    class ClientReadCallback : public utils::ICallback {
      public:
        ClientReadCallback(Client& c);
        // read from sock,
        virtual int call(int fd);
      private:
        Client& _client;
    };
  private:
    utils::unique_ptr<c_api::ClientSocket> _client_sock;
    std::vector<char> _buf;  // string?
    http::Request _rq;
    http::Response _rs;
};

#endif  // WS_CLIENT_H
