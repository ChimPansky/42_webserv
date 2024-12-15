#ifndef WS_CLIENT_SESSION_H
#define WS_CLIENT_SESSION_H

#include <ClientSocket.h>
#include <RequestBuilder.h>
#include <Server.h>
#include <multiplexers/ICallback.h>
#include <response_processors/AResponseProcessor.h>
#include <shared_ptr.h>
#include <sys/types.h>
#include <unique_ptr.h>

#include "LocationConfig.h"
#include "ServerConfig.h"

#define CLIENT_RD_CALLBACK_RD_SZ 512

class ClientSession {
    class ChooseServerCb : public http::IChooseServerCb {
      public:
        ChooseServerCb(ClientSession& client);

        virtual http::ChosenServerParams Call(const http::Request& rq);

      private:
        ClientSession& client_;
    };

  private:
    ClientSession(const ClientSession&);
    ClientSession& operator=(const ClientSession&);
    ClientSession();

  public:
    ClientSession(utils::unique_ptr<c_api::ClientSocket> client_sock, int master_sock_fd,
                  utils::shared_ptr<Server> default_server);
    ~ClientSession();
    bool connection_closed() const;
    bool IsRequestReady() const;
    void ProcessNewData(size_t bytes_recvd);
    void CloseConnection();
    void PrepareResponse(utils::unique_ptr<http::Response> rs);
    void ResponseSentCleanup(bool close_connection);
    class ClientReadCallback : public c_api::ICallback {
      public:
        ClientReadCallback(ClientSession& client);
        virtual void Call(int);

      private:
        ClientSession& client_;
    };
    class ClientWriteCallback : public c_api::ICallback {
      public:
        ClientWriteCallback(ClientSession& client, std::vector<char> buf,
                            bool close_after_sending_rs_);
        virtual void Call(int);

      private:
        ClientSession& client_;
        std::vector<char> buf_;
        size_t buf_send_idx_;
        bool close_after_sending_rs_;
    };
    class ClientProceedWithResponseCallback : public http::IResponseCallback {
      public:
        ClientProceedWithResponseCallback(ClientSession& client);
        virtual void Call(utils::unique_ptr<http::Response> rs);

      private:
        ClientSession& client_;
    };

  private:
    // TODO (vilvl) refact somehow, ignore state is invalid
    enum CsState {
        CS_READ,
        CS_IGNORE
    };
    utils::unique_ptr<c_api::ClientSocket> client_sock_;
    int master_socket_fd_;
    utils::shared_ptr<Server> associated_server_;
    utils::unique_ptr<AResponseProcessor> response_processor_;
    http::RequestBuilder rq_builder_;
    bool connection_closed_;
    CsState read_state_;
};


#endif  // WS_CLIENT_SESSION_H
