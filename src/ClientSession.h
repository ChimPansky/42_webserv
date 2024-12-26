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
                                const utils::maybe<std::string>& file_to_send,
                                bool close_after_sending_rs_);
        virtual void Call(int);

      private:
        ClientSession& client_;
        c_api::SendPackage pack_;
        utils::unique_ptr<c_api::SendFilePackage> file_pack_;
        bool close_after_sending_rs_;
    };

    class ChooseServerCb : public http::IOnHeadersReadyCb {
      public:
        ChooseServerCb(ClientSession& client) : client_(client) {}
        virtual http::HeadersValidationResult Call(const http::Request& rq);

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

    enum ClientSessionState {
        CS_READY_TO_RECV,
        CS_BUSY
    };

  public:
    bool connection_closed() const { return connection_closed_; }
    UnixTimestampS last_activity_time() const { return last_activity_time_; }
    void ProcessNewData(c_api::RecvPackage& data_pack);
    void CloseConnection();
    void PrepareResponse(utils::unique_ptr<http::Response> rs);
    void ResponseSentCleanup(bool close_connection);

  private:
    void UpdateLastActivityTime_();

  private:
    utils::unique_ptr<c_api::ClientSocket> client_sock_;
    int master_socket_fd_;
    RequestDestination rq_destination_;
    utils::unique_ptr<AResponseProcessor> response_processor_;
    http::RequestBuilder rq_builder_;
    bool connection_closed_;
    ClientSessionState session_state_;
    UnixTimestampS last_activity_time_;
};

#endif  // WS_CLIENT_SESSION_H
