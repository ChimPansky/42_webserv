#ifndef WS_CLIENT_H
#define WS_CLIENT_H

#include <vector>

#include "c_api/ClientSocket.h"
#include "c_api/EventManager.h"
#include "http/Request.h"
#include "http/Response.h"
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
    class ClientReadCallback : public c_api::EventManager::ICallback {
      public:
        ClientReadCallback(ClientSession& client);
        // read from sock,
        virtual void Call(int fd);

      private:
        ClientSession& client_;
    };
    class ClientWriteCallback : public c_api::EventManager::ICallback {
      public:
        ClientWriteCallback(ClientSession& client);
        // read from sock,
        virtual void Call(int fd);

      private:
        ClientSession& client_;
    };

  private:
    utils::unique_ptr<c_api::ClientSocket> client_sock_;
    std::vector<char> buf_;  // string?
    size_t buf_send_idx_;
    http::Request rq_;
    http::Response rs_;
    bool connection_closed_;
};

#endif  // WS_CLIENT_H
