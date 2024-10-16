#ifndef WS_CLIENT_H
#define WS_CLIENT_H

#include <sys/types.h>

#include <vector>

#include "c_api/ClientSocket.h"
#include "c_api/multiplexers/ICallback.h"
#include "http/RequestBuilder.h"
#include "utils/unique_ptr.h"

#define CLIENT_RD_CALLBACK_RD_SZ 20

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
    void ProcessNewData(size_t bytes_recvd);
    void CloseConnection();
    void PrepareResponse();  // later: get this from server
    class ClientReadCallback : public c_api::ICallback {
      public:
        ClientReadCallback(ClientSession& client);
        virtual void Call(int);

      private:
        ClientSession& client_;
    };
    class ClientWriteCallback : public c_api::ICallback {
      public:
        ClientWriteCallback(ClientSession& client);
        virtual void Call(int);

      private:
        ClientSession& client_;
    };

  private:
    utils::unique_ptr<c_api::ClientSocket> client_sock_;
    int master_socket_fd_;   // to choose correct server later
    std::vector<char> buf_;  // string?
    size_t buf_send_idx_;
    http::RequestBuilder rq_builder_;
    http::Request rq_;
    bool connection_closed_;

    Server* virtual_server;
};

#endif  // WS_CLIENT_H
