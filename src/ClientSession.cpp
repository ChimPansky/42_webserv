#include "ClientSession.h"

#include "c_api/EventManager.h"
#include "c_api/multiplexers/ICallback.h"
#include "http/RequestBuilder.h"
#include "utils/logger.h"
#include "utils/unique_ptr.h"

ClientSession::ClientSession(utils::unique_ptr<c_api::ClientSocket> sock, int master_sock_fd)
    : client_sock_(sock), master_socket_fd_(master_sock_fd), buf_send_idx_(0),
      connection_closed_(false)
{
    if (c_api::EventManager::get().RegisterCallback(
        client_sock_->sockfd(), c_api::CT_READ,
        utils::unique_ptr<c_api::ICallback>(new ClientReadCallback(*this))) != 0) {
            LOG(ERROR) << "Could not register read callback for client: " << client_sock_->sockfd();
            CloseConnection();
            return ;
        }
}

ClientSession::~ClientSession()
{}

void ClientSession::CloseConnection() {
    connection_closed_ = true;
    c_api::EventManager::get().DeleteCallback(client_sock_->sockfd(), c_api::CT_READWRITE);
    LOG(INFO) << "Client " << client_sock_->sockfd() << ": Connection closed";
}

bool ClientSession::connection_closed() const
{
    return connection_closed_;
}

#include <cstring>
#define HTTP_RESPONSE \
    "HTTP/1.1 200 OK\r\n\
Date: Mon, 27 Jul 2009 12:28:53 GMT\n\r\
Server: ft_webserv\n\r\
Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\n\r\
Content-Length: 88\n\r\
Content-Type: text/html\n\r\
Connection: Closed\n\r\
\n\r\
<html>\n\r\
<body>\n\r\
<h1>Hello, World!</h1>\n\r\
</body>\n\r\
</html>\n\r"

void ClientSession::PrepareResponse()
{
    LOG(INFO) << "";
    LOG(INFO) << "ClientSession::PrepareResponse";
    buf_send_idx_ = 0;
    buf_.resize(rq_builder_.buf().size());
    std::memcpy(buf_.data(), rq_builder_.buf().data(), rq_builder_.buf().size());
}

ClientSession::ClientReadCallback::ClientReadCallback(ClientSession& client) : client_(client)
{}

void ClientSession::ClientReadCallback::Call(int /*fd*/)
{
    LOG(INFO) << "";
    LOG(INFO) << "     ClientReadCallback::Call";

    ssize_t bytes_recvd = client_.client_sock_->Recv(client_.rq_builder_.buf(), CLIENT_RD_CALLBACK_BUF_SZ);
    if (bytes_recvd < 0) {
        LOG(ERROR) << "Could not read from client: " << client_.client_sock_->sockfd();
        client_.CloseConnection();
        return;
    }
    client_.ProcessNewData(bytes_recvd);
}

void ClientSession::ProcessNewData(size_t bytes_recvd) {
    LOG(INFO) << "";
    LOG(INFO) << "     ClientSession::ProcessNewData()";
    LOG(INFO) << "     Requestbuilder::Build()...";
    rq_builder_.Build(bytes_recvd);
    if (rq_builder_.builder_status() == http::RB_NEED_INFO_FROM_SERVER) {
        // get max_body_size (and maybe pointer to virtual server) from server...
        LOG(INFO) << "     RequestBuilder needs info from server (client_max_body_size)...";
        rq_builder_.ApplyServerInfo(1000);
        LOG(INFO) << "     RequestBuilder::ApplyServerInfo(): After matching and getting";
        LOG(INFO) << "       info from Server (Function not implemented yet):";
        LOG(INFO) << "       tell RequestBuilder what the max_body_size is...";
        virtual_server = NULL; // set this with pointer to server later...
        if (rq_builder_.builder_status() == http::RB_NEED_DATA_FROM_CLIENT) {
            return ; // reached end of buffer, so break out of ProcessNewData and wait for new data from client
        }
        LOG(INFO) << "     ClientSession::ProcessNewData(): there is still something left in buffer!";
        LOG(INFO) << "       -> call Build another time to reach end of buffer";
        LOG(INFO) << "     Requestbuilder::Build()...";
        rq_builder_.Build(bytes_recvd);
    }

    if (rq_builder_.builder_status() == http::RB_DONE) {
        LOG(INFO) << "RQ-Building finished -> deregister readcallback, register writecallback,";
        LOG(INFO) << "  close client connection (later: keep-alive) and";
        LOG(INFO) << "  prepare response (later: comes from server)";
        c_api::EventManager::get().DeleteCallback(client_sock_->sockfd(), c_api::CT_READ);
        if (c_api::EventManager::get().RegisterCallback(client_sock_->sockfd(), c_api::CT_WRITE,
            utils::unique_ptr<c_api::ICallback>(new ClientWriteCallback(*this))) != 0) {
                LOG(ERROR) << "Could not register write callback for client: "
                    << client_sock_->sockfd();
                CloseConnection();
                return ;
        }
        rq_builder_.rq().Print();
        PrepareResponse();  // for now just echo the request-buf-content
    }
}

ClientSession::ClientWriteCallback::ClientWriteCallback(ClientSession& client) : client_(client)
{
}

void ClientSession::ClientWriteCallback::Call(int /*fd*/)
{
    // assert fd == client_sock.fd
    ssize_t bytes_sent = client_.client_sock_->Send(client_.buf_, client_.buf_send_idx_,
                                                    client_.buf_.size() - client_.buf_send_idx_);
    if (bytes_sent <= 0) {
        LOG(ERROR) << "error on send";  // add perror
        client_.CloseConnection();
        return;
    }
    if (client_.buf_send_idx_ == client_.buf_.size()) {
        LOG(INFO) << client_.buf_send_idx_
                  << " bytes sent, close connection";
        client_.CloseConnection();
    }
}
