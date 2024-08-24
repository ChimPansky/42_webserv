#ifndef WS_CLIENT_H
#define WS_CLIENT_H

#include <vector>

#include "c_api/ClientSocket.h"
#include "http/Request.h"
#include "http/RequestParser.h"
#include "http/Response.h"
#include "utils/ICallback.h"
#include "utils/unique_ptr.h"

class Client {
  private:
    Client(const Client&);
    Client& operator=(const Client&);
    Client();

  public:
    Client(utils::unique_ptr<c_api::ClientSocket> client_sock);
    ~Client();
    bool connection_closed() const;
    bool IsRequestReady() const;
    void ProcessNewData(ssize_t bytes_recvdd);
    class ClientReadCallback : public utils::ICallback {
      public:
        ClientReadCallback(Client& client);
        // read from sock,
        virtual void Call(int fd);

      private:
        Client& _client;
    };
    class ClientWriteCallback : public utils::ICallback {
      public:
        ClientWriteCallback(Client& client);
        // read from sock,
        virtual void Call(int fd);

      private:
        Client& _client;
    };

  private:
    utils::unique_ptr<c_api::ClientSocket> _client_sock;
    std::vector<char> _buf;  // string?
    size_t _buf_send_idx;
    http::Request _rq;
    http::RequestParser _rq_parser;
    http::Response _rs;
    bool _connection_closed;
};

#endif  // WS_CLIENT_H
