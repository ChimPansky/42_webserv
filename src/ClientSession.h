#ifndef WS_CLIENT_H
#define WS_CLIENT_H

#include <sys/types.h>

#include <vector>

#include <ClientSocket.h>
#include <Server.h>
#include <multiplexers/ICallback.h>
#include <RequestBuilder.h>
#include <unique_ptr.h>
#include <shared_ptr.h>

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
    void PrepareResponse(utils::unique_ptr<http::Response> rs);
    void ResponseSentCleanup();
    class ClientReadCallback : public c_api::ICallback {
      public:
        ClientReadCallback(ClientSession& client);
        virtual void Call(int);

      private:
        ClientSession& client_;
    };
    class ClientWriteCallback : public c_api::ICallback {
      public:
        ClientWriteCallback(ClientSession& client, std::vector<char> buf);
        virtual void Call(int);

      private:
        ClientSession& client_;
        std::vector<char> buf_;
        size_t buf_send_idx_;
    };
    class ClientProceedWithResponseCallback : public http::IResponseCallback {
      public:
        ClientProceedWithResponseCallback(ClientSession& client);
        virtual void Call(utils::unique_ptr<http::Response> rs);
      private:
        ClientSession& client_;
    };

  private:
    enum CsState {
      CS_READ,
      CS_IGNORE
    };
    utils::unique_ptr<c_api::ClientSocket> client_sock_;
    int master_socket_fd_;   // to choose correct server later
    utils::shared_ptr<Server> associated_server_;
    http::RequestBuilder rq_builder_;
    http::Request rq_;
    bool connection_closed_;
    CsState read_state_;
    // Server* virtual_server; later: set this once request was successfully matched to corresponding server
};

#endif  // WS_CLIENT_H
