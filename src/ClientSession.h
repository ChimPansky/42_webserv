#ifndef WS_CLIENT_H
#define WS_CLIENT_H

#include <vector>

#include "c_api/ClientSocket.h"
#include "http/Request.h"
#include "http/Response.h"
#include "utils/ICallback.h"
#include "utils/unique_ptr.h"

class ClientSession {
  private:
    ClientSession(const ClientSession&);
    ClientSession& operator=(const ClientSession&);
    ClientSession();

  public:
    ClientSession(utils::unique_ptr<c_api::ClientSocket> client_sock);
    ~ClientSession();
    bool connection_closed() const;
    bool IsRequestReady() const;
    void ProcessNewData(ssize_t bytes_recvdd);
    class ClientReadCallback : public utils::ICallback {
      public:
        ClientReadCallback(ClientSession& client);
        // read from sock,
        virtual void Call(int fd);

      private:
        ClientSession& _client;
    };
    class ClientWriteCallback : public utils::ICallback {
      public:
        ClientWriteCallback(ClientSession& client);
        // read from sock,
        virtual void Call(int fd);

      private:
        ClientSession& _client;
    };

  private:
    utils::unique_ptr<c_api::ClientSocket> _client_sock;
    std::vector<char> _buf;  // string?
    size_t _buf_send_idx;
    http::Request _rq;
    http::Response _rs;
    bool _connection_closed;
};

#endif  // WS_CLIENT_H
