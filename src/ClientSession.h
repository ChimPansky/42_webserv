#ifndef WS_CLIENT_H
#define WS_CLIENT_H

#include <vector>

#include "c_api/ClientSocket.h"
#include "c_api/EventManager.h"
#include "http/RequestBuilder.h"
#include "http/Response.h"
#include "utils/unique_ptr.h"

class ClientSession {
  private:
    ClientSession(const ClientSession&);
    ClientSession& operator=(const ClientSession&);
    ClientSession();

  public:
    ClientSession(utils::unique_ptr<c_api::ClientSocket> client_sock, int master_sock_fd);
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
    int master_socket_fd_;  // to choose correct server later
    std::vector<char> client_buf_;  // string?
    size_t client_buf_idx_;
    http::RequestBuilder rq_builder_;
    http::Response rs_;
    bool connection_closed_;
};

#endif  // WS_CLIENT_H
