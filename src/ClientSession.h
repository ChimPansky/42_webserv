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

#define CLIENT_RD_CALLBACK_RD_SZ 512

class ClientSession {
  private:
    ClientSession(const ClientSession&);
    ClientSession& operator=(const ClientSession&);
    ClientSession();

  public:
    ClientSession(utils::unique_ptr<c_api::ClientSocket> client_sock, int master_sock_fd,
                  utils::shared_ptr<Server> default_server);
    ~ClientSession();

  private:
    // all the callbacks take the reference to the client
    //  therefore if client was destroyed, but callback was not,
    //  we'll get UB when CB is triggered.
    //  this can be handled by owning all the callbacks only

    class OnReadyToRecvFromClientCb : public c_api::ICallback {
      public:
        OnReadyToRecvFromClientCb(ClientSession& client) : client_(client) {}
        virtual void Call(int);

      private:
        ClientSession& client_;
    };

    class OnReadyToSendToClientCb : public c_api::ICallback {
      public:
        OnReadyToSendToClientCb(ClientSession& client, std::vector<char> buf,
                                bool close_after_sending_rs_);
        virtual void Call(int);

      private:
        ClientSession& client_;
        std::vector<char> buf_;
        size_t buf_send_idx_;
        bool close_after_sending_rs_;
    };

    class ChooseServerCb : public http::IChooseServerCb {
      public:
        ChooseServerCb(ClientSession& client) : client_(client) {}
        virtual http::ChosenServerParams Call(const http::Request& rq);

      private:
        ClientSession& client_;
    };

    class ClientProceedWithResponseCallback : public http::IResponseCallback {
      public:
        ClientProceedWithResponseCallback(ClientSession& client) : client_(client) {}
        virtual void Call(utils::unique_ptr<http::Response> rs);

      private:
        ClientSession& client_;
    };

    // TODO (vilvl) refact somehow, ignore state is invalid
    enum CsState {
        CS_READ,
        CS_IGNORE
    };

  public:
    bool connection_closed() const { return connection_closed_; }
    time_t last_activity_time() const { return last_activity_time_; }
    void ProcessNewData(size_t bytes_recvd);
    void CloseConnection();
    void PrepareResponse(utils::unique_ptr<http::Response> rs);
    void ResponseSentCleanup(bool close_connection);

  private:
    void UpdateLastActivitiTime_();

  private:
    utils::unique_ptr<c_api::ClientSocket> client_sock_;
    int master_socket_fd_;
    utils::shared_ptr<Server> associated_server_;
    utils::unique_ptr<AResponseProcessor> response_processor_;
    http::RequestBuilder rq_builder_;
    bool connection_closed_;
    CsState read_state_;
    time_t last_activity_time_;
};

#endif  // WS_CLIENT_SESSION_H
