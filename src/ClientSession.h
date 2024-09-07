#ifndef WS_CLIENT_H
#define WS_CLIENT_H

#include <vector>

#include "c_api/ClientSocket.h"
#include "c_api/EventManager.h"
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
    ProcessState ProcessRead();
    class ClientCallback : public c_api::EventManager::ICallback {
      public:
        ClientCallback(ClientSession& client);
        // read/write from/to sock,
        virtual void Call(int fd);
        virtual c_api::EventManager::CallbackType callback_mode();

      private:
        ClientSession& client_;
        c_api::EventManager::CallbackType callback_mode_;
        void ReadCall();
        void WriteCall();
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
