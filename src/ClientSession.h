#ifndef WS_CLIENT_H
#define WS_CLIENT_H

#include <sys/types.h>
#include <vector>

#include "c_api/ClientSocket.h"
#include "http/RequestBuilder.h"
#include "c_api/multiplexers/ICallback.h"
#include "http/Response.h"
#include "utils/unique_ptr.h"

class ClientSession {
  public:
    enum ProcessState {
        PS_ONGOING,
        PS_DONE
    };

  private:
    ClientSession(const ClientSession&);
    ClientSession& operator=(const ClientSession&);
    ClientSession();

  public:
    ClientSession(utils::unique_ptr<c_api::ClientSocket> client_sock, int master_sock_fd);
    ~ClientSession();
    bool connection_closed() const;
    bool IsRequestReady() const;
    ProcessState ProcessRead(ssize_t bytes_recvd); // not used now , use when building request...
    void PrepareResponse(); // later: get this from server
    class ClientReadCallback : public c_api::ICallback {
      public:
        ClientReadCallback(ClientSession& client);
        // Server reads from client socket,
        virtual void Call(int);

      private:
        ClientSession& client_;
    };
    class ClientWriteCallback : public c_api::ICallback {
      public:
        ClientWriteCallback(ClientSession& client);
        // Server writes to client socket,
        virtual void Call(int);

      private:
        ClientSession& client_;
    };

  private:
    utils::unique_ptr<c_api::ClientSocket> client_sock_;
    int master_socket_fd_;  // to choose correct server later
    std::vector<char> client_buf_;
    size_t send_idx_;
    http::RequestBuilder rq_builder_;
    http::Response rs_;
    bool connection_closed_;
};

#endif  // WS_CLIENT_H
