#include "ClientSession.h"

#include "Server.h"
#include "c_api/EventManager.h"
#include "c_api/multiplexers/ICallback.h"
#include "utils/logger.h"
#include "utils/unique_ptr.h"

ClientSession::ClientSession(utils::unique_ptr<c_api::ClientSocket> sock, int master_sock_fd, ServerCluster* server_cluster)
    : client_sock_(sock), master_socket_fd_(master_sock_fd), buf_send_idx_(0),
      connection_closed_(false), server_cluster_(server_cluster)
{
    if (c_api::EventManager::get().RegisterCallback(
            client_sock_->sockfd(), c_api::CT_READ,
            utils::unique_ptr<c_api::ICallback>(new ClientReadCallback(*this))) != 0) {
        LOG(ERROR) << "Could not register read callback for client: " << client_sock_->sockfd();
        CloseConnection();
        return;
    }
}

ClientSession::~ClientSession()
{}

void ClientSession::CloseConnection()
{
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
    buf_send_idx_ = 0;
    buf_.resize(sizeof(HTTP_RESPONSE));
    std::memcpy(buf_.data(), HTTP_RESPONSE, sizeof(HTTP_RESPONSE));
}

ClientSession::ClientReadCallback::ClientReadCallback(ClientSession& client) : client_(client)
{}

void ClientSession::ClientReadCallback::Call(int /*fd*/)
{
    // assert fd == client_sock.fd
    long bytes_recvd = client_.client_sock_->Recv(client_.buf_);
    if (bytes_recvd <= 0) {
        LOG(ERROR) << "Could not read from client: " << client_.client_sock_->sockfd();
        client_.CloseConnection();
        return;
    }
    LOG(DEBUG) << "ClientReadCallback::Call: " << bytes_recvd << " bytes recvd from "
               << client_.client_sock_->sockfd();
    if (static_cast<size_t>(bytes_recvd) < client_.client_sock_->buf_sz()) {
        c_api::EventManager::get().DeleteCallback(client_.client_sock_->sockfd(), c_api::CT_READ);
        if (c_api::EventManager::get().RegisterCallback(
                client_.client_sock_->sockfd(), c_api::CT_WRITE,
                utils::unique_ptr<c_api::ICallback>(new ClientWriteCallback(client_))) != 0) {
            LOG(ERROR) << "Could not register write callback for client: "
                       << client_.client_sock_->sockfd();
            client_.CloseConnection();
            return;
        }
        client_.PrepareResponse();
    }
}

ClientSession::ClientWriteCallback::ClientWriteCallback(ClientSession& client) : client_(client)
{}

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
        LOG(INFO) << client_.buf_send_idx_ << " bytes sent, close connection";
        client_.CloseConnection();
    }
}
