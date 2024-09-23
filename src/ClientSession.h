#ifndef WS_CLIENT_H
#define WS_CLIENT_H

#include <sys/types.h>
#include <vector>

#include "c_api/ClientSocket.h"
#include "c_api/multiplexers/ICallback.h"
#include "http/Request.h"
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
    void CloseConnection();
    ProcessState ProcessRead(ssize_t bytes_recvd); // not used now , use when building request...
    void PrepareResponse(); // later: get this from server
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
    int master_socket_fd_;  // to choose correct server later
    std::vector<char> buf_;  // string?
    size_t buf_send_idx_;
    http::Request rq_;
    http::Response rs_;
    bool connection_closed_;
};

#endif  // WS_CLIENT_H
